#include <rtthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dfs_def.h>
#include <finsh.h>
#include "types/type_def.h"
#include "dsp/fh_system_mpi.h"
#include "dsp/fh_vpu_mpi.h"
#include "dsp/fh_venc_mpi.h"
#include "di/di_config.h"
#include "sample_common_isp.h"
#include "libvlcview.h"
#include "osd.h"
#include "fontlib.h"
#include "logo_64x64.h"

#define LOGO_ALPHA_MAX          255
#define LOGO_MAX_WIDTH          64
#define LOGO_MAX_HEIGHT         64

enum masaic_size {
	MASAIC_SIZE_16 = 0,
	MASAIC_SIZE_32
};

static FH_SINT32 g_exit = 1;
static rt_thread_t g_thread_isp = 0;
static rt_thread_t g_thread_stream = 0;


void sample_overlay_exit()
{
	rt_thread_t exit_process;

	exit_process = rt_thread_find("vlc_get_stream");
	if(exit_process)
		rt_thread_delete(exit_process);

	API_ISP_Exit();
	exit_process = rt_thread_find("vlc_isp");
	if(exit_process)
		rt_thread_delete(exit_process);

	osd_unload_fontlib(FHEN_FONT_TYPE_ASC);
	osd_unload_fontlib(FHEN_FONT_TYPE_CHINESE);
	osd_uninit();
	FH_SYS_Exit();
}

void sample_overlay_get_stream_proc(void *arg)
{
	FH_SINT32 ret, i;
	FH_ENC_STREAM_ELEMENT stream;
	struct vlcview_enc_stream_element stream_element;

	while (!g_exit) {
		do {
			ret = FH_VENC_GetStream(0, &stream);
			if (ret == RETURN_OK) {
				stream_element.frame_type = stream.frame_type;
				stream_element.frame_len = stream.length;
				stream_element.time_stamp = stream.time_stamp;
				stream_element.nalu_count = stream.nalu_cnt;
				for (i = 0; i < stream_element.nalu_count; i++) {
					stream_element.nalu[i].start = stream.nalu[i].start;
					stream_element.nalu[i].len = stream.nalu[i].length;
				}
				vlcview_pes_stream_pack(0, stream_element);
				FH_VENC_ReleaseStream(0);
			}
		} while (ret == RETURN_OK);
		rt_thread_sleep(1);
	}
}

FH_UINT32 argb2ayuv(FH_UINT32 A, FH_UINT32 R, FH_UINT32 G, FH_UINT32 B)
{
	FH_UINT32 Y = (FH_UINT32)(0.299 * R + 0.587 * G + 0.114 * B);
	FH_UINT32 Cr = (FH_UINT32)((0.500*R - 0.4187*G - 0.0813*B) + 128);
	FH_UINT32 Cb = (FH_UINT32)((-0.1687*R-0.3313*G+0.500*B)  + 128);

	return (((A & 0xff) << 24) | ((Y & 0xff) << 16) | ((Cb & 0xff) << 8) | ((Cr & 0xff)));
}

void sample_overlay_set_mask(FH_BOOL masaic_enable)
{
	FH_VPU_MASK mask_config;
	FH_SINT32 index;
	FH_SINT32 ret;

	for (index = 0; index < MAX_MASK_AREA; index++) {
		mask_config.mask_enable[index] = 1;
		mask_config.area_value[index].u32X = (index % 4) * 320;
		mask_config.area_value[index].u32Y = index > 3 ? 620 : 0;
		mask_config.area_value[index].u32Width = 100;
		mask_config.area_value[index].u32Height = 100;
	}

	mask_config.color = argb2ayuv(255, 0, 255, 0); // Green

	if (masaic_enable) {
		mask_config.masaic.masaic_enable = 1;
		mask_config.masaic.masaic_size = MASAIC_SIZE_16;
	} else {
		mask_config.masaic.masaic_enable = 0;
	}

	ret = FH_VPSS_SetMask(&mask_config);
	if (ret != RETURN_OK) {
		printf("Error: FH_VPSS_SetMask failed with %d\n", ret);
	}
}

FH_SINT32 load_font_lib(OSD_FontType_e type, FH_SINT32 size, char* src)
{
	FH_SINT32 ret;
	OSD_FontLib_t font_lib;

	font_lib.pLibData = src;
	font_lib.fontSize = size;

	ret = osd_load_fontlib(type, &font_lib);
	if (ret != 0) {
		printf("Error: osd_load_fontlib failed, ret=%d\n", ret);
		return ret;
	}

	return 0;
}

void sample_overlay_set_osd()
{
	FH_SINT32 i, ret;
	OSD_CONFIG_t osd_config;

	ret = osd_init(1280, 720); // same with channel width and height;
	if (ret != RETURN_OK) {
		printf("Error: osd_init failed with %d\n", ret);
		return;
	}

	if (0 != load_font_lib(FHEN_FONT_TYPE_ASC,sizeof(asc16), asc16))
		return;

	if (0 != load_font_lib(FHEN_FONT_TYPE_CHINESE,sizeof(gb2312),gb2312))
		return;

	osd_config.osdColor.norcolor.fRed = 255;
	osd_config.osdColor.norcolor.fGreen = 255;
	osd_config.osdColor.norcolor.fBlue = 255;
	osd_config.osdColor.invcolor.fRed = 0;
	osd_config.osdColor.invcolor.fGreen = 0;
	osd_config.osdColor.invcolor.fGreen = 0;

	osd_config.timeOsdEnable = 1;
	osd_config.timeOsdPosition.pos_x = 0;
	osd_config.timeOsdPosition.pos_y = 0;
	osd_config.timeOsdFormat = 9;
	osd_config.timeOsdNorm = 0;
	osd_config.weekFlag = 1;

	osd_config.text01Enable = 1;
	osd_config.sttext01Position.pos_x = 0;
	osd_config.sttext01Position.pos_y = 32;
	strcpy(osd_config.text01Info, "abcd 1234");

	osd_config.text02Enable = 1;
	osd_config.sttext02Position.pos_x = 0;
	osd_config.sttext02Position.pos_y = 64;
	strcpy(osd_config.text02Info, "this is demo.");

	ret = osd_set_text(&osd_config);
	if (ret != RETURN_OK) {
		printf("Error: osd_set_text failed with %d\n", ret);
	}
}

void sample_overlay_set_logo()
{
	FH_SINT32 ret;
	OSD_Logo_t logo_config;
	FILE *logo_file;
	struct stat stat_info;

	logo_config.enable = 1;
	logo_config.alpha = 255;
	logo_config.area.fTopLeftX = 640;
	logo_config.area.fTopLeftY = 360;
	logo_config.area.fWidth = 64;
	logo_config.area.fHeigh = 64;

	logo_config.pData = logo_data;
	ret = osd_set_logo(&logo_config);
	if (ret != RETURN_OK) {
		printf("Error: osd_set_logo failed with %d\n", ret);
	}
}

int overlay(char* target_ip, rt_uint32_t port_no){
	FH_VPU_SIZE vi_pic;
	FH_VPU_CHN_CONFIG chn_attr;
	FH_CHR_CONFIG cfg_param;
	FH_SINT32 ret;

	if (!g_exit) {
		printf("overlay is running!\n");
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
	 step  5: configure vpss channel 0
	******************************************/
	chn_attr.vpu_chn_size.u32Width = 1280;
	chn_attr.vpu_chn_size.u32Height = 720;
	ret = FH_VPSS_SetChnAttr(0, &chn_attr);
	if (RETURN_OK != ret) {
		printf("Error: FH_VPSS_SetChnAttr failed with %d\n", ret);
		goto err_exit;
	}

	/******************************************
	 step  6: open vpss channel 0
	******************************************/
	ret = FH_VPSS_OpenChn(0);
	if (RETURN_OK != ret) {
		printf("Error: FH_VPSS_OpenChn failed with %d\n", ret);
		goto err_exit;
	}


	/******************************************
	 step  7: create venc channel 0
	******************************************/
	cfg_param.chn_attr.size.u32Width = 1280;
	cfg_param.chn_attr.size.u32Height = 720;
	cfg_param.rc_config.bitrate = 2048 * 1024;
	cfg_param.rc_config.FrameRate.frame_count = 30;
	cfg_param.rc_config.FrameRate.frame_time  = 1;
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
	ret = FH_VENC_CreateChn(0, &cfg_param);
	if (RETURN_OK != ret) {
		printf("Error: FH_VENC_CreateChn failed with %d\n", ret);
		goto err_exit;
	}

	/******************************************
	 step  8: start venc channel 0
	******************************************/
	ret = FH_VENC_StartRecvPic(0);
	if (RETURN_OK != ret) {
		printf("Error: FH_VENC_StartRecvPic failed with %d\n", ret);
		goto err_exit;
	}

	/******************************************
	 step  9: bind vpss channel 0 with venc channel 0
	******************************************/
	ret = FH_SYS_BindVpu2Enc(0, 0);
	if (RETURN_OK != ret) {
		printf("Error: FH_SYS_BindVpu2Enc failed with %d\n", ret);
		goto err_exit;
	}

	/******************************************
	 step  10: set mask
	******************************************/
	sample_overlay_set_mask(0);

	/******************************************
	 step  11: set osd,logo by using FHAdv API
	******************************************/
	sample_overlay_set_osd();
	sample_overlay_set_logo();

	ret = vlcview_pes_init();
	if (0 != ret) {
		printf("Error: vlcview_pes_init failed with %d\n", ret);
		goto err_exit;
	}
	vlcview_pes_send_to_vlc(0, target_ip, port_no);

	/******************************************
	 step  12: get stream, then save it to file
	******************************************/
	g_thread_stream = rt_thread_create("vlc_get_stream",
		                               (void *)sample_overlay_get_stream_proc,
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
	sample_overlay_exit();
	vlcview_pes_uninit();
	deinit_stream_pack();
	return -1;
}

int overlay_exit()
{
	if (!g_exit) {
		g_exit = 1;
		sample_overlay_exit();
		vlcview_pes_uninit();
		deinit_stream_pack();
	} else {
		printf("overlay is not running!\n");
	}

	return 0;
}

#ifdef RT_USING_FINSH
#include <finsh.h>
FINSH_FUNCTION_EXPORT(overlay, overlay(target_ip, port));
FINSH_FUNCTION_EXPORT(overlay_exit, overlay_exit());
#endif