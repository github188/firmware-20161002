#ifndef __STREAM_PACK_H__
#define __STREAM_PACK_H__

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <rtthread.h>
#include <rtdef.h>
#include "libvlcview.h"
#include "PSMuxLib.h"

#define MAX_ENC_CHAN            2
#define MAX_STREAM_ELEMENT_LEN  0x80000

struct rec_pool_info {
	unsigned char *addr;
	unsigned int write_index;
	unsigned int read_index;
	unsigned int total_len;
};

struct vlc_info {
	int channel;
	int server_sock;
	int port;
	rt_thread_t thread_id;
	struct rt_mutex rec_pool_lock;
	char ip[80];
};

struct vlcview_info {
	unsigned int channel_count;
	struct rec_pool_info rec_pool[MAX_ENC_CHAN];
	struct vlc_info  vlc_info[MAX_ENC_CHAN];
};

struct vlcview_info g_vlcview_info;

int init_stream_pack();
int deinit_stream_pack();
#endif /* __DEMO_DSP_COMMON_H__ */

