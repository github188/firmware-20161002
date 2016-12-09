#ifndef __FH_COMMON_H__
#define __FH_COMMON_H__
#include "types/type_def.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"
{
#endif
#endif /* __cplusplus */

//#define API_PRT
#ifdef API_PRT
#define API_DEBUG(...) do{\
	printf( "%s %d: ", __func__,  __LINE__);\
	printf(__VA_ARGS__);\
	}while(0)
#else
#define API_DEBUG(...) 
#endif

#define VPU_CHN_COUNT	      				4
#define PAE_CHN_COUNT						8
#define PAE_MAX_NALU_CNT  					20
#define MAX_OSD_LINE 						3
#define MAX_INVERT_CNT						8

#define RETURN_NULL                         (0)
#define RETURN_OK                           (0)
#define NO_DEVICE                           (-1)
#define OPEN_DEVICE_ERR                     (-1)
#define CLOSE_DEVICE_ERR                    (-2)
#define FIND_DEVICE_ERR                     (-3)
#define PARAM_NULL                          (-4)
#define PARAM_ERR                           (-5)
#define ALIGN_ERR                           (-6)
#define MODULE_ENABLE_ERR                   (-7)
#define CHAN_ERR                            (-8)
#define MEM_NULL                            (-9)
#define GET_STREAM_EMPTY                    (-10)

#define MAX_MASK_AREA                        (8)
#define MIN_PITCH_ALIGN                      (8)
#define DEFALT_ALIGN                         (8)
#define MAX_VPU_CHANNO                       (4)
#define MAX_ENC_CHANNO                       (8)

#define MOU_ENABLE                           (1)
#define MOU_UNABLE                           (0)

#define MALLOCED_MEM_BASE_DSP				 (1)
#define MALLOCED_MEM_VPUPKG					 (0)

typedef struct fhVERSION_INFO_
{
	FH_UINT32  			build_date; /**<������*/
	FH_UINT32 			sw_version; /**<����汾��*/
	FH_UINT32  			hw_version; /**<Ӳ�������汾��*/
}FH_VERSION_INFO;


typedef struct fhPOINT_
{
    FH_UINT32 			u32X; /**< ˮƽ���� */
    FH_UINT32 			u32Y; /**< ��ֱ���� */
}FH_POINT;

typedef struct fhSIZE_S
{
    FH_UINT32 			u32Width; /**< �������  */
    FH_UINT32 			u32Height; /**<�߶�����  */
} FH_SIZE;

typedef struct fhRECT_S
{
    FH_UINT32 			u32X; /**< ��ʼ��ˮƽ���� */
    FH_UINT32 			u32Y; /**< ��ʼ�㴹ֱ����*/
    FH_UINT32 			u32Width; /**< ������*/
    FH_UINT32 			u32Height; /**< ����߶� */
}FH_AREA;


typedef struct
{
	FH_ADDR   				addr;
	FH_UINT32				size;
}FH_ADDR_INFO;

typedef struct
{
	unsigned int base;	/**< �����ַ */
	void * vbase;		/**< �����ַ */
	unsigned int size;	/**< �ڴ��С */
}FH_MEM_INFO;

typedef struct
{
	FH_UINT16 				frame_count;		/**< ֡�� */
	FH_UINT16 				frame_time;		/**< ͳ��ʱ�� (s)*/
} FH_FRAMERATE;

typedef struct
{
FH_UINT32				y;
FH_UINT32				cb;
FH_UINT32				cr;
FH_UINT32				alpha;
} FH_YC_COLOR;

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif  /* _FH_COMMON_H_ */
