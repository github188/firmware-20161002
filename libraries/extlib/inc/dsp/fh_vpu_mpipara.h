#ifndef __FH_VPU_MPIPARA_H__
#define __FH_VPU_MPIPARA_H__

#include "types/type_def.h"
#include "fh_common.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"
{
#endif
#endif /* __cplusplus */

typedef struct
{
	FH_SIZE   				vi_size;
}FH_VPU_SIZE;

typedef enum
{ 	
	VPU_MODE_ISP 	= 0,			/**< ��ISP����ģʽ*/
	VPU_MODE_MEM	= 1,			/**< ��MEM����ģʽ*/
}FH_VPU_VI_MODE;


typedef struct
{
	FH_SIZE   				vpu_chn_size;
}FH_VPU_CHN_CONFIG;


typedef struct
{
	FH_SIZE   				pic_size;	/**< ͼƬ�ߴ� */
	FH_ADDR  				yluma;	/**< ͼƬ���ݵ�luma��ַ */
	FH_ADDR 				chroma;	/**< ͼƬ���ݵ�chroma��ַ */
	FH_UINT32 				ystride;/**< ͼƬ���ݵ�luma stride */
	FH_UINT32 				cstride;/**< ͼƬ���ݵ� chroma stride */
}FH_VPU_USER_PIC;

///YUV��ʽ
typedef enum
{
	PIXEL_FORMAT_MONOCHROME 	= 0,	/**<��ɫ*/
	PIXEL_FORMAT_420 			= 1,	/**<420*/
	PIXEL_FORMAT_422 			= 2,	/**<422*/
	PIXEL_FORMAT_444 			= 3 	/**<444*/
}FH_VPU_PIXELFORMAT;

typedef struct
{
	FH_VPU_PIXELFORMAT		pixelFormat;/**< ���ظ�ʽ */
	FH_UINT32 				stride;		/**< ͼ���� */
	FH_SIZE   				size;		/**< ͼ�����ݳ��� */
	FH_MEM_INFO				yluma;		/**< luma��ַ*/
	FH_MEM_INFO 			chroma;		/**< chroma ��ַ*/
	FH_UINT64				time_stamp; /**< ʱ���*/
}FH_VPU_STREAM;


typedef struct
{
	 FH_UINT32  			masaic_enable;	/**<ʹ�ܣ�1��ʾ 0 ����ʾ */
	 FH_UINT32  			masaic_size;	/**<�ߴ�0: 16x16��1: 32x32 */
}FH_MASK_MASAIC;


typedef struct
{
	FH_BOOL 				mask_enable[MAX_MASK_AREA];	/**< ����ʹ�� */
	FH_AREA 				area_value[MAX_MASK_AREA];	/**< �������� */
	FH_UINT32  				color;	/**< ��ʾ��ɫ */
	FH_MASK_MASAIC 			masaic;	/**< ���������� */
}FH_VPU_MASK;


typedef struct
{
	FH_UINT32 				alpha_value;/**< ͼ�β��ͼ���ʽ��ARGB1555��ʽ����A=1��ʱ��˴���Alpha�����ã�ȡֵ[0..127] ȡֵԽС��Խ͸�� */
	FH_UINT32 				dtvmode; /*0ΪSDTVģʽ��1ΪHDTVģʽ*/
	FH_UINT32 				rgbmode;/*0Ϊstdio RGB��1Ϊcomputer RGB*/
	FH_SIZE 				logo_size;/**< logo��С */
}FH_LOGO_CFG;


typedef struct
{
	FH_UINT32       		logo_enable;	/**< ͼƬ����ʹ�� */
	FH_UINT8     			*logo_addr;		/**< logo���ݵ������ַ */
	FH_LOGO_CFG       		logo_cfg;		/**< logo ���� */
	FH_POINT  				logo_startpos;	/**< logo ���Ͻ���ʼ�� */
	FH_POINT				logo_cutpos;  /**<logo�и���ʾ��ʼ�� */
	FH_UINT32				stride_value;	 /**<�г��� */
}FH_VPU_LOGO;


typedef enum fh_OSDROTATE_E
{
    OSD_ROTATE_NONE = 0,
    OSD_ROTATE_90   = 1,
    OSD_ROTATE_180  = 2,
    OSD_ROTATE_270  = 3,
    OSD_ROTATE_BUTT
} FH_OSD_ROTATE;
typedef struct
{
	FH_UINT32 				Vamp;/*ˮƽ�Ŵ�һ��*/
	FH_UINT32 				Hamp;/*��ֱ�Ŵ�һ��*/
	FH_UINT32 				txtrotate;/*�����ַ���ת*/
	FH_UINT32 				txtw;/*�ַ����*/
	FH_UINT32 				txth;/*�ַ��߶�,ֻ��txt2ģʽʱ����*/
	FH_UINT32 				fontsize;/*����ռ���ڴ��С*/
	FH_UINT32 				fontw;
	FH_UINT32 				fonth;
}FH_OSD_CFG;


typedef struct
{
	FH_UINT32 				norcolor;/*�������ַ���ɫ*/
	FH_UINT32 				invcolor;/*��ɫ���ַ���ɫ*/
}FH_OSD_COLOR;

typedef struct
{
	FH_UINT32 				invert_enable;
	FH_UINT32 				text_idex;
	FH_UINT32 				invert_word[MAX_INVERT_CNT];
}FH_INVERT_CTRL;

typedef struct
{
	FH_UINT32 				idex;/*�ַ���ǣ�0Ϊtxt0,1Ϊtext1,2Ϊtext2*/
	FH_UINT32 				osd_enable;

	FH_UINT8 				*osdtxtaddr;/*�ַ�����ʼ��ַ*/
	FH_UINT8 				*osdfontaddr;/*�ַ���ĵ�ַ*/
	FH_INVERT_CTRL 			invert_ctl; /*�ַ���ɫ��ɫ����*/
	FH_OSD_CFG   			osd_cfg;/**< ���� */
	FH_OSD_COLOR 			osd_color;/**< �ַ���ɫ */
	FH_OSD_COLOR 			osd_blgcolor;/**< ������ɫ */
	FH_POINT				osd_startpos;/**< ͼ������ʾ��λ�� */
}FH_VPU_OSD;

typedef struct
{
	FH_UINT32 				frame_id;
	FH_ADDR_INFO 			ymean;
	FH_ADDR_INFO 			cmean;
}FH_VPU_YCMEAN;

typedef struct
{
	FH_AREA  				vpu_crop_area;
}FH_VPU_CROP;


typedef struct 
{
	FH_UINT16 				CNREdgeT;/*�߽���ֵǿ��U10*/
	FH_UINT16 				CNREdgeT1;/*�߽���ֵǿ��U10*/
	FH_UINT16 				CNRYDwSkipMode;/*CNR��ʹ�õ�YNR��Ϣ�²���ģʽU2*/
	FH_UINT16 				CNRSigma;/*��������U8*/
	FH_UINT16 				CNRAlpha;/*��������U4*/
	FH_UINT16 				YNRTh;/*YNR�е���ֵU10*/
}FH_VPU_YCNR_PARAMS;

typedef struct 
{
	FH_UINT8 				APCPGain;/*����APC��������U4.4*/
	FH_UINT8 				APCNGain;/*����APC��������U4.4*/
	FH_UINT8 				MergeSel;/*ϸ����ǿ�ͱ߽��񻯺ϲ�ģʽU1*/
	FH_UINT8 				DEPGain;/*ϸ����ǿ��������U7*/
	FH_UINT8 				DENGain;/*ϸ����ǿ��������U4.4*/
	FH_UINT16 				DESTHL;/* ϸ����ǿSlice LOW����ֵU10*/
	FH_UINT16 				DESTHH;/* ϸ����ǿSlice high����ֵU10*/
	FH_UINT8 				ESPGain;/*�߽�����������*/
	FH_UINT8 				ESNGain;/*�߽��񻯸�������*/
	FH_UINT16 				ESSTHL;/*�߽��� slice low����ֵ*/
	FH_UINT16 				ESSTHH;/*�߽���slice HIGH����ֵ*/
}FH_VPU_APC_PARAMS;


typedef struct
{
	FH_UINT8 				*base;
	FH_UINT32  				size;
	FH_UINT32  				user_w;
	FH_UINT32  				user_h;
}FH_VPU_CHN_INFO;

typedef struct
{
	FH_UINT32 				base;
	void 					*vbase;
	FH_UINT32 				size;
}FH_PKG_INFO;

typedef enum
{
	FH_VPU_MOD_MASK 		= (1 << 1),	/**<��˽���� */
	FH_VPU_MOD_LOGO 		= (1 << 3),	/**<ͼ��� */
	FH_VPU_MOD_STAT_MEAN 	= (1 << 4),	/**<ͳ����� */
	FH_VPU_MOD_OSD0			= (1 << 5),	/**<���ֲ�0 */
	FH_VPU_MOD_OSD1			= (1 << 6),	/**<���ֲ�1 */
	FH_VPU_MOD_OSD2			= (1 << 7),	/**<���ֲ�2 */
	FH_VPU_MOD_DITHER		= (1 << 15),/**<������*/
	FH_VPU_MOD_CNR			= (1 << 16),/**<ɫ�Ƚ��� */
	FH_VPU_MOD_YNR			= (1 << 17),/**<���Ƚ��� */
	FH_VPU_MOD_APC			= (1 << 18),/**<�� */
	FH_VPU_MOD_PURPLE		= (1 << 19),/**<ȥ�ϱ�*/
}FH_VPU_MODULE;

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif
