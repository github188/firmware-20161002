#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <finsh.h>
#include "types/type_def.h"
#include "dsp/fh_system_mpi.h"
#include "dsp/fh_vpu_mpi.h"
#include "dsp/fh_venc_mpi.h"
#include "dsp/fh_jpeg_mpi.h"
#include "di/di_config.h"
#include "sample_common_isp.h"
#include "libvlcview.h"

struct channel_info {
	FH_UINT32 width;     /* align to 16 */
	FH_UINT32 height;    /* align to 16 */
	FH_UINT32 framerate;
	FH_UINT32 bps;
};

static struct channel_info g_channels[] = {
	{
		.width = 1280,
		.height = 720,
		.framerate = 0x00010030,
		.bps = 2048 * 1024
	},
	{
		.width = 352,
		.height = 288,
		.framerate = 0x00010010,
		.bps = 512 * 1024
	}
};

static FH_SINT32 g_exit = 1;
static rt_thread_t g_thread_isp = 0;
static rt_thread_t g_thread_dbi = 0;
static rt_thread_t g_thread_stream = 0;

void change_resolution()
{
	FH_VPU_CHN_CONFIG chn_attr;
	FH_CHR_CONFIG cfg_param;
	FH_SINT32 channel = 0;
	FH_SINT32 ret;

	printf("Change resolution to %d x %d\n\n",
		g_channels[channel].width, g_channels[channel].height);

	ret = FH_VPSS_Disable();
	if (ret != RETURN_OK) {
		printf("Error: FH_VPSS_Disable failed with %d\n", ret);
		return;
	}

	ret = FH_VENC_StopRecvPic(channel);
	if (ret != RETURN_OK) {
		printf("Error: FH_VENC_StopRecvPic failed with %d\n", ret);
		return;
	}

	chn_attr.vpu_chn_size.u32Width = g_channels[channel].width;
	chn_attr.vpu_chn_size.u32Height = g_channels[channel].height;
	ret = FH_VPSS_SetChnAttr(channel, &chn_attr);
	if (ret != RETURN_OK) {
		printf("Error: FH_VPSS_SetChnAttr failed with %d\n", ret);
		return;
	}

	ret = FH_VPSS_OpenChn(channel);
	if (ret != RETURN_OK) {
		printf("Error: FH_VPSS_OpenChn failed with %d\n", ret);
		return;
	}

	cfg_param.chn_attr.profile = FH_PRO_MAIN;
	cfg_param.chn_attr.i_frame_intterval = 30;
	cfg_param.chn_attr.size.u32Width = g_channels[channel].width;
	cfg_param.chn_attr.size.u32Height = g_channels[channel].height;
	cfg_param.rc_config.bitrate = g_channels[channel].bps;
	cfg_param.init_qp = 28;
	cfg_param.rc_config.ImaxQP = 35;
	cfg_param.rc_config.IminQP = 3;
	cfg_param.rc_config.PmaxQP = 39;
	cfg_param.rc_config.PminQP = 3;
	cfg_param.rc_config.RClevel = FH_RC_LOW;
	cfg_param.rc_config.RCmode = FH_RC_FIXEQP;
	cfg_param.rc_config.max_delay = 8;
	cfg_param.rc_config.FrameRate.frame_count = (g_channels[channel].framerate & 0xffff);
	cfg_param.rc_config.FrameRate.frame_time  = (g_channels[channel].framerate >> 16);
	ret = FH_VENC_SetChnAttr(channel, &cfg_param);
	if (ret != RETURN_OK) {
		printf("Error: FH_VENC_SetChnAttr failed with %d\n", ret);
		return;
	}

	ret = FH_VENC_StartRecvPic(channel);
	if (ret != RETURN_OK) {
		printf("Error: FH_VENC_StartRecvPic failed with %d\n", ret);
		return;
	}

	ret = FH_SYS_BindVpu2Enc(channel, channel);
	if (ret != RETURN_OK) {
		printf("Error: FH_SYS_BindVpu2Enc failed with %d\n", ret);
		return;
	}
	
	ret = FH_VPSS_Enable(0);
	if (ret != RETURN_OK) {
		printf("Error: FH_VPSS_Enable failed with %d\n", ret);
		return;
	}
}

void change_fps()
{
	FH_SINT32 fps;
	FH_SINT32 ret;

	ret = FH_VPSS_Disable();
	if (ret != RETURN_OK) {
		printf("Error: FH_VPSS_Disable failed with %d\n", ret);
		return;
	}

	fps = sample_isp_change_fps();

	ret = FH_VPSS_Enable(0);
	if (ret != RETURN_OK) {
		printf("Error: FH_VPSS_Enable failed with %d\n", ret);
		return;
	}

	printf("Change current fps to %d\n\n", fps);
}

void rotate()
{
	FH_ROTATE rotate_info;
	FH_SINT32 channel = 0;
	static FH_UINT32 rotate_value = 0;
	FH_SINT32 ret;

	rotate_value = (rotate_value + 1) % 4;	
	rotate_info.enable = 1;
	rotate_info.rotate = rotate_value;
	ret = FH_VENC_SetRotate(channel, &rotate_info);
	if (ret != RETURN_OK) {
		printf("Error: FH_VENC_SetRotate failed with %d\n", ret);
	}

	printf("Rotate to %d degree\n\n", rotate_value * 90);
}

void capture_jpeg()
{
	FH_JPEG_CONFIG jpeg_c;
	FH_JPEG_STREAM_INFO jpeg_stream = {0};
	FH_SINT32 channel = 1;
	FH_SINT32 ret;
	static FH_SINT32 jpeg_inited = 0;

	if (!jpeg_inited) {
		ret = FH_JPEG_InitMem(1280, 720);
		if (ret != RETURN_OK) {
			printf("Error: FH_JPEG_InitMem failed with %d\n", ret);
			return;
		}
		jpeg_inited = 1;
	}

	jpeg_c.QP = 50;
	jpeg_c.rate = 0x70;
	ret = FH_JPEG_Setconfig(&jpeg_c);
	if (ret != RETURN_OK) {
		printf("Error: FH_JPEG_Setconfig failed with %d\n", ret);
		return;
	}

	ret = FH_SYS_BindVpu2Jpeg(channel);
	if (ret == RETURN_OK) {
		ret = FH_JPEG_Getstream_Block(&jpeg_stream);
		if (ret == RETURN_OK) {
			if (jpeg_stream.stream.size != 0) {
				printf("CAPTURE JPEG: addr=%p size=%d\n",
					jpeg_stream.stream.addr, jpeg_stream.stream.size);
			}
		} else {
			printf("Error: FH_JPEG_Getstream_Block failed with %d\n", ret);
			return;
		}
	} else {
		printf("Error: FH_SYS_BindVpu2Jpeg failed with %d\n", ret);
		return;
	}
}

void toggle_freeze()
{
	static FH_SINT32 on = 0;
	FH_SINT32 ret;

	on = !on;
	if (on) {
		printf("Freeze Video\n\n");
		ret = FH_VPSS_FreezeVideo();
		if (ret != RETURN_OK) {
			printf("Error: FH_VPSS_FreezeVideo failed with %d\n", ret);
		}
	}
	else {
		printf("Unfreeze Video\n\n");
		ret = FH_VPSS_UnfreezeVideo();
		if (ret != RETURN_OK) {
			printf("Error: FH_VPSS_UnfreezeVideo failed with %d\n", ret);
		}
	}
}

void change_8X8_to_16X16(FH_UINT8 *src, FH_UINT8 *dst)
{
	FH_SINT32 h, w, numblock;

	for (numblock = 0; numblock < 4; numblock++) {
		for (h = 0; h < 8; h++) {
			for (w = 0; w < 8; w++) {
				*(dst + numblock / 2 * 8 * 8 * 2 + 2 * 8 * h + numblock % 2 * 8 + w) = *(src + numblock * 8 * 8 + h * 8 + w);
			}
		}
	}
}

void yuv_transform(FH_UINT8 *src_y, FH_UINT8 *src_c, FH_UINT8 *dst, FH_UINT32 w, FH_UINT32 h)
{
	FH_SINT32 w_align, h_align;
	FH_SINT32 w_align32, h_align32;
	FH_SINT32 i, j, numblock;
	FH_UINT8 orderbuf[8*8*4];
	FH_UINT8 *ybuf, *cbuf, *yuvbuf;
	FH_UINT8 *ubuf, *vbuf;

	w_align = (w + 15)&(~15);
	h_align = (h + 15)&(~15);
	w_align32 = (w + 31)&(~31);
	h_align32 = (h + 31)&(~31);

	ybuf = src_y;
	cbuf = src_c;
	yuvbuf = dst;

	// Y
	for (numblock = 0; numblock < (w_align*h_align/16/16); numblock++) {
		change_8X8_to_16X16(ybuf + 8*8*4 *numblock, orderbuf);
		for (i = 0; i < 16; i++) {
			for (j = 0; j < 16; j++) {
				*(yuvbuf + numblock / (w_align / 16) * w * 16 + numblock %( w_align / 16 ) * 16 + i * w + j) =*(orderbuf + i * 16 +j) ;
			}
		}
	}

	// UV
	ubuf = yuvbuf + w*h;
	vbuf = yuvbuf + + w*h + w*h/4;
	for (numblock = 0; numblock < (w_align*h_align/16/16); numblock++) {
		for (i = 0; i < 8; i++) {
			for (j = 0; j < 8; j++) {
				*(ubuf + numblock  / (w_align / 16) * w / 2 * 8 + numblock %( w_align / 16 ) * 8 + i * w / 2 + j) = *(cbuf + numblock * 8 * 8 * 2 + i * 8 + j);
				*(vbuf + numblock  / (w_align / 16) * w / 2 * 8 + numblock %( w_align / 16 ) * 8 + i * w / 2 + j) = *(cbuf + numblock * 8 * 8 * 2 + 8 * 8 + i * 8 + j);
			}
		}
	}
}

void capture_yuv()
{
	FILE *yuv_file;
	FH_VPU_STREAM yuv_data;
	FH_UINT8 *dst;
	FH_SINT32 ret;
	FH_SINT32 chan = 1;

	dst = rt_malloc(g_channels[chan].width * g_channels[chan].height * 3 / 2);
	if (dst == NULL) {
		printf("Error: failed to allocate yuv transform buffer\n");
		return;
	}

	ret = FH_VPSS_GetChnFrame(chan, &yuv_data);
	if (ret == RETURN_OK) {
		yuv_transform(yuv_data.yluma.vbase, yuv_data.chroma.vbase, dst, g_channels[chan].width, g_channels[chan].height);
		printf("GET CHN %d YUV DATA w:%d h:%d OK\n\n",
			chan, g_channels[chan].width , g_channels[chan].height);
	} else {
		printf("Error: FH_VPSS_GetChnFrame failed with %d\n\n", ret);
	}
	rt_free(dst);
}

void sample_venc_exit()
{
	rt_thread_t exit_process;

	exit_process = rt_thread_find("vlc_get_stream");
	if(exit_process)
		rt_thread_delete(exit_process);

	API_ISP_Exit();
	exit_process = rt_thread_find("vlc_isp");
	if(exit_process)
		rt_thread_delete(exit_process);

#if ENABLE_COOLVIEW
	rt_thread_sleep(100);
	exit_process = rt_thread_find("coolview");
	if(exit_process)
		rt_thread_delete(exit_process);
#endif

	FH_SYS_Exit();
}

void sample_venc_get_stream_proc(void *arg)
{
	FH_SINT32 i, j;
	FH_SINT32 ret;
	FH_ENC_STREAM_ELEMENT stream;
	struct vlcview_enc_stream_element stream_element;

	while (!g_exit) {
		for (i = 0; i < 2; i++) {
			do {
				ret = FH_VENC_GetStream(i, &stream);
				if (ret == RETURN_OK) {
					stream_element.frame_type = stream.frame_type;
					stream_element.frame_len = stream.length;
					stream_element.time_stamp = stream.time_stamp;
					stream_element.nalu_count = stream.nalu_cnt;
					for (j = 0; j < stream_element.nalu_count; j++) {
						stream_element.nalu[j].start = stream.nalu[j].start;
						stream_element.nalu[j].len = stream.nalu[j].length;
					}
					vlcview_pes_stream_pack(stream.chan, stream_element);
					FH_VENC_ReleaseStream(i);
				}
			} while (ret == RETURN_OK);
		}
		rt_thread_sleep(1);
	}

	return;
}

void print_func_help()
{
	printf("Available function:\n");
	printf("   venc_func(\"h\")    print help info\n");
	printf("   venc_func(\"w\")    change resolution\n");
	printf("   venc_func(\"e\")    change frame rate\n");
	printf("   venc_func(\"r\")    rotate\n");
	printf("   venc_func(\"j\")    capture jpeg\n");
	printf("   venc_func(\"f\")    toggle freeze/unfreeze\n");
	printf("   venc_func(\"y\")    save one frame yuv data to file\n");
}

int venc(char* target_ip, rt_uint32_t port_no)
{
	FH_VPU_SIZE vi_pic;
	FH_VPU_CHN_CONFIG chn_attr;
	FH_CHR_CONFIG cfg_param;
	FH_SINT32 i;
	FH_SINT32 ret;

	if (!g_exit) {
		printf("vicview is running!\n");
		return 0;
	}

	g_exit = 0;

#if defined(CONFIG_CHIP_FH8620) || defined(CONFIG_CHIP_FH8620G)
	bufferInit((unsigned char *)FH_SDK_MEM_START, FH_SDK_MEM_SIZE);
#elif defined CONFIG_CHIP_FH8810
	bufferInit((unsigned char *)0xa1000000, (32 * 1024 * 1024));
#endif

	/******************************************
	 step  1: init media platform
	******************************************/
	ret = FH_SYS_Init();
	if (RETURN_OK != ret) {
		printf("Error: FH_SYS_Init failed with %d\n", ret);
		goto err_exit;
	}

	/******************************************
	 step  2: init ISP, and then start ISP process thread
	******************************************/

	if (sample_isp_init() != 0) {
		goto err_exit;
	}
	g_thread_isp = rt_thread_create("vlc_isp",
		                            sample_isp_proc,
		                            &g_exit,
		                            3*1024,
		                            RT_APP_THREAD_PRIORITY,
		                            10);
	rt_thread_startup(g_thread_isp);

#ifdef ENABLE_COOLVIEW
	/******************************************
	 step  2.1: start debug interface thread
	******************************************/
	rt_thread_t debug_interface_proc;
	DI_Config dic;
	dic.type = DIT_ETH;
	dic.conf.eth.debug_port = 8888;
	strcpy(dic.conf.eth.debug_ip, "0.0.0.0");
	DebugInterface_Init(&dic);
	debug_interface_proc = rt_thread_create("coolview",
		                                    (void*)DebugInterface_MainLoop,
		                                    &g_exit,
		                                    3*1024,
		                                    RT_APP_THREAD_PRIORITY + 10,
		                                    20);
	if (debug_interface_proc != RT_NULL) {
		rt_thread_startup(debug_interface_proc);
	} else {
		printf("error: g_dbi creat failed...\n");
		goto err_exit;
	}
#endif

	/******************************************
	 step  3: set vpss resolution
	******************************************/
	vi_pic.vi_size.u32Width = 1280;
	vi_pic.vi_size.u32Height = 720;
	ret = FH_VPSS_SetViAttr(&vi_pic);
	if (RETURN_OK != ret) {
		printf("Error: FH_VPSS_SetViAttr failed with %d\n", ret);
		goto err_exit;
	}

	/******************************************
	 step  4: start vpss
	******************************************/
	ret = FH_VPSS_Enable(0);
	if (RETURN_OK != ret) {
		printf("Error: FH_VPSS_Enable failed with %d\n", ret);
		goto err_exit;
	}

	/******************************************
	 step  5: create 2 venc channels
	******************************************/
	for (i = 0; i < 2; i++) {
		/******************************************
		 step 5.1 configure vpss channel
		******************************************/
		chn_attr.vpu_chn_size.u32Width = g_channels[i].width;
		chn_attr.vpu_chn_size.u32Height = g_channels[i].height;
		ret = FH_VPSS_SetChnAttr(i, &chn_attr);
		if (RETURN_OK != ret) {
			printf("Error: FH_VPSS_SetChnAttr failed with %d\n", ret);
			goto err_exit;
		}

		/******************************************
		 step 5.2 open vpss channel
		******************************************/
		ret = FH_VPSS_OpenChn(i);
		if (RETURN_OK != ret) {
			printf("Error: FH_VPSS_OpenChn failed with %d\n", ret);
			goto err_exit;
		}

		/******************************************
		 step 5.3 create venc channel
		******************************************/
		cfg_param.chn_attr.size.u32Width = g_channels[i].width;
		cfg_param.chn_attr.size.u32Height = g_channels[i].height;
		cfg_param.rc_config.bitrate = g_channels[i].bps;
		cfg_param.rc_config.FrameRate.frame_count = (g_channels[i].framerate & 0xffff);
		cfg_param.rc_config.FrameRate.frame_time  = (g_channels[i].framerate >> 16);
		cfg_param.chn_attr.profile = FH_PRO_MAIN;
		cfg_param.chn_attr.i_frame_intterval = 30;
		cfg_param.init_qp = 28;
		cfg_param.rc_config.ImaxQP = 35;
		cfg_param.rc_config.IminQP = 3;
		cfg_param.rc_config.PmaxQP = 39;
		cfg_param.rc_config.PminQP = 3;
		cfg_param.rc_config.RClevel = FH_RC_LOW;
		cfg_param.rc_config.RCmode = FH_RC_FIXEQP;
		cfg_param.rc_config.max_delay = 8;

		ret = FH_VENC_CreateChn(i, &cfg_param);
		if (RETURN_OK != ret) {
			printf("Error: FH_VENC_CreateChn failed with %d\n", ret);
			goto err_exit;
		}

		/******************************************
		 step 5.4 start venc channel
		******************************************/
		ret = FH_VENC_StartRecvPic(i);
		if (RETURN_OK != ret) {
			printf("Error: FH_VENC_StartRecvPic failed with %d\n", ret);
			goto err_exit;
		}

		/******************************************
		 step 5.5 bind vpss channel with venc channel
		******************************************/
		ret = FH_SYS_BindVpu2Enc(i, i);
		if (RETURN_OK != ret) {
			printf("Error: FH_SYS_BindVpu2Enc failed with %d\n", ret);
			goto err_exit;
		}
	}

	ret = vlcview_pes_init();
	if (0 != ret) {
		printf("Error: vlcview_pes_init failed with %d\n", ret);
		goto err_exit;
	}
	vlcview_pes_send_to_vlc(0, target_ip, port_no);
	vlcview_pes_send_to_vlc(1, target_ip, port_no + 1);

	/******************************************
	 step 6: get stream, then save it to file
	******************************************/
	g_thread_stream = rt_thread_create("vlc_get_stream",
		                               sample_venc_get_stream_proc,
		                               RT_NULL,
		                               3*1024,
		                               RT_APP_THREAD_PRIORITY,
		                               20);
	if (g_thread_stream != RT_NULL) {
		rt_thread_startup(g_thread_stream);
	} else {
		printf("error: vlc_get_stream creat failed...\n");
		goto err_exit;
	}

	return 0;

err_exit:
	g_exit = 1;
	sample_venc_exit();
	vlcview_pes_uninit();
	deinit_stream_pack();

	return -1;
}

int venc_exit()
{
	if (!g_exit) {
		g_exit = 1;
		sample_venc_exit();
		vlcview_pes_uninit();
		deinit_stream_pack();
	} else {
		printf("vicview is not running!\n");
	}

	return 0;
}

void venc_func(char *cmd)
{
	if (strcmp(cmd, "h") == 0) {
		print_func_help();
	} else if (strcmp(cmd, "w") == 0) {
		if (g_channels[0].width == 1280 && g_channels[0].height == 720) {
			g_channels[0].width = 720;
			g_channels[0].height = 576;
		} else {
			g_channels[0].width = 1280;
			g_channels[0].height = 720;
		}
		change_resolution();
	} else if (strcmp(cmd, "e") == 0) {
		change_fps();
	} else if (strcmp(cmd, "r") == 0) {
		rotate();
	} else if (strcmp(cmd, "j") == 0) {
		capture_jpeg();
	} else if (strcmp(cmd, "f") == 0) {
		toggle_freeze();
	} else if (strcmp(cmd, "y") == 0) {
		capture_yuv();
	} else {
		printf("Invalid function cmd !\n");
		print_func_help();
	}
}

#ifdef RT_USING_FINSH
#include <finsh.h>
FINSH_FUNCTION_EXPORT(venc, venc(target_ip, port));
FINSH_FUNCTION_EXPORT(venc_exit, venc_exit());
FINSH_FUNCTION_EXPORT(venc_func, venc_func(cmd); use venc_func("h") for more info);
#endif
