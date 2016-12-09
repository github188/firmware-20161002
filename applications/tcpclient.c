/*
 * File      : tcpclient.c
 *             A TCP client example
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2006 - 2016, RT-Thread Development Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Change Logs:
 * Date           Author       Notes
 * 2016-09-08     Bernard      the first version
 */

#include <rtthread.h>

#include <lwip/netdb.h>
#include <lwip/sockets.h>
#include <lwip/netif.h>

#include <finsh.h>

int tcpclient(int argc, char** argv)
{
    int sock;
    int port;
    int tick;

    struct sockaddr_in server_addr;
    char *str;

    if (argc != 4)
    {
        rt_kprintf("Usage: tcpclient host port string\n");
        return -1;
    }

    if (netif_default == RT_NULL)
    {
        /* no network interface */
        rt_kprintf("****************************************\n");
        rt_kprintf("\n");
        rt_kprintf("Wi-Fi failed!!!\n");
        rt_kprintf("Wi-Fi failed!!!\n");
        rt_kprintf("Wi-Fi failed!!!\n");
        rt_kprintf("\n");
        rt_kprintf("****************************************\n");

        return -1;
    }

    /* wait for IP address ready */
    tick = rt_tick_get();
    do
    {
        rt_thread_delay(rt_tick_from_millisecond(200));

        if (rt_tick_get() - tick > rt_tick_from_millisecond(1000 * 30))
            break;
    } while ((netif_default->ip_addr.addr == 0));

    if (!(netif_default->ip_addr.addr))
    {
        rt_kprintf("****************************************\n");
        rt_kprintf("\n");
        rt_kprintf("DHCP failed!!!\n");
        rt_kprintf("DHCP failed!!!\n");
        rt_kprintf("DHCP failed!!!\n");
        rt_kprintf("\n");
        rt_kprintf("****************************************\n");

        return -1;
    }

    list_if();

    port = atoi(argv[2]);
    str = argv[3];

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        rt_kprintf("Socket error\n");

        return -1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(argv[1]);;
    rt_memset(&(server_addr.sin_zero), 0, sizeof(server_addr.sin_zero));

    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) == -1)
    {
        rt_kprintf("****************************************\n");
        rt_kprintf("\n");
        rt_kprintf("Connect fail!\n");
        rt_kprintf("Connect fail!\n");
        rt_kprintf("Connect fail!\n");
        rt_kprintf("\n");
        rt_kprintf("****************************************\n");

        lwip_close(sock);

        return 0;
    }

    /* send data then close the connection */
    send(sock, str, rt_strlen(str), 0);

    lwip_close(sock);

    return 0;
}
MSH_CMD_EXPORT(tcpclient, startup tcp client);
