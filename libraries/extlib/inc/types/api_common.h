#ifndef _API_COMMON_H_
#define _API_COMMON_H_

#include <stdio.h>

#include <rtthread.h>
#include <rtdevice.h>
//#include <posix_types.h>
#include <pthread.h>
#include "bufCtrl.h"

//device
#define SYS_NAME	"media"
#define VPU_NAME	"vpu"
#define PAE_NAME	"enc"
#define VOU_NAME	"vou"
#define JPEG_NAME	"jpeg"
#define AUDIO_NAME	"fh_audio"

typedef rt_device_t DEV_FD;
#define dev_ioctl(dev,cmd,data)	errno = rt_device_control(dev,cmd,data)
#define dev_open(name,p1,p2)	rt_device_find(name);rt_device_open(rt_device_find(name),RT_DEVICE_OFLAG_RDWR)
#define dev_close(dev)		rt_device_close(dev)


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif

