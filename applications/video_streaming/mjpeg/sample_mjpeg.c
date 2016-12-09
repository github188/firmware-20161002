#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <rtthread.h>

#include "types/type_def.h"
#include "dsp/fh_system_mpi.h"
#include "dsp/fh_vpu_mpi.h"
#include "dsp/fh_jpeg_mpi.h"
#include "sample_common_isp.h"

static FH_SINT32 g_exit = 1;
static rt_thread_t g_thread_isp = 0;
static rt_thread_t g_thread_stream = 0;

void sample_mjpeg_exit()
{
    rt_thread_t exit_process;

    exit_process = rt_thread_find("vlc_get_stream");
    if(exit_process)
        rt_thread_delete(exit_process);

    API_ISP_Exit();
    exit_process = rt_thread_find("vlc_isp");
    if(exit_process)
        rt_thread_delete(exit_process);

    mjpeg_stop_server();

    FH_SYS_Exit();
}

void sample_mjpeg_get_stream_proc(void *arg)
{
    FH_SINT32 ret, i;
    FH_JPEG_STREAM_INFO jpeg_info;
    FH_JPEG_CONFIG jpeg_config;

    jpeg_config.QP = 50;
    jpeg_config.rate = 0x70;
    FH_JPEG_Setconfig(&jpeg_config);

    while (!g_exit) {
        ret = FH_SYS_BindVpu2Jpeg(0);
        if (ret == 0) {
            do {
                ret = FH_JPEG_Getstream_Block(&jpeg_info);
            } while (ret != 0);

            if (jpeg_info.stream.size != 0) {
                mjpeg_send_stream(jpeg_info.stream.addr, jpeg_info.stream.size);
            } else {
                printf("jpeg stream size is zero\n");
            }
        } else {
            printf("jpeg bind failed %d\n", ret);
        }
    }
}

int mjpeg(int port)
{
    FH_VPU_SIZE vi_pic;
    FH_VPU_CHN_CONFIG chn_attr;
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
     step  7: init jpeg system
    ******************************************/
    ret = FH_JPEG_InitMem(1280, 720);
    if (ret != RETURN_OK) {
        printf("Error: FH_JPEG_InitMem failed with %d\n", ret);
        goto err_exit;
    }

    /******************************************
     step  8: start mjpeg server thread and mjpeg stream thread,
              the server waits the clients to connect
    ******************************************/
    mjpeg_start_server(port);
    g_thread_stream = rt_thread_create("vlc_get_stream",
                                       sample_mjpeg_get_stream_proc,
                                       &g_exit,
                                       3*1024,
                                       RT_APP_THREAD_PRIORITY,
                                       10);
    rt_thread_startup(g_thread_stream);

    return 0;

    /******************************************
     step  10: exit process
    ******************************************/
err_exit:
    sample_mjpeg_exit();

    return -1;
}

void mjpeg_exit(void)
{
    if (!g_exit) {
        g_exit = 1;
        sample_mjpeg_exit();
    } else {
        printf("vicview is not running!\n");
    }
}

void mjpeg_usage(void)
{
	printf("mjpeg start [port]\n");
	printf("mjpeg stop\n");
}

int mjpeg_main(int argc, char ** argv)
{
	int port = 80;

	if (argc == 1)
	{
		mjpeg_usage();
		return 0;
	}

	if (strcmp(argv[1], "start") == 0)
	{
		if (argc == 3) port = atoi(argv[2]);

		mjpeg(port);
	}
	else if (strcmp(argv[1], "stop") == 0)
	{
		mjpeg_exit();
	}
	else
	{
		mjpeg_usage();
	}

	return 0;
}

#ifdef RT_USING_FINSH
#include <finsh.h>
FINSH_FUNCTION_EXPORT(mjpeg, mjpeg(server_port));
FINSH_FUNCTION_EXPORT(mjpeg_exit, mjpeg_exit());
FINSH_FUNCTION_EXPORT_ALIAS(mjpeg_main, __cmd_mjpeg, mjpeg program);
#endif
