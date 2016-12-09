#include <lwip/sockets.h>
#include "stream_pack.h"

#define DSP_SHARE_BUF_LEN          0x180000

static int g_cancel = 0;
static unsigned char g_rec_pool_buf[DSP_SHARE_BUF_LEN];

#ifdef WRITE_FILE
static FILE *g_record_file = NULL;
#endif


static void send_stream_to_vlc(struct vlc_info vlc_info, unsigned char *data, int size)
{
#ifdef WRITE_FILE
	if (g_record_file) {
		fwrite(data, 1, size, g_record_file);
	}
#else
	int ret, sendbytes = 0;
	struct sockaddr_in vlc_addr;

	if (vlc_info.server_sock != -1)
	{
		bzero(&vlc_addr, sizeof(struct sockaddr_in));
		vlc_addr.sin_family = AF_INET;
		vlc_addr.sin_addr.s_addr = inet_addr(vlc_info.ip);
		vlc_addr.sin_port = htons(vlc_info.port);

		while (size > 0)
		{
			int tosend = 1024;
			if (size < tosend)
				tosend = size;

			ret = sendto(vlc_info.server_sock, (char *)data + sendbytes, tosend, 0, (struct sockaddr *)&vlc_addr, sizeof(struct sockaddr_in));

			if (ret < 0)
			{
				printf("[ERROR]: send(%d, %d) failed,%d,%s\n", vlc_info.server_sock, tosend, ret, strerror(errno));
			}
			else 
			{
				sendbytes += ret;
				size -= ret;
			}
		}
	}
#endif
}

static void send_thread_proc(void *arg)
{
	struct rec_pool_info *pool;
	int has_data;
	int r, w;
	struct vlc_info *vlc_info = (struct vlc_info *) arg;

	if (vlc_info->server_sock == -1) {
		vlc_info->server_sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

		if (vlc_info->server_sock == -1)
		{
			perror("[ERROR]: create stream server socket\n");
			return ;
		}
	}

	while (!g_cancel)
	{
		has_data = 0;
		pool = g_vlcview_info.rec_pool + vlc_info->channel;

		rt_mutex_take(&vlc_info->rec_pool_lock, RT_WAITING_FOREVER);
		r = pool->read_index;
		w = pool->write_index;
		if (r != w)
		{
			if (w > r)
			{
				send_stream_to_vlc(*vlc_info, pool->addr + r, w - r);
			}
			else
			{
				send_stream_to_vlc(*vlc_info, pool->addr + r, pool->total_len - r);
				send_stream_to_vlc(*vlc_info, pool->addr, w);
			}
			has_data = 1;
			pool->read_index = w;
		}
		rt_mutex_release(&vlc_info->rec_pool_lock);

		if (has_data == 0)
		{
			rt_thread_sleep(1);
		}
	}
	return;
}

static int init_rec_pool(void)
{
	int i;
	unsigned char *p;

	g_vlcview_info.channel_count = 2;
	g_vlcview_info.rec_pool[0].total_len = 1024*1024;
	g_vlcview_info.rec_pool[1].total_len = 512*1024;

	for (i = 0; i < g_vlcview_info.channel_count; i++)
	{
		g_vlcview_info.rec_pool[i].write_index = 0;
		g_vlcview_info.rec_pool[i].read_index = 0;
	}

	p = g_rec_pool_buf;
	for(i = 0; i < g_vlcview_info.channel_count; i++)
	{
		g_vlcview_info.rec_pool[i].addr = p;
		p += g_vlcview_info.rec_pool[i].total_len;
	}

	if (p > g_rec_pool_buf + DSP_SHARE_BUF_LEN) 
	{
		printf("Error: rec pool address out of buf! need %d bytes, but have %d bytes\n",p - g_rec_pool_buf, DSP_SHARE_BUF_LEN); 
		return -1; 
	}

	return 0;
}

int vlcview_pes_init(void)
{
	int ret;
	int i;

	g_cancel = 0;
	ret = init_rec_pool();
	if (ret != 0)
		return ret;
	ret = init_stream_pack();
	if (ret != 0)
		return ret;

	for (i = 0; i < MAX_ENC_CHAN; i++) {
		g_vlcview_info.vlc_info[i].channel = -1;
		g_vlcview_info.vlc_info[i].server_sock = -1;
		g_vlcview_info.vlc_info[i].port = 0;
		g_vlcview_info.vlc_info[i].thread_id = 0;
		rt_mutex_init(&g_vlcview_info.vlc_info[i].rec_pool_lock,
			          "app_mutex",
			          RT_IPC_FLAG_PRIO);
	}

#ifdef WRITE_FILE
	time_t now;
	struct timeval tv;
	struct tm t;
	char fname[256];

	gettimeofday(&tv, NULL);
	now = tv.tv_sec + 8*3600/*convert to BeiJing TimeZone*/;
	gmtime_r(&now, &t);
	sprintf(fname, "/mnt/ch0_%02d-%02d_%02d-%02d-%02d.264",
		t.tm_mon+1, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec);

	g_record_file = fopen(fname, "wb");
#endif

	return 0;
}

int vlcview_pes_uninit(void)
{
	int i;

	g_cancel = 1;

	for (i = 0; i < MAX_ENC_CHAN; i++) {
		if (g_vlcview_info.vlc_info[i].server_sock != -1)
			lwip_close(g_vlcview_info.vlc_info[i].server_sock);
		if (g_vlcview_info.vlc_info[i].thread_id != 0)
			rt_thread_delete(g_vlcview_info.vlc_info[i].thread_id);
	}

	return 0;
}

int vlcview_pes_send_to_vlc(int channel, char *ip, int port)
{
	char thread_name[20];

	g_vlcview_info.vlc_info[channel].channel = channel;
	strcpy(g_vlcview_info.vlc_info[channel].ip, ip);
	g_vlcview_info.vlc_info[channel].port = port;

	snprintf(thread_name, 20, "pes_to_vlc_chn_%d", channel);
	g_vlcview_info.vlc_info[channel].thread_id = rt_thread_create(
		thread_name,
		send_thread_proc,
		&g_vlcview_info.vlc_info[channel],
		4 * 1024,
		RT_APP_THREAD_PRIORITY,
		10);

	rt_thread_startup(g_vlcview_info.vlc_info[channel].thread_id);

	return 0;
}

