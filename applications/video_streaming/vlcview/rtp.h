#ifndef __RTP_H__
#define __RTP_H__

#include <stdio.h>
#include <rtthread.h>
#include <lwip/sockets.h>
#include <string.h>

#define RTP_DEBUG

#ifdef RTP_DEBUG
#  define debug rt_kprintf
#else
#  define debug
#endif

#define RTP_THREAD_STACK_SIZE     1024
#define RTP_THREAD_TIMESLICE      10
#define RTP_PACKET_MAX_SIZE       1472

enum rtp_transport {
    RTP_TRANSPORT_UDP,
    RTP_TRANSPORT_TCP
};

struct list_t {
    struct list_t *prev;
    struct list_t *next;
};

struct sdp_info {
    unsigned char sps[32];
    int sps_len;
    int has_sps;
    unsigned char pps[32];
    int pps_len;
    int has_pps;
    unsigned char content[1024];
    int len;
};

struct rtp_packet {
    unsigned char buf[RTP_PACKET_MAX_SIZE];
    int len;
    struct list_t link;
};

struct pkt_pool {
    unsigned char *buf;
    int len;
    int pos;
};

struct rtp_info {
    unsigned short sequence_num;
    unsigned long ssrc;
    struct sdp_info sdp;
    struct pkt_pool pool;
    enum rtp_transport transport;
    int udp_send_socket;
};

struct client_context {
    int fd;
    struct sockaddr_in addr;
    int udp_send_port;
    int start_play;
    int find_i_frame;
    void *session;
    int (*process_func)(struct client_context *ctx);
    struct list_t link;
};

struct server_context {
    int stop;
    int sd;
    rt_thread_t thread_id;
    int port;
    void (*init_client)(struct client_context *ctx);
    void (*release_client)(struct client_context *ctx);
    struct rtp_info rtp_info;
    struct list_t client_list;
};

void rtp_tcp_server_thread(void *arg);
struct server_context *get_server(void);
void set_server(struct server_context *server);
void server_cleanup(struct server_context *server);

#endif
