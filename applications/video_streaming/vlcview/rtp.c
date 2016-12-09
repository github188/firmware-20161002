#include <lwip/netif.h>
#include "rtp.h"

#define RTP_STREAM_MULTICAST_IP   "224.2.1.88"
#define RTP_STREAM_MULTICAST_PORT 1234
#define SAP_MULTICAST_IP          "224.2.127.254"
#define SAP_MULTICAST_PORT        9875

static struct server_context *g_server;

#define rtp_list_container_of(ptr, sample, member)              \
(__typeof__(sample))((char *)(ptr) - offsetof(__typeof__(*sample), member))

#define rtp_list_for_each(pos, head, member)                    \
for (pos = rtp_list_container_of((head)->next, pos, member);    \
    &pos->member != (head);                                 \
    pos = rtp_list_container_of(pos->member.next, pos, member))

#define rtp_list_for_each_safe(pos, tmp, head, member)           \
for (pos = rtp_list_container_of((head)->next, pos, member),       \
    tmp = rtp_list_container_of((pos)->member.next, tmp, member);  \
    &pos->member != (head);                                  \
    pos = tmp,                                               \
    tmp = rtp_list_container_of(pos->member.next, tmp, member))

struct server_context *get_server(void)
{
    return g_server;
}

void set_server(struct server_context *server)
{
    g_server = server;
}

void rtp_list_init(struct list_t *list)
{
    list->prev = list;
    list->next = list;
}

void rtp_list_add(struct list_t *list, struct list_t *elm)
{
    elm->prev = list;
    elm->next = list->next;
    list->next = elm;
    elm->next->prev = elm;
}

void rtp_list_del(struct list_t *elm)
{
    elm->prev->next = elm->next;
    elm->next->prev = elm->prev;
    elm->next = NULL;
    elm->prev = NULL;
}

void init_packet_pool(struct rtp_info *info)
{
    info->pool.buf = rt_malloc(512 * 1024);
    info->pool.len = 512 * 1024;
    info->pool.pos = 0;
}

void reset_packet_pool(struct rtp_info *info)
{
    info->pool.pos = 0;
}

struct rtp_packet *get_packet_from_pool(struct rtp_info *info)
{
    unsigned char *ptr;
    struct rtp_packet *pkt;

    ptr = info->pool.buf + info->pool.pos;
    info->pool.pos += sizeof(struct rtp_packet);
    rt_memset(ptr, 0, sizeof(struct rtp_packet));
    pkt = (struct rtp_packet *) ptr;

    return pkt;
}

void base64_encode(unsigned char *dst, unsigned char *src, int len)
{
    int out_len;
    int i;
    int left_count;
    char index;
    static const char base64[128] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

    if (left_count = len % 3)
        out_len = (len / 3 + 1) * 4;
    else
        out_len = len / 3 * 4;

    for (i = 0; i < len / 3; i++) {
        index = src[i*3] >> 2;
        dst[i*4] = base64[index];
        index = ((src[i*3] & 0x03) << 4) | ((src[i*3+1] & 0xf0) >> 4);
        dst[i*4+1] = base64[index];
        index = ((src[i*3+1] & 0x0f) << 2) | ((src[i*3+2] & 0xc0) >> 6);
        dst[i*4+2] = base64[index];
        index = src[i*3+2] & 0x3f;
        dst[i*4+3] = base64[index];
    }
    if (left_count == 1) {
        index = src[i*3] >> 2;
        dst[i*4] = base64[index];
        index = (src[i*3] & 0x03) << 4;
        dst[i*4 + 1] = base64[index];
        dst[i*4 + 2] = '=';
        dst[i*4 + 3] = '=';
    }
    if (left_count == 2) {
        index = src[i*3] >> 2;
        dst[i*4] = base64[index];
        index = ((src[i*3] & 0x03) << 4) | ((src[i*3+1] & 0xf0) >> 4);
        dst[i*4 + 1] = base64[index];
        index = (src[i*+1] & 0x0f) << 2;
        dst[i*4 + 2] = base64[index];
        dst[i*4 + 3] = '=';
    }
    dst[out_len] = 0;
}

void generate_sdp(struct sdp_info *sdp)
{
    char tmp[64];

    if (!sdp->has_sps || !sdp->has_pps)
        return;

    sdp->content[0] = 0;

    strcat(sdp->content,
        "v=0\r\n");

    if (get_server()->port == 0) {
        strcat(sdp->content,
            "o=- 0 0 IN IP4 "RTP_STREAM_MULTICAST_IP"\r\n"
            "s=FH8620 Live Video Stream\r\n"
            "c=IN IP4 "RTP_STREAM_MULTICAST_IP"\r\n"
            "t=0 0\r\n");
        snprintf(tmp, 64, "m=video %d RTP/AVP 96\r\n", RTP_STREAM_MULTICAST_PORT);
    } else {
        snprintf(tmp, 64, "m=video 0 RTP/AVP 96\r\n");
    }
    strcat(sdp->content, tmp);
    strcat(sdp->content,
        "a=rtpmap:96 H264/90000\r\n"
        "a=fmtp:96 profile-level-id=4DE028;packetization-mode=1;"
        "sprop-parameter-sets=");

    base64_encode(tmp, sdp->sps, sdp->sps_len);
    strcat(sdp->content, tmp);
    strcat(sdp->content, ",");
    base64_encode(tmp, sdp->pps, sdp->pps_len);
    strcat(sdp->content, tmp);
    strcat(sdp->content, "\r\n");
    sdp->len = strlen(sdp->content);
}

void generate_rtp_header(
    unsigned char *dst,
    int payload_len,
    int is_last,
    unsigned short seq_num,
    unsigned long long pts,
    unsigned long ssrc)
{
    int index = 0;

    if (get_server()->rtp_info.transport == RTP_TRANSPORT_TCP) {
        /* RTSP interleaved frame header */
        dst[index++] = 0x24;
        dst[index++] = 0x00;
        dst[index++] = ((payload_len + 12) >> 8) & 0xff;
        dst[index++] = (payload_len + 12) & 0xff;
    }

    /* RTP header */
    dst[index++] = 0x80;
    dst[index++] = (is_last?0x80:0x00) | 96;
    dst[index++] = (seq_num >> 8) & 0xff;
    dst[index++] = seq_num & 0xff;
    dst[index++] = (pts >> 24) & 0xff;
    dst[index++] = (pts >> 16) & 0xff;
    dst[index++] = (pts >> 8) & 0xff;
    dst[index++] = pts & 0xff;
    dst[index++] = (ssrc >> 24 ) & 0xff;
    dst[index++] = (ssrc >> 16 ) & 0xff;
    dst[index++] = (ssrc >>  8 ) & 0xff;
    dst[index++] = ssrc & 0xff;
}

void generate_rtp_packets(
    struct list_t* pkt_list,
    void *data,
    int size,
    unsigned long long pts)
{
    unsigned char *nal;
    int nal_len;
    struct rtp_packet *pkt;
    int header_offset, max_len;

    if (pkt_list->next != pkt_list)
        return;

    if (get_server()->rtp_info.transport == RTP_TRANSPORT_TCP) {
        header_offset = 16;
        max_len = TCP_MSS;
    } else {
        header_offset = 12;
        max_len = RTP_PACKET_MAX_SIZE;
    }

    nal = data + 4;
    nal_len = size - 4;

    if (nal_len > max_len - header_offset) {
        int fragment_len = max_len - header_offset - 2;
        int pkt_num, i;
        int fu_len = fragment_len;
        unsigned char *fu_buf;

        if ((nal_len - 1) % fragment_len == 0)
            pkt_num = (nal_len - 1) / fragment_len;
        else
            pkt_num = (nal_len - 1) / fragment_len + 1;
        for (i = 0; i < pkt_num; i++) {
            pkt = get_packet_from_pool(&get_server()->rtp_info);
            if (i == pkt_num - 1)
                fu_len = nal_len - 1 - i * fragment_len;
            get_server()->rtp_info.sequence_num++;
            generate_rtp_header(pkt->buf,
                                fu_len + 2,
                                i == pkt_num - 1,
                                get_server()->rtp_info.sequence_num,
                                pts,
                                get_server()->rtp_info.ssrc);
            fu_buf = pkt->buf + header_offset;
            fu_buf[0] = 0x00 | (nal[0] & 0x60) | 28;
            fu_buf[1] = (i == 0 ? 0x80 : 0x00) | ((i == pkt_num - 1) ? 0x40 : 0x00) | (nal[0] & 0x1f);
            memcpy(fu_buf + 2, nal + 1 + i * fragment_len, fu_len);
            pkt->len = header_offset + 2 + fu_len;
            rtp_list_add(pkt_list->prev, &pkt->link);
        }
    } else {
        get_server()->rtp_info.sequence_num++;
        pkt = get_packet_from_pool(&get_server()->rtp_info);
        generate_rtp_header(pkt->buf,
                            nal_len,
                            0,
                            get_server()->rtp_info.sequence_num,
                            pts,
                            get_server()->rtp_info.ssrc);
        memcpy(pkt->buf + header_offset, nal, nal_len);
        pkt->len = header_offset + nal_len;
        rtp_list_add(pkt_list->prev, &pkt->link);
    }
}

int _send(int fd, void *buf, int len)
{
    struct timeval tv;
    fd_set fds;
    int total_write_bytes;
    int write_bytes;
    int ret;

    do {
        tv.tv_sec = 0;
        tv.tv_usec = 1000;
        FD_ZERO(&fds);
        FD_SET(fd, &fds);
        ret = select(fd + 1, NULL, &fds, NULL, &tv);
        if (ret > 0) {
            if (FD_ISSET(fd, &fds)) {
                send(fd, buf, len, 0);
            }
        } else if (ret == 0) {
            continue;
        } else {
            return -1;
        }
    } while (ret == 0);

    return 0;
}

void send_stream(struct client_context* client, struct list_t *packets)
{
    struct rtp_packet *pkt;

        rtp_list_for_each(pkt, packets, link) {
        if (get_server()->rtp_info.transport == RTP_TRANSPORT_UDP) {
            client->addr.sin_port = htons(client->udp_send_port);
            sendto(get_server()->rtp_info.udp_send_socket,
                   pkt->buf,
                   pkt->len,
                   0,
                   (struct sockaddr *)&client->addr,
                   sizeof(struct sockaddr_in));
        } else {
            _send(client->fd, pkt->buf, pkt->len);
        }
    }
}

void rtp_tcp_server_thread(void *arg)
{
    struct server_context *server_ctx = (struct server_context *)arg;
    struct sockaddr_in addr;
    int on;
    rt_thread_t client;
    socklen_t addr_len = sizeof(struct sockaddr_in);

    g_server = server_ctx;
    rtp_list_init(&server_ctx->client_list);
    init_packet_pool(&server_ctx->rtp_info);

    server_ctx->sd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_ctx->sd < 0) {
        debug("create server socket failed\n");
        return;
    }

    /* ignore "socket already in use" errors */
    on = 1;
    lwip_setsockopt(server_ctx->sd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
    setsockopt(server_ctx->sd, IPPROTO_TCP, TCP_NODELAY, (void *)&on, sizeof(on));

    addr.sin_family = AF_INET;
    addr.sin_port = server_ctx->port;
    addr.sin_addr.s_addr = INADDR_ANY;
    rt_memset(&(addr.sin_zero),8, sizeof(addr.sin_zero));

    if (bind(server_ctx->sd, (struct sockaddr*)&addr, sizeof(addr)) != 0 ) {
        debug("Unable to bind\n");
        return;
    }

    if (listen(server_ctx->sd, RT_LWIP_TCP_PCB_NUM) != 0) {
        debug("Listen error\n");
        return;
    }

    while (!server_ctx->stop) {
        struct client_context *client_ctx = rt_malloc(sizeof(struct client_context));

        if (client_ctx == RT_NULL) {
            debug("failed to allocate (a very small amount of) memory\n");
            return;
        }
        client_ctx->fd = accept(server_ctx->sd,
                                (struct sockaddr *)&client_ctx->addr,
                                &addr_len);
        client_ctx->start_play = 0;
        client_ctx->find_i_frame = 0;
        if (server_ctx->init_client)
            server_ctx->init_client(client_ctx);
        rtp_list_add(server_ctx->client_list.prev, &client_ctx->link);
    }
    lwip_close(server_ctx->sd);
}

void rtp_push_data(void *data, int size, unsigned long long pts)
{
    rt_err_t result;
    int i;
    char *p = (char *) data;
    struct client_context *client, *next;
    struct list_t packets;

    if (!get_server()->rtp_info.sdp.has_sps || !get_server()->rtp_info.sdp.has_pps) {
        if (7 == (p[4] & 0x1f)) {
            memcpy(get_server()->rtp_info.sdp.sps, p, size);
            get_server()->rtp_info.sdp.sps_len = size;
            get_server()->rtp_info.sdp.has_sps = 1;
        } else if (8 == (p[4] & 0x1f)) {
            memcpy(get_server()->rtp_info.sdp.pps, p, size);
            get_server()->rtp_info.sdp.pps_len = size;
            get_server()->rtp_info.sdp.has_pps = 1;
        }
        generate_sdp(&get_server()->rtp_info.sdp);
    }

    if (get_server()->client_list.next && get_server()->client_list.next != &get_server()->client_list) {
        rtp_list_init(&packets);
        rtp_list_for_each_safe(client, next, &get_server()->client_list, link) {
            if (client) {
                if (client->process_func) {
                    if (client->process_func(client) < 0) {
                        rtp_list_del(&client->link);
                        rt_free(client);
                        continue;
                    }
                }
                if (client->start_play) {
                    if (get_server()->port != 0 && client->find_i_frame == 0) { /* using RTSP */
                        if ((p[4] & 0x1f) == 5) /* I frame */ {
                            client->find_i_frame = 1;
                        }
                    }
                    if (client->find_i_frame) {
                        generate_rtp_packets(&packets, data, size, pts * 9 / 100);
                        send_stream(client, &packets);
                    }
                }
            }
        }
        reset_packet_pool(&get_server()->rtp_info);
    }
}

void server_cleanup(struct server_context *server)
{
    struct client_context *client, *next;

    rtp_list_for_each_safe(client, next, &server->client_list, link) {
        if (server->release_client)
            server->release_client(client);
        rtp_list_del(&client->link);
        if (client->fd != -1) {
            debug("close client socket %d\n", client->fd);
            lwip_close(client->fd);
        }
        rt_free(client);
    }
    if (server->sd != -1) {
        debug("close server socket %d\n", server->sd);
        lwip_close(server->sd);
    }
    if (server->rtp_info.udp_send_socket != -1) {
        debug("close udp send socket %d\n", server->rtp_info.udp_send_socket);
        lwip_close(server->rtp_info.udp_send_socket);
    }
    rt_free(server->rtp_info.pool.buf);
}


static struct server_context g_rtp_server;
static char g_sap_buf[1024];

void sap_thread(void *arg)
{
    int sd;
    struct sockaddr_in addr;
    int sdp_ready = 0;
    int sap_len;
    
    struct server_context *server_ctx = (struct server_context *)arg;
    int server_ip;
    unsigned short msg_id;

    sd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sd < 0) {
        debug("create sap socket failed\n");
        return;
    }
    addr.sin_family = AF_INET;
    addr.sin_port = htons(SAP_MULTICAST_PORT);
    addr.sin_addr.s_addr = inet_addr(SAP_MULTICAST_IP);

    if (netif_default)
        server_ip = netif_default->ip_addr.addr;
    else
        server_ip = inet_addr(SAP_MULTICAST_IP);

    srand((int)time(RT_NULL));
    msg_id = rand() & 0xffff;

    g_sap_buf[0] = 0x20;
    g_sap_buf[1] = 0;
    g_sap_buf[2] = (msg_id >> 8) & 0xff;
    g_sap_buf[3] = msg_id & 0xff;
    memcpy(g_sap_buf + 4, (char *)&server_ip, 4);
    snprintf(g_sap_buf + 8, 16, "application/sdp");

    while (!server_ctx->stop) {
        if (sdp_ready == 0) {
            if (server_ctx->rtp_info.sdp.has_sps && server_ctx->rtp_info.sdp.has_pps) {
                sdp_ready = 1;
                memcpy(g_sap_buf + 24, server_ctx->rtp_info.sdp.content, server_ctx->rtp_info.sdp.len);
                sap_len = 24 + server_ctx->rtp_info.sdp.len;
            }
        } else {
            sendto(sd, g_sap_buf, sap_len, 0, (struct sockaddr*)&addr, sizeof(addr));
        }
        rt_thread_delay(100);
    }
}

int sap_rtp_start_server(void)
{
    struct client_context *multicast_client;

    rt_memset(&g_rtp_server, 0, sizeof(struct server_context));

    g_rtp_server.thread_id = rt_thread_create("sap_announcer",
                                     sap_thread,
                                     &g_rtp_server,
                                     RTP_THREAD_STACK_SIZE,
                                     RT_APP_THREAD_PRIORITY + 1,
                                     RTP_THREAD_TIMESLICE);
    if (g_rtp_server.thread_id != RT_NULL)
        rt_thread_startup(g_rtp_server.thread_id);
    else
        return -1;

    set_server(&g_rtp_server);
    g_rtp_server.port = 0;
    g_rtp_server.stop = 0;
    g_rtp_server.init_client = RT_NULL;
    g_rtp_server.release_client = RT_NULL;
    g_rtp_server.rtp_info.transport = RTP_TRANSPORT_UDP;
    rtp_list_init(&g_rtp_server.client_list);
    init_packet_pool(&g_rtp_server.rtp_info);

    g_rtp_server.rtp_info.udp_send_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (g_rtp_server.rtp_info.udp_send_socket < 0) {
        debug("sap-rtp: create udp_send_socket failed\n");
        return -1;
    }

    multicast_client = rt_malloc(sizeof(struct client_context));
    if (multicast_client == RT_NULL) {
        debug("sap-rtp: create client context failed\n");
        return -1;
    }
    bzero(&multicast_client->addr, sizeof(struct sockaddr_in));
    multicast_client->addr.sin_family = AF_INET;
    multicast_client->addr.sin_addr.s_addr = inet_addr(RTP_STREAM_MULTICAST_IP);
    multicast_client->udp_send_port = RTP_STREAM_MULTICAST_PORT;
    multicast_client->start_play = 1;
    multicast_client->process_func = RT_NULL;
    multicast_client->find_i_frame = 1; /* bypass I frame check procedure */
    multicast_client->fd = -1;
    rtp_list_add(g_rtp_server.client_list.prev, &multicast_client->link);

    return 0;
}

void sap_rtp_stop_server(void)
{
    debug("sap-rtp: stop server\n");

    g_rtp_server.stop = 1;

    if (g_rtp_server.thread_id != RT_NULL && g_rtp_server.thread_id->stat != RT_THREAD_CLOSE) {
        debug("sap-rtp: delete sap announcer thread\n");
        rt_thread_delete(g_rtp_server.thread_id);
    }
    server_cleanup(&g_rtp_server);
}
