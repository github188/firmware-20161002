/*
 * File      : telnet.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2006 - 2013, RT-Thread Development Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rt-thread.org/license/LICENSE
 *
 * Change Logs:
 * Date           Author       Notes
 * 2010-04-29     bernard      the first version
 * 2013-05-16     aozima       use the RT-Thread ringbuffer.
 */

#include <rtthread.h>
#include <rtdevice.h>

#include <lwip/api.h>
#include <lwip/tcp.h>

#include <finsh.h>
#include <shell.h>

#define TELNET_THREAD_NAME "telnet"
#define TELNET_DEVICE_NAME "telnet"

#define TELNET_PORT         23
#define TELNET_RX_BUFFER    256
#define TELNET_TX_BUFFER    4096

#define ISO_nl              0x0a
#define ISO_cr              0x0d

#define STATE_NORMAL        0
#define STATE_IAC           1
#define STATE_WILL          2
#define STATE_WONT          3
#define STATE_DO            4
#define STATE_DONT          5

#define TELNET_IAC          255
#define TELNET_WILL         251
#define TELNET_WONT         252
#define TELNET_DO           253
#define TELNET_DONT         254
#define TELNET_SB           250
#define TELNET_SE           240

/* Echo option */
#define TELNET_OP_ECHO      1

/* Suppress Go Ahead option */
#define TELNET_OP_GA        3

/* line mode option of telnet(RFC1184) */
#define TELNET_OP_LM        34
/* suboption MODE */
#define TELNET_OP_LM_MODE   1
/* the mask on the server side. Only LIT_ECHO is set. */
#define TELNET_OP_LM_MASK   16

#define NW_RX 0x01
#define NW_TX 0x02
#define NW_CLOSED 0x04
#define NW_MASK (NW_RX | NW_TX | NW_CLOSED)

struct telnet_session
{
    struct rt_device device;

    struct rt_mutex read_lock;
    struct rt_mutex write_lock;

    struct rt_event nw_event;
    struct netconn *conn;

    /* telnet protocol */
    rt_uint8_t state;
    rt_uint8_t echo_mode;

    /* WILL ECHO sent from this side. */
    rt_uint16_t host_ask_for_echo : 1;
    /* DO ECHO sent from other side. */
    rt_uint16_t remote_ask_me_echo : 1;
    /* get the current ECHO mode from finsh_get_echo */
    /* WILL GA sent from this side. */
    rt_uint16_t host_ask_for_ga : 1;
    /* DO GA sent from this side. */
    rt_uint16_t host_request_ga : 1;
    /* WILL GA sent from other side. I.e., remote will suppress GA. */
    rt_uint16_t remote_ask_for_ga : 1;
    /* DO GA sent from this side. I.e., we should suppress GA on this side. */
    rt_uint16_t remote_ask_me_ga : 1;

    struct rt_ringbuffer rx_ringbuffer;
    struct rt_ringbuffer tx_ringbuffer;
    rt_uint8_t rx_rbp[TELNET_RX_BUFFER];
    rt_uint8_t tx_rbp[TELNET_TX_BUFFER];
};
/* we have to use a global variable here because the rx_callback of struct
 * netconn need the telnet session and there is no usable user_data field in
 * that struct. All the other part of the code should avoid to use this
 * directly.
 *
 * The drawback is that we could only have one telnet session on the board. But
 * who need an other? */
static struct telnet_session the_telnet;

#ifdef RT_USING_LOGTRACE
#include <log_trace.h>
static struct log_trace_session telnet_lg = {{"tlnt"}, LOG_TRACE_LEVEL_WARNING};

#define telnet_lg_error(fmt, ...) \
    log_session(&telnet_lg, LOG_TRACE_ERROR fmt, ##__VA_ARGS__)
#define telnet_lg_warning(fmt, ...) \
    log_session(&telnet_lg, LOG_TRACE_WARNING fmt, ##__VA_ARGS__)
#define telnet_lg_info(fmt, ...) \
    log_session(&telnet_lg, LOG_TRACE_INFO fmt, ##__VA_ARGS__)
#define telnet_lg_debug(fmt, ...) \
    log_session(&telnet_lg, LOG_TRACE_DEBUG fmt, ##__VA_ARGS__)
#else  // RT_USING_LOGTRACE
/* console will be redirect to telnet after it start up. Don't put the info
 * to the console. */
#define telnet_lg_error(fmt, ...)
#define telnet_lg_warning(fmt, ...)
#define telnet_lg_info(fmt, ...)
#define telnet_lg_debug(fmt, ...)
#endif  // RT_USING_LOGTRACE

static void telnet_reset(struct telnet_session *telnet)
{
    telnet->state = STATE_NORMAL;

    telnet->host_ask_for_echo  = 0;
    telnet->remote_ask_me_echo = 0;
    telnet->host_ask_for_ga    = 0;
    telnet->host_request_ga    = 0;
    telnet->remote_ask_for_ga  = 0;
    telnet->remote_ask_me_ga   = 0;

    rt_event_control(&telnet->nw_event, RT_IPC_CMD_RESET, 0);

    /* re-initialize ring buffer */
    rt_ringbuffer_init(&telnet->rx_ringbuffer, telnet->rx_rbp,
                       TELNET_RX_BUFFER);
    rt_ringbuffer_init(&telnet->tx_ringbuffer, telnet->tx_rbp,
                       TELNET_TX_BUFFER);
}

/* RT-Thread Device Driver Interface */
static rt_err_t telnet_init(rt_device_t dev) { return RT_EOK; }
static rt_err_t telnet_open(rt_device_t dev, rt_uint16_t oflag)
{
    return RT_EOK;
}

static rt_err_t telnet_close(rt_device_t dev)
{
    struct telnet_session *telnet = (struct telnet_session *)dev->user_data;

    return rt_event_send(&telnet->nw_event, NW_CLOSED);
}

static rt_size_t telnet_read(rt_device_t dev, rt_off_t pos, void *buffer,
                             rt_size_t size)
{
    rt_err_t result;
    rt_size_t length;
    struct telnet_session *telnet = (struct telnet_session *)dev->user_data;

    if (rt_interrupt_get_nest()) return 0;

    result = rt_mutex_take(&telnet->read_lock, RT_WAITING_FOREVER);
    if (result != RT_EOK)
    {
        return 0;
    }

    length = rt_ringbuffer_data_len(&telnet->rx_ringbuffer);
    if (length > size)
    {
        length = size;
    }

    size = rt_ringbuffer_get(&telnet->rx_ringbuffer, buffer, length);

    result = rt_mutex_release(&telnet->read_lock);

    return size;
}

static rt_size_t telnet_write(rt_device_t dev, rt_off_t pos, const void *buffer,
                              rt_size_t size)
{
    rt_err_t result;
    const rt_uint8_t *ptr;
    struct telnet_session *telnet = (struct telnet_session *)dev->user_data;

    ptr = (rt_uint8_t *)buffer;

    if (rt_interrupt_get_nest()) return 0;

    result = rt_mutex_take(&telnet->write_lock, RT_WAITING_FOREVER);
    if (result != RT_EOK)
    {
        return 0;
    }

    while (size)
    {
        if (*ptr == '\n') rt_ringbuffer_putchar(&telnet->tx_ringbuffer, '\r');

        if (rt_ringbuffer_putchar(&telnet->tx_ringbuffer, *ptr) == 0)  /* overflow */
            break;

        ptr++;
        size--;
    }

    result = rt_mutex_release(&telnet->write_lock);

    /* send to network side */
    rt_event_send(&telnet->nw_event, NW_TX);

    return (rt_uint32_t)ptr - (rt_uint32_t)buffer;
}

static rt_err_t telnet_control(rt_device_t dev, rt_uint8_t cmd, void *args)
{
    return RT_EOK;
}

/* netconn callback function */
static void rx_callback(struct netconn *conn, enum netconn_evt evt,
                        rt_uint16_t len)
{
    if (evt == NETCONN_EVT_RCVPLUS)
    {
        rt_event_send(&the_telnet.nw_event, NW_RX);
    }

    if (conn->state == NETCONN_CLOSE)
    {
        rt_event_send(&the_telnet.nw_event, NW_CLOSED);
    }
}

/* send telnet option to remote */
static void telnet_send_option(struct telnet_session *telnet, rt_uint8_t option,
                               rt_uint8_t value)
{
    rt_err_t result;
    rt_uint8_t optbuf[4];

    optbuf[0] = TELNET_IAC;
    optbuf[1] = option;
    optbuf[2] = value;
    optbuf[3] = 0;

    result = rt_mutex_take(&telnet->write_lock, RT_WAITING_FOREVER);
    if (result != RT_EOK)
    {
        return;
    }

    rt_ringbuffer_put(&telnet->tx_ringbuffer, optbuf, 3);

    result = rt_mutex_release(&telnet->write_lock);

    /* trigger a tx event */
    rt_event_send(&telnet->nw_event, NW_TX);
}

/* process tx data */
static void telnet_process_tx(struct telnet_session *telnet)
{
    rt_err_t result;
    rt_size_t length;
    rt_uint8_t tx_buffer[32];

    result = rt_mutex_take(&telnet->write_lock, RT_WAITING_FOREVER);
    if (result != RT_EOK)
    {
        return;
    }

    for (length = rt_ringbuffer_data_len(&telnet->tx_ringbuffer); length;
         length = rt_ringbuffer_data_len(&telnet->tx_ringbuffer))
    {
        if (length > sizeof(tx_buffer)) length = sizeof(tx_buffer);

        /* do a tx procedure */
        rt_ringbuffer_get(&telnet->tx_ringbuffer, tx_buffer, length);
        result = netconn_write(telnet->conn, tx_buffer, length, NETCONN_COPY);
        if (result < 0)
        {
            telnet_lg_error("connect closed, errno=%d\n", result);
            telnet->conn->state = NETCONN_CLOSE;
            rt_event_send(&telnet->nw_event, NW_CLOSED);
            break;
        }
    }

    result = rt_mutex_release(&telnet->write_lock);
}

/* process rx data */
static void telnet_process_rx(struct telnet_session *telnet, rt_uint8_t *data,
                              rt_size_t length)
{
    rt_size_t rx_length, index;

    for (index = 0; index < length; index++)
    {
        switch (telnet->state)
        {
        case STATE_IAC:
            /* set telnet state according to received package */
            switch (*data)
            {
            case TELNET_WILL:
                telnet->state = STATE_WILL;
                break;
            case TELNET_WONT:
                telnet->state = STATE_WONT;
                break;
            case TELNET_DO:
                telnet->state = STATE_DO;
                break;
            case TELNET_DONT:
                telnet->state = STATE_DONT;
                break;
            case TELNET_IAC:
                {
                    rt_err_t res = rt_mutex_take(&telnet->read_lock, RT_WAITING_FOREVER);
                    if (res == RT_EOK)
                    {
                        rt_ringbuffer_putchar(&telnet->rx_ringbuffer, *data);
                        rt_mutex_release(&telnet->read_lock);
                    }
                }
                telnet->state = STATE_NORMAL;
                break;
            default:
                telnet->state = STATE_NORMAL;
                break;
            }
            break;

        /* don't option */
        case STATE_WILL:
            switch (*data)
            {
            case TELNET_OP_GA:
                if (telnet->host_request_ga)
                    telnet->host_request_ga = 0;
                else
                    telnet_send_option(telnet, TELNET_DO, TELNET_OP_GA);
                telnet->remote_ask_for_ga = 1;
                telnet->state             = STATE_NORMAL;
                break;
            default:
                telnet_send_option(telnet, TELNET_DONT, *data);
                telnet->state = STATE_NORMAL;
                break;
            };
            break;
        case STATE_WONT:
            telnet_send_option(telnet, TELNET_DONT, *data);
            telnet->state = STATE_NORMAL;
            break;

        /* won't option */
        case STATE_DO:
            switch (*data)
            {
            case TELNET_OP_ECHO:
                /* be silent on ACK from other side */
                if (telnet->host_ask_for_echo)
                    telnet->host_ask_for_echo = 0;
                else
                    telnet_send_option(telnet, TELNET_WILL, TELNET_OP_ECHO);
                finsh_set_echo(1);
                telnet->state = STATE_NORMAL;
                break;
            case TELNET_OP_GA:
                /* be silent on ACK from other side */
                if (telnet->host_ask_for_ga)
                    telnet->host_ask_for_ga = 0;
                else
                    telnet_send_option(telnet, TELNET_WILL, TELNET_OP_GA);
                telnet->remote_ask_me_ga = 1;
                telnet->state            = STATE_NORMAL;
                break;
            default:
                telnet_send_option(telnet, TELNET_WONT, *data);
                telnet->state = STATE_NORMAL;
                break;
            };
            break;
        case STATE_DONT:
            if (*data == TELNET_OP_ECHO)
            {
                telnet->remote_ask_me_echo = 0;
                finsh_set_echo(0);
            }
            telnet_send_option(telnet, TELNET_WONT, *data);
            telnet->state = STATE_NORMAL;
            break;

        case STATE_NORMAL:
            if (*data == TELNET_IAC)
                telnet->state = STATE_IAC;
            else
                {
                    rt_err_t res = rt_mutex_take(&telnet->read_lock, RT_WAITING_FOREVER);
                    if (res == RT_EOK)
                    {
                        rt_ringbuffer_putchar(&telnet->rx_ringbuffer, *data);
                        rt_mutex_release(&telnet->read_lock);
                    }
                }
            break;
        }

        data++;
    }
    rx_length = rt_ringbuffer_data_len(&telnet->rx_ringbuffer);

    /* indicate there are reception data */
    if ((rx_length > 0) && (telnet->device.rx_indicate != RT_NULL))
        telnet->device.rx_indicate(&telnet->device,
                                   rx_length);

    return;
}

/* process netconn close */
static void telnet_process_close(struct telnet_session *telnet)
{
    if (telnet->conn)
    {
        /* set console */
        rt_console_set_device(RT_CONSOLE_DEVICE_NAME);
        /* set finsh device */
        finsh_set_device(RT_CONSOLE_DEVICE_NAME);
        /* set log_trace device */
        // log_trace_set_device(RT_CONSOLE_DEVICE_NAME);

        /* close connection */
        netconn_delete(telnet->conn);
        telnet->conn = RT_NULL;

        /* restore shell option */
        finsh_set_echo(telnet->echo_mode);

        telnet_lg_info("resume console to %s\n", RT_CONSOLE_DEVICE_NAME);
    }
}

/* telnet server thread entry */
static void telnet_thread(void *parameter)
{
    rt_err_t result;
    rt_uint32_t event;
    struct netbuf *buf;
    struct netconn *conn;
    struct telnet_session *telnet = &the_telnet;

#ifdef RT_USING_LOGTRACE
    log_trace_register_session(&telnet_lg);
#endif

    rt_mutex_init(&telnet->read_lock, "tel_rx", RT_IPC_FLAG_PRIO);
    rt_mutex_init(&telnet->write_lock, "tel_tx", RT_IPC_FLAG_PRIO);

    /* create network event */
    rt_event_init(&telnet->nw_event, "telnet", RT_IPC_FLAG_FIFO);

    /* Create a new connection identifier. */
    conn = netconn_new(NETCONN_TCP);

    /* Bind connection to well known port number 23. */
    netconn_bind(conn, NULL, TELNET_PORT);

    /* Tell connection to go into listening mode. */
    netconn_listen(conn);

    /* register telnet device */
    telnet->device.type    = RT_Device_Class_Char;
    telnet->device.init    = telnet_init;
    telnet->device.open    = telnet_open;
    telnet->device.close   = telnet_close;
    telnet->device.read    = telnet_read;
    telnet->device.write   = telnet_write;
    telnet->device.control = telnet_control;

    /* no private */
    telnet->device.user_data = telnet;

    /* register telnet device */
    rt_device_register(&telnet->device, TELNET_DEVICE_NAME,
                       RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_STREAM);

    while (1)
    {
        err_t net_err;
        rt_kprintf("\ntelnet server waiting for connection\n");

        telnet_reset(telnet);

        /* Grab new connection. */
        net_err = netconn_accept(conn, &telnet->conn);
        if (telnet->conn == RT_NULL || net_err) continue;

        // netconn_set_nonblocking(telnet->conn, 1);
        tcp_nagle_disable(telnet->conn->pcb.tcp);
        telnet->conn->pcb.tcp->so_options |= SOF_KEEPALIVE;
        telnet->conn->recv_timeout = 10;
        /* set network rx call back */
        telnet->conn->callback = rx_callback;

        /* not a real warning, I just want to log it whatevet the level is. */
        telnet_lg_info("new telnet connection, switch console to telnet...\n");

        /* Process the new connection. */
        /* set console */
        rt_console_set_device(TELNET_DEVICE_NAME);
        /* set log_trace */
        // log_trace_set_device(TELNET_DEVICE_NAME);
        /* set finsh device */
        finsh_set_device(TELNET_DEVICE_NAME);

        telnet->echo_mode = finsh_get_echo();
        // finsh_set_echo(0);

        /* wait 0.5s for passive negotiation terminal. Any negotiation will
         * be handled in telnet_process_rx. */
        result = rt_event_recv(&telnet->nw_event, NW_RX,
                               RT_EVENT_FLAG_AND | RT_EVENT_FLAG_CLEAR,
                               RT_TICK_PER_SECOND / 2, &event);
        if (result == RT_EOK)
        {
            net_err = netconn_recv(telnet->conn, &buf);
            while (net_err == ERR_OK)
            {
                if (buf != RT_NULL)
                {
                    do
                    {
                        rt_uint8_t *data;
                        rt_uint16_t length;

                        netbuf_data(buf, (void **)&data, &length);
                        telnet_process_rx(telnet, data, length);
                    } while (netbuf_next(buf) != -1);

                    netbuf_delete(buf);
                }
                net_err = netconn_recv(telnet->conn, &buf);
            }
        }

        if (finsh_get_echo() == 0)
        {
            /* try to negotiate echo on my side but only turn on the echo when
             * permitted. */
            telnet->host_ask_for_echo = 1;
            telnet_send_option(telnet, TELNET_WILL, TELNET_OP_ECHO);
        }

        /* supress Go Ahead */
        if (!telnet->remote_ask_me_ga)
        {
            telnet->host_ask_for_ga = 1;
            telnet_send_option(telnet, TELNET_WILL, TELNET_OP_GA);
        }
        if (!telnet->remote_ask_for_ga)
        {
            telnet->host_request_ga = 1;
            telnet_send_option(telnet, TELNET_DO, TELNET_OP_GA);
        }

        /* show version */
        rt_show_version();
        /* show hello. */
        rt_kprintf(FINSH_PROMPT);

        while (1)
        {
            /* try to send all data in tx ringbuffer */
            telnet_process_tx(telnet);

            telnet_lg_debug("start receive event\n");
            /* receive network event */
            result = rt_event_recv(&telnet->nw_event, NW_MASK,
                                   RT_EVENT_FLAG_OR | RT_EVENT_FLAG_CLEAR,
                                   RT_TICK_PER_SECOND, &event);
            if (result == RT_EOK)
            {
                /* get event successfully */
                if (event & NW_RX)
                {
                    telnet_lg_info("RX\n");
                    /* do a rx procedure */
                    net_err = netconn_recv(telnet->conn, &buf);
                    while (net_err == ERR_OK)
                    {
                        if (buf != RT_NULL)
                        {
                            do
                            {
                                rt_uint8_t *data;
                                rt_uint16_t length;

                                netbuf_data(buf, (void **)&data, &length);
                                telnet_process_rx(telnet, data, length);
                            } while (netbuf_next(buf) != -1);

                            netbuf_delete(buf);
                        }
                        net_err = netconn_recv(telnet->conn, &buf);
                    }

                    if (net_err != ERR_TIMEOUT)
                    {
                        telnet_lg_info("lose connection in RX\n");
                        /* close connection */
                        telnet_process_close(telnet);
                        break;
                    }
                }

                if (event & NW_TX)
                {
                    telnet_lg_info("TX\n");
                    telnet_process_tx(telnet);
                }

                if (event & NW_CLOSED)
                {
                    telnet_lg_info("CLOSED\n");
                    /* process close */
                    telnet_process_close(telnet);
                    break;
                }
            }
            else if (result == -RT_ETIMEOUT)
            {
                if (telnet->conn->state == NETCONN_CLOSE)
                {
                    telnet_lg_info("netconn close state detected\n");
                    telnet_process_close(telnet);
                    break;
                }
            }
        }
    }
}

/* telnet server */
void telnet_srv(void)
{
    rt_thread_t tid;

    tid = rt_thread_find(TELNET_THREAD_NAME);
    if (tid != RT_NULL) return;

    tid = rt_thread_create(TELNET_THREAD_NAME, telnet_thread, RT_NULL, 2048, 25,
                           5);
    if (tid != RT_NULL) rt_thread_startup(tid);
}

#ifdef RT_USING_FINSH
#include <finsh.h>
FINSH_FUNCTION_EXPORT(telnet_srv, startup telnet server);
FINSH_FUNCTION_EXPORT_ALIAS(telnet_srv, __cmd_telnetd, start telnet server);
#endif
