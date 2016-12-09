#include "stream_pack.h"

#define PACK_OUT_BUF_LEN    0x80000
#define MAX_NALU_NUM 		16
#define MAX_BYTE_RATE       (256*1024)
#define BPS_VBR				0
#define BPS_CBR				1
#define I_FRAME_MODE        0x00001001
#define P_FRAME_MODE        0x00001003

struct stream_pack_info {
	unsigned int channel_id;
	void *ps_handle;
	PSMUX_PARAM ps_mux_param;
	unsigned int nalu_num;
	struct vlcview_h264_nalu nalu[MAX_NALU_NUM];
	unsigned int frame_len;
	unsigned int picture_mode;
	unsigned long long time_stamp;
	unsigned int frame_num;
};

static struct stream_pack_info g_stream_pack_info[2];
static unsigned char g_pack_out_buf[PACK_OUT_BUF_LEN];

int init_stream_pack()
{
	unsigned int chan;

	memset((void *)g_stream_pack_info, 0, sizeof(g_stream_pack_info));
	for (chan=0; chan < g_vlcview_info.channel_count; chan++) {
		g_stream_pack_info[chan].channel_id = chan;
	}

	return 0;
}

int deinit_stream_pack()
{
	return 0;
}

static void set_psmux_info(PSMUX_ES_INFO *pes_info)
{
	pes_info->video_stream_type = STREAM_TYPE_VIDEO_H264;
	pes_info->max_packet_len = MAX_PES_PACKET_LEN;
	pes_info->set_frame_end_flg = 1;
	pes_info->max_byte_rate = MAX_BYTE_RATE;
}

static int create_psmux_handle(struct stream_pack_info *pack_info)
{
 	PSMUX_PARAM *psmux_param = &pack_info->ps_mux_param;

	pack_info->ps_handle = NULL;
	if(psmux_param->buffer && psmux_param->buffer_size)
	{
		rt_free(psmux_param->buffer);
		psmux_param->buffer = NULL;
		psmux_param->buffer_size = 0;
	}

	set_psmux_info(&psmux_param->info);

	PSMUX_GetMemSize(psmux_param);
	if ((psmux_param->buffer = rt_malloc(psmux_param->buffer_size)) == NULL)
		return -1;

	memset(psmux_param->buffer, 0, psmux_param->buffer_size);

	if (PSMUX_Create(psmux_param, &pack_info->ps_handle) != PSMUX_LIB_S_OK)
	{
		pack_info->ps_handle = NULL;
		return -1;
	}

	return 0;
}

static int send_stream_to_rec_pool(unsigned int chan, unsigned char *stream_data, unsigned int len)
{
	unsigned int w,r;
	int err = 0;
	unsigned int spare_len,part1,part2;
	unsigned int total_len;
	char *p;
	struct rec_pool_info *rec_pool;

	if (chan < g_vlcview_info.channel_count)
		rec_pool = &(g_vlcview_info.rec_pool[chan]);
	else
		return -1;

	if (NULL == stream_data || 0 == len || 0 == rec_pool->total_len)
		return -2;

	if (rec_pool->addr == NULL)
		return -3;

	rt_mutex_take(&g_vlcview_info.vlc_info[chan].rec_pool_lock, RT_WAITING_FOREVER);

	if (rec_pool->total_len)
	{
		w = rec_pool->write_index;
		r = rec_pool->write_index; //不判断读指针，循环覆盖
		total_len = rec_pool->total_len;
		p = (char *)rec_pool->addr;
		spare_len = (r + total_len - w - 1) % total_len;

		if (len > spare_len)
		{
			printf("libvlcview: lost one frame\n");
			goto exit;
		}
		part1 = total_len - w;
		if (len > part1)
		{
			memcpy((unsigned char *)(p + w), (unsigned char *)stream_data, part1);
			part2 = len - part1;
			memcpy((unsigned char *)p, (unsigned char *)stream_data + part1, part2);
		}
		else
		{
			memcpy((unsigned char *)(p+w), (unsigned char *)stream_data, len);
		}
		rec_pool->write_index = (rec_pool->write_index + len) % total_len;
	}
exit:
	rt_mutex_release(&g_vlcview_info.vlc_info[chan].rec_pool_lock);
	return err;
}

static unsigned int video_ps_pack(PSMUX_PROCESS_PARAM psmux_param, struct stream_pack_info *pack_info)
{
	int j =0, ret = 0;
	unsigned int chan;
	unsigned int ps_len = 0;
	unsigned char *out_buf = g_pack_out_buf;

	if(NULL == pack_info)
		return 0;

	chan = pack_info->channel_id;

	psmux_param.encrypt = 0;
	for (j = 0; j < pack_info->nalu_num; j++)
	{
		psmux_param.unit_in_buf = pack_info->nalu[j].start;
		psmux_param.unit_in_len = pack_info->nalu[j].len;
		psmux_param.out_buf = out_buf;
		psmux_param.out_buf_len = 0;
		psmux_param.out_buf_size = MAX_STREAM_ELEMENT_LEN;
		psmux_param.is_unit_start = 1;
		psmux_param.is_unit_end  = 1;
		psmux_param.is_first_unit = (j == 0);
		psmux_param.is_last_unit = (j == (pack_info->nalu_num - 1));

		ret = PSMUX_Process(pack_info->ps_handle, &psmux_param);
		if (ret == PSMUX_LIB_S_OK)
		{
			ps_len += psmux_param.out_buf_len;
			out_buf += psmux_param.out_buf_len;
		}
		else {
			printf("libvlcview: create pes packet for nalu[%d] failed\n", j);
		}
	}

	if(ps_len & 3)
	{
		ps_len += (4 - (ps_len & 3));
	}
	send_stream_to_rec_pool(chan, g_pack_out_buf, ps_len);

	return ps_len;
}

static int do_stream_pack(struct stream_pack_info *pack_info)
{
	unsigned int chan = pack_info->channel_id;
	unsigned int output_len = 0;
	unsigned long long clock;

	if (pack_info->nalu_num > 5 || pack_info->frame_len > MAX_STREAM_ELEMENT_LEN)
		return 0;
	
	clock = pack_info->time_stamp;

	PSMUX_PROCESS_PARAM psmux_param;
	if(pack_info->ps_handle != NULL && (g_vlcview_info.rec_pool[chan].total_len > 0))
	{
		memset (&psmux_param, 0, sizeof(psmux_param));
		if (pack_info->picture_mode == I_FRAME_MODE)
		{
			psmux_param.is_key_frame = 1;
			psmux_param.frame_type = FRAME_TYPE_VIDEO_IFRAME;
			psmux_param.sys_clk_ref = clock;
			psmux_param.ptime_stamp = clock;
		}
		else if (pack_info->picture_mode == P_FRAME_MODE)
		{
			psmux_param.is_key_frame = 0;
			psmux_param.frame_type = FRAME_TYPE_VIDEO_PFRAME;
			psmux_param.sys_clk_ref = clock;
			psmux_param.ptime_stamp = clock;
		}
		else
		{
			return -1;
		}

		psmux_param.frame_num = pack_info->frame_num;
		output_len = video_ps_pack(psmux_param, pack_info);
	}
	else
	{
		printf("libvlcview: Lost Record Frame\n");
	}
	return 0;
}

int vlcview_pes_stream_pack(int channel, struct vlcview_enc_stream_element stream_element)
{
	RT_ASSERT(channel <= 3);
	unsigned int i = 0;
    unsigned long long clock_90K;
    int err;

	clock_90K = stream_element.time_stamp * 9 /100;
	if (stream_element.frame_type == VLCVIEW_ENC_H264_I_FRAME)
	{
		g_stream_pack_info[channel].picture_mode = I_FRAME_MODE;
	}
	else
	{
		g_stream_pack_info[channel].picture_mode = P_FRAME_MODE;
	}

    g_stream_pack_info[channel].time_stamp = clock_90K;
	g_stream_pack_info[channel].frame_num++;
	g_stream_pack_info[channel].frame_len = stream_element.frame_len;
	g_stream_pack_info[channel].nalu_num = stream_element.nalu_count;

	for (i = 0; i < g_stream_pack_info[channel].nalu_num; i++)
	{
		g_stream_pack_info[channel].nalu[i].start = stream_element.nalu[i].start;
		g_stream_pack_info[channel].nalu[i].len = stream_element.nalu[i].len;
	}

	if (g_stream_pack_info[channel].ps_handle == NULL) {
		err = create_psmux_handle(&g_stream_pack_info[channel]);
		if (err)
		{
			printf("create_psmux_handle ERROR!!!, chan=%d", g_stream_pack_info[channel].channel_id);
			return err;
		}
	}

	return do_stream_pack(&g_stream_pack_info[channel]);
}

