#include "rtp.h"

#include <errno.h>

#define REQUEST_READ_BUF_SIZE 256
#define RESPONSE_BUF_SIZE     1024
#define RTP_UDP_SEND_PORT     3056

struct rtsp_session {
    struct client_context *ctx;
    unsigned char read_buf[REQUEST_READ_BUF_SIZE];
    unsigned char response_buf[RESPONSE_BUF_SIZE];
    char cseq[10];
    unsigned long session_num;
};

enum {
    RTSP_MSG_NULL,
    RTSP_MSG_OPTIONS,
    RTSP_MSG_DESCRIBE,
    RTSP_MSG_SETUP,
    RTSP_MSG_PLAY,
    RTSP_MSG_TEARDOWN
}; 

static struct server_context g_rtsp_server;

void generate_response_header(struct rtsp_session *session)
{
    char tmp[32];

    session->response_buf[0] = 0;
    strcat(session->response_buf, "RTSP/1.0 200 OK\r\n");
    snprintf(tmp, sizeof(tmp), "CSeq: %s\r\n", session->cseq);
    strcat(session->response_buf, tmp);
}

void send_response(int fd, void *buffer, int size)
{
    send(fd, buffer, size, 0);
}

void generate_session_number(struct rtsp_session *session)
{
    struct timeval tv;

    gettimeofday(&tv, NULL);
    session->session_num = tv.tv_sec*1000L + tv.tv_usec/1000L;
}

void rtsp_handle_options(struct rtsp_session *session)
{
    generate_response_header(session);
    strcat(session->response_buf,
           "Public: OPTIONS, DESCRIBE, SETUP, TEARDOWN, PLAY, PAUSE\r\n\r\n");
    send_response(session->ctx->fd, session->response_buf, strlen(session->response_buf));
}

void rtsp_handle_describe(struct rtsp_session *session)
{
    char tmp[32];

    generate_response_header(session);
    strcat(session->response_buf, "Content-Type: application/sdp\r\n");
    snprintf(tmp, sizeof(tmp), "Content-Length: %d\r\n\r\n", strlen(g_rtsp_server.rtp_info.sdp.content));
    strcat(session->response_buf, tmp);
    strcat(session->response_buf, g_rtsp_server.rtp_info.sdp.content);
    send_response(session->ctx->fd, session->response_buf, strlen(session->response_buf));
}

void rtsp_handle_setup(struct rtsp_session *session)
{
    char tmp[100];

    generate_session_number(session);
    generate_response_header(session);
    snprintf(tmp, sizeof(tmp), "Session: %lu\r\n", session->session_num);
    strcat(session->response_buf, tmp);

    if (g_rtsp_server.rtp_info.transport == RTP_TRANSPORT_UDP) {
        if (session->ctx->udp_send_port == 0)
            session->ctx->udp_send_port = RTP_UDP_SEND_PORT;
        snprintf(tmp, 100,
            "Transport: RTP/AVP;unicast;"
            "client_port=%d-%d\r\n\r\n",
            session->ctx->udp_send_port, session->ctx->udp_send_port + 1);
        strcat(session->response_buf, tmp);
    } else {
        strcat(session->response_buf,
            "Transport: RTP/AVP/TCP;unicast;interleaved=0-1;\r\n"
            "\r\n");
    }
    send_response(session->ctx->fd, session->response_buf, strlen(session->response_buf));
}

void rtsp_handle_play(struct rtsp_session *session)
{
    generate_response_header(session);
    strcat(session->response_buf, "\r\n");
    send_response(session->ctx->fd, session->response_buf, strlen(session->response_buf));
    session->ctx->start_play = 1;
}

int _read(int fd, void *buffer, int len, int timeout_ms)
{
    fd_set fds;
    struct timeval tv;
    int total_read_bytes = 0;
    int read_bytes;
    int ret;

    do {
        tv.tv_sec = 0;
        tv.tv_usec = timeout_ms * 1000;
        FD_ZERO(&fds);
        FD_SET(fd, &fds);
        if ((ret = select(fd + 1, &fds, NULL, NULL, &tv)) > 0) {
            if (FD_ISSET(fd, &fds)) {
                read_bytes = recv(fd, buffer + total_read_bytes, 1, 0);
                if (read_bytes > 0) {
                    total_read_bytes += read_bytes;
                } else if (read_bytes == 0) {
                    return 0; /* timeout */
                } else {
                    return -1; /* error */
                }
            }
        } else if (ret == 0) {
            return 0; /* timeout */
        } else {
            return -1; /* error */
        }
    } while (total_read_bytes < len);

    return total_read_bytes;
}

int _readline(int fd, void *buffer, int len, int timeout_ms)
{
    char c;
    int i;
    char *out = buffer;
    int ret;

    for (i = 0; i < len && c != '\n'; i++) {
        if ((ret = _read(fd, &c, 1, timeout_ms)) > 0)
            *(out++) = c;
        else if (ret == 0)
            return 0;  /* timeout */
        else
            return -1; /* error */
    }
    return i;
}

int rtsp_get_request(struct rtsp_session *session)
{
    int msg_type;
    int count;
    int ret;
    unsigned char *sub_str;

    if ((ret = _readline(session->ctx->fd, session->read_buf, REQUEST_READ_BUF_SIZE - 1, 1)) > 0) {
        if (strstr(session->read_buf, "OPTIONS"))
            msg_type = RTSP_MSG_OPTIONS;
        else if(strstr(session->read_buf, "DESCRIBE"))
            msg_type = RTSP_MSG_DESCRIBE;
        else if(strstr(session->read_buf, "SETUP"))
            msg_type = RTSP_MSG_SETUP;
        else if(strstr(session->read_buf, "PLAY"))
            msg_type = RTSP_MSG_PLAY;
        else if(strstr(session->read_buf, "TEARDOWN"))
            msg_type = RTSP_MSG_TEARDOWN;
        else
            msg_type = RTSP_MSG_NULL;
    } else if (ret == 0) {
        return RTSP_MSG_NULL;
    } else {
        return -1;
    }

    do {
        rt_memset(session->read_buf, 0, REQUEST_READ_BUF_SIZE);
        if ((count = _readline(session->ctx->fd, session->read_buf, REQUEST_READ_BUF_SIZE - 1, 10))) {
            if (strstr(session->read_buf, "CSeq: ")) {
                unsigned char *pos = strstr(session->read_buf, "\r\n");
                if (pos) {
                    memcpy(session->cseq, session->read_buf + 6, pos - session->read_buf - 6);\
                    session->cseq[pos - session->read_buf - 6] = 0;
                }
            }
            if ((sub_str = strstr(session->read_buf, "client_port="))){
                unsigned char *pos = strstr(session->read_buf, "-");
                if (pos) {
                    char tmp[10];
                    memcpy(tmp, sub_str + 12, pos - sub_str - 12);
                    tmp[pos - sub_str - 12] = 0;
                    session->ctx->udp_send_port = strtol(tmp, RT_NULL, 0);
                }
            }
        } else if (count < 0) {
            return -1;
        }
    } while (count > 0);

    return msg_type;
}

int rtsp_message_process(struct rtsp_session *session)
{
    int type;

    type = rtsp_get_request(session);

    if (type < 0)
        return -1;

    switch(type) {
    case RTSP_MSG_OPTIONS:
        rtsp_handle_options(session);
        break;
    case RTSP_MSG_DESCRIBE:
        rtsp_handle_describe(session);
        break;
    case RTSP_MSG_SETUP:
        rtsp_handle_setup(session);
        break;
    case RTSP_MSG_PLAY:
        rtsp_handle_play(session);
        break;
    case RTSP_MSG_TEARDOWN:
        session->ctx->start_play = 0;
        return -1;
    }
    return 0;
}

int rtsp_process_func(struct client_context *client)
{
    struct rtsp_session *session = (struct rtsp_session *) client->session;
    if (rtsp_message_process(session) < 0) {
        debug("rtsp: client teardown, socket=%d\n", client->fd);
        lwip_close(client->fd);
        return -1;
    }
    return 0;
}

void init_rtsp_session(struct client_context *client)
{
    struct rtsp_session *session;

    debug("rtsp: new client connected, socket=%d\n", client->fd);

    session = rt_malloc(sizeof(struct rtsp_session));
    rt_memset(session, 0, sizeof(struct rtsp_session));
    session->ctx = client;
    client->session = session;
    client->process_func = rtsp_process_func;
}

void cleanup_rtsp_session(struct client_context *client)
{
    if (client->session)
        rt_free(client->session);
}

int rtsp_start_server(enum rtp_transport transport)
{
    int port_inet;

    rt_memset(&g_rtsp_server, 0, sizeof(struct server_context));

    if (transport != RTP_TRANSPORT_UDP &&
        transport != RTP_TRANSPORT_TCP)
        g_rtsp_server.rtp_info.transport = RTP_TRANSPORT_TCP;
    else
        g_rtsp_server.rtp_info.transport = transport;

    if (g_rtsp_server.rtp_info.transport == RTP_TRANSPORT_UDP) {
        g_rtsp_server.rtp_info.udp_send_socket = socket(AF_INET, SOCK_DGRAM, 0);
        if (g_rtsp_server.rtp_info.udp_send_socket == -1)
            return -1;
    }

    g_rtsp_server.stop = 0;
    port_inet = htons(554);
    g_rtsp_server.port = port_inet;
    g_rtsp_server.init_client = init_rtsp_session;
    g_rtsp_server.release_client = cleanup_rtsp_session;
    g_rtsp_server.thread_id = rt_thread_create("rtsp_server",
                                               rtp_tcp_server_thread,
                                               &g_rtsp_server,
                                               RTP_THREAD_STACK_SIZE,
                                               RT_APP_THREAD_PRIORITY,
                                               RTP_THREAD_TIMESLICE);
    if (g_rtsp_server.thread_id != RT_NULL)
        rt_thread_startup(g_rtsp_server.thread_id);
    else
        return -1;

    return 0;
}

void rtsp_stop_server(void)
{
    debug("rtsp: stop server\n");

    g_rtsp_server.stop = 1;

    if (g_rtsp_server.thread_id != RT_NULL && g_rtsp_server.thread_id->stat != RT_THREAD_CLOSE) {
        debug("rtsp: delete server thread\n");
        rt_thread_delete(g_rtsp_server.thread_id);
    }
    server_cleanup(&g_rtsp_server);
}