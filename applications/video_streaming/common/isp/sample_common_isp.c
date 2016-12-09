
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <rtthread.h>
#include <dfs_posix.h>
#include "sample_common_isp.h"
#include "multi_sensor.h"
#include "dsp/fh_vpu_mpi.h"
#include "isp_common.h"
#include "isp_api.h"
#include "isp_enum.h"

#define ERR_CNT	5

FH_SINT32 g_isp_format;
SensorCtrl_t *g_sensor;

FH_UINT32 sample_isp_change_fps(void)
{
	FH_UINT32 fps;

	API_ISP_Pause();

	switch(g_isp_format)
	{
	case  FORMAT_720P25:
		g_isp_format = FORMAT_720P30;
		fps = 30;
		break;
	case FORMAT_720P30:
		g_isp_format = FORMAT_720P25;
		fps = 25;
		break;
	case FORMAT_960P25:
		g_isp_format = FORMAT_960P30;
		fps = 30;
		break;
	case FORMAT_960P30:
		g_isp_format = FORMAT_960P25;
		fps = 25;
		break;
	}

	API_ISP_SetSensorFmt(g_isp_format);
	API_ISP_Resume();

	return fps;
}

void isp_vpu_reconfig(void)
{
	FH_VPU_SIZE vpu_size;
	ISP_VI_ATTR_S isp_vi;

	FH_VPSS_Reset();
	API_ISP_Pause();
	API_ISP_Resume();

	FH_VPSS_GetViAttr(&vpu_size);
	API_ISP_GetViAttr(&isp_vi);
	if (vpu_size.vi_size.u32Width != isp_vi.u16PicWidth
		|| vpu_size.vi_size.u32Height != isp_vi.u16PicHeight)
	{
		vpu_size.vi_size.u32Width = isp_vi.u16PicWidth;
		vpu_size.vi_size.u32Height = isp_vi.u16PicHeight;
		FH_VPSS_SetViAttr(&vpu_size);
	}

	API_ISP_KickStart();
}

int sample_isp_init(void)
{

	FH_SINT32 ret;
	FH_UINT32 param_addr,param_size;

	g_isp_format = FORMAT_720P25;
	ret = API_ISP_MemInit(1280, 720);
	if (ret) {
		printf ("Error: API_ISP_MemInit failed with %d\n", ret);
		return ret;
	}

	ret = API_ISP_GetBinAddr(&param_addr, &param_size);
	if (ret) {
		printf("Error: API_ISP_GetBinAddr failed with %d\n", ret);
		return ret;
	}

	// sensor ops
	if( Sensor_Probe() == -1 )
	{
		printf("Error: sensor probe failed.\n");
		return -1;
	};

	g_sensor = isp_sensor_Create(Sensor_Getmodel());
	if (g_sensor->sensor == NULL) {
		printf("Error: sensor create error\n");
		return -1;
	}

	API_ISP_SensorRegCb(0, g_sensor->sensor);
	API_ISP_SensorInit();
	API_ISP_SetSensorFmt(g_isp_format);

	ret = API_ISP_Init();
	if (ret) {
		printf("Error: API_ISP_Init failed with %d\n", ret);
		return ret;
	}

	API_ISP_LoadIspParam(g_sensor->param);

	return 0;
}

void sample_isp_proc(void *arg)
{
	FH_SINT32 ret;
	FH_SINT32 err_cnt = 0;
	FH_SINT32 *cancel = (FH_SINT32 *)arg;

	while (! *cancel) {
		ret = API_ISP_Run();
		if (ret) {
			err_cnt++;
			ret = API_ISP_DetectPicSize();
			if (ret && err_cnt > ERR_CNT) {
				isp_vpu_reconfig();
				err_cnt = 0;
			}
		}
		rt_thread_sleep(1);
	}
}


