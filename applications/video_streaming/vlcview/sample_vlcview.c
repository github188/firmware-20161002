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
#include "rtp.h"

static FH_SINT32 g_exit = 1;
static rt_thread_t g_thread_isp = 0;
static rt_thread_t g_thread_stream = 0;

/*save the vlcview mode (UDP,rtsp_udp,rtsp_tcp,rtp)*/
unsigned int mode=2;

void sample_vlcview_exit()
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

void sample_vlcview_get_stream_proc(void *arg)
{
	FH_SINT32 ret, i;
	FH_ENC_STREAM_ELEMENT stream;
	struct vlcview_enc_stream_element stream_element;
	FH_SINT32 *cancel = (FH_SINT32 *)arg;

	while (! *cancel) {
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

					if (mode!=0)rtp_push_data(stream.nalu[i].start, stream.nalu[i].length, stream.time_stamp);
				}
				if (mode==0) vlcview_pes_stream_pack(0, stream_element);
				FH_VENC_ReleaseStream(0);
			}
		} while (ret == RETURN_OK);
		rt_thread_sleep(1);
	}
}

int vlcview(char* dsp_ip, rt_uint32_t port_no)
{
	FH_VPU_SIZE vi_pic;
	FH_VPU_CHN_CONFIG chn_attr;
	FH_CHR_CONFIG cfg_param;
	FH_SINT32 ret;
	int port = port_no;

	if (!g_exit) {
		printf("vicview is running!\n");
		return 0;
	}

	g_exit = 0;

#if defined(CONFIG_CHIP_FH8620) || defined(CONFIG_CHIP_FH8620G)
	bufferInit((unsigned char *)FH_SDK_MEM_START, FH_SDK_MEM_SIZE);
#elif defined CONFIG_CHIP_FH8810
	bufferInit((unsigned char *)0xa1000000,(32 * 1024 * 1024));
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
	cfg_param.rc_config.bitrate = 1 * 1024 * 1024;
	cfg_param.rc_config.FrameRate.frame_count = 30;
	cfg_param.rc_config.FrameRate.frame_time  = 1;
	cfg_param.chn_attr.profile = FH_PRO_MAIN;
	cfg_param.chn_attr.i_frame_intterval = 50;
	cfg_param.init_qp = 26;
	cfg_param.rc_config.ImaxQP = 50;
	cfg_param.rc_config.IminQP = 28;
	cfg_param.rc_config.PmaxQP = 50;
	cfg_param.rc_config.PminQP = 28;
	cfg_param.rc_config.RClevel = FH_RC_MIDDLE;
	cfg_param.rc_config.RCmode = FH_RC_VBR;
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
	 step  10: int vlcview lib
	******************************************/
	ret = vlcview_pes_init();
	if (0 != ret) {
		printf("Error: vlcview_pes_init failed with %d\n", ret);
		goto err_exit;
	}

	/******************************************
	 step  11: get stream, pack as PES stream and then send to vlc
	******************************************/
	switch (mode)
	{
	  case 0:
		  vlcview_pes_send_to_vlc(0, dsp_ip, port);
		  break;
	  case 1:
		  rtsp_start_server(RTP_TRANSPORT_UDP);
		  break;
	  case 2:
		  rtsp_start_server(RTP_TRANSPORT_TCP);
		  break;
	  case 3:
		  sap_rtp_start_server();
		  break;
	  default:
		  rtsp_start_server(RTP_TRANSPORT_TCP);
	}

	g_thread_stream = rt_thread_create("vlc_get_stream",
		                               sample_vlcview_get_stream_proc,
		                               &g_exit,
		                               3*1024,
		                               RT_APP_THREAD_PRIORITY,
		                               10);
	if (g_thread_stream != RT_NULL) {
		rt_thread_startup(g_thread_stream);
	}

	// rt_thread_sleep(10);

#if ENABLE_COOLVIEW
#include "di/di_config.h"

	rt_thread_t thread_dbg;
	DI_Config dic;
	dic.type = DIT_ETH;
	dic.conf.eth.debug_port = 8888;
	strcpy(dic.conf.eth.debug_ip, "0.0.0.0");

	DebugInterface_Init(&dic);

	thread_dbg = rt_thread_create("coolview",
		                          (void*)DebugInterface_MainLoop,
		                          &g_exit,
		                          4*1024,
		                          RT_APP_THREAD_PRIORITY + 10,
		                          10);
	if (thread_dbg != RT_NULL) {
		rt_thread_startup(thread_dbg);
	}
#endif

	return 0;

err_exit:
	g_exit = 1;
	sample_vlcview_exit();
	vlcview_pes_uninit();
	deinit_stream_pack();

	switch (mode)
	{
	  case 0:
		  break;
	  case 1:
		  rtsp_stop_server();
		  break;
	  case 2:
		  rtsp_stop_server();
		  break;
	  case 3:
		  sap_rtp_stop_server();
		  break;
	  default:
		  break;
	}

	return -1;
}

int vlcview_exit()
{
	if (!g_exit) {
		g_exit = 1;
		sample_vlcview_exit();
		vlcview_pes_uninit();
		deinit_stream_pack();

		switch (mode)
		{
		case 0:
			break;
	  	case 1:
		  	rtsp_stop_server();
			break;
		case 2:
			rtsp_stop_server();
			break;
		case 3:
			sap_rtp_stop_server();
			break;
		default:
			break;
		}
	} else {
		printf("vicview is not running!\n");
	}

	return 0;
}

void vlcview_udp(char* dsp_ip, rt_uint32_t port_no)
{
	if (!g_exit)
	{
		vlcview_exit();
	}
	mode=0;
	vlcview(dsp_ip, port_no);
	return;
}

void rtsp_udp(void)
{
	if (!g_exit)
	{
		vlcview_exit();
	}
	mode=1;
	vlcview(RT_NULL,RT_NULL);
	return;
}

void rtsp_tcp(void)
{
	if (!g_exit)
	{
		vlcview_exit();
	}
	mode=2;
	vlcview(RT_NULL,RT_NULL);
	return;
}

void vlcview_rtp(void)
{
	if (!g_exit)
	{
		vlcview_exit();
	}
	mode=3;
	vlcview(RT_NULL,RT_NULL);
	return;
}

void test_udp(char argc, char** argv)
{
	vlcview_udp(argv[1], 2345);
}

#ifdef RT_USING_FINSH
#include <finsh.h>
int rtsp(int argc, char** argv)
{
    if (argc == 1)
    {
        rtsp_tcp();
    }
    else
    {
        if (strcmp(argv[1], "tcp") == 0)
        {
            rtsp_tcp();
        }
        else if (strcmp(argv[1], "udp") == 0)
        {
            rtsp_udp();
        }
        else if (strcmp(argv[1], "rtp") == 0)
        {
            vlcview_rtp();
        }
        else if (strcmp(argv[1], "stop") == 0)
        {
            vlcview_exit();
        }
        else
        {
            rt_kprintf("rtsp tcp - RTSP over TCP\n");
            rt_kprintf("rtsp udp - RTSP over UDP\n");
            rt_kprintf("rtsp rtp - RTP view\n");
        }
    }

    return 0;
}
MSH_CMD_EXPORT(rtsp, rtsp command);

FINSH_FUNCTION_EXPORT(vlcview_udp, vlcview_udp(dst_ip, port));
FINSH_FUNCTION_EXPORT(rtsp_tcp, start rtsp serveice with tcp protocol);
FINSH_FUNCTION_EXPORT(vlcview_exit, vlcview_exit());
MSH_CMD_EXPORT(test_udp, start rtsp serveice with udp protocol);
#endif
