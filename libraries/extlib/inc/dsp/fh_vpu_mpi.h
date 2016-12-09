#ifndef __FH_VPU_MPI_H__
#define __FH_VPU_MPI_H__

#include "types/type_def.h"
#include "fh_vpu_mpipara.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"
{
#endif
#endif /* __cplusplus */

/**
* FH_VPSS_SetViAttr
*@brief ������Ƶ��������
*@param [in] pstViconfig����Ƶ��������ֵ��ָ��
*@param [out] ��
*@return �Ƿ�ɹ�
* - RETURN_OK(0):  �ɹ�
* - ������ʧ��,������������
*/
FH_SINT32 FH_VPSS_SetViAttr(FH_VPU_SIZE *pstViconfig);

/**
* FH_VPSS_GetViAttr
*@brief ��ȡ��Ƶ��������
*@param [in] ��
*@param [out] pstViconfig����Ƶ��������ֵ��ָ��
*@return �Ƿ�ɹ�
* - RETURN_OK(0):  �ɹ�
* - ������ʧ��,������������
*/
FH_SINT32 FH_VPSS_GetViAttr(FH_VPU_SIZE *pstViconfig);

/**
* FH_VPSS_SysInitMem
*@brief ��ʼ��VPSSģ���ϵͳ�ڴ棬���ڲ����ã��û�����Ҫ���Ե���
*@param [in] ��
*@param [out] ��
*@return �Ƿ�ɹ�
* - RETURN_OK(0):  �ɹ�
* - ������ʧ��,������������
*/
FH_SINT32 FH_VPSS_SysInitMem(void);

/**
* FH_VPSS_Enable
*@brief ָ��ģʽ����VPU ͨ�����������ģʽʱ�����ȵ���FH_VPSS_Disable,Ȼ���ٵ��ô˺���
*@param [in] mode ѡ��VPU����ģʽ����ISP��ȡ���ݻ��MEM ��ȡ����
*@param [out] ��
*@return �Ƿ�ɹ�
* - RETURN_OK(0):  �ɹ�
* - ������ʧ��,������������
*/
FH_SINT32 FH_VPSS_Enable(FH_VPU_VI_MODE mode);

/**
* FH_VPSS_Disable
*@brief ����VPSS��
*@param [in] ��
*@param [out] ��
*@return �Ƿ�ɹ�
* - RETURN_OK(0):  �ɹ�
* - ������ʧ��,������������
*/
FH_SINT32 FH_VPSS_Disable(void);

/**
* FH_VPSS_FreezeVideo
*@brief ��Ƶ����
*@param [in] ��
*@param [out] ��
*@return �Ƿ�ɹ�
* - RETURN_OK(0):  �ɹ�
* - ������ʧ��,������������
*/
FH_SINT32 FH_VPSS_FreezeVideo(void);

/**
* FH_VPSS_UnfreezeVideo
*@brief ��Ƶ�ⶳ
*@param [in] ��
*@param [out] ��
*@return �Ƿ�ɹ�
* - RETURN_OK(0):  �ɹ�
* - ������ʧ��,������������
*/
FH_SINT32 FH_VPSS_UnfreezeVideo(void);

/**
* FH_VPSS_GetChnFrame
*@brief ��VPUͨ����ȡһ֡ͼ��,����
*@param [in] chan ͨ���ţ�ȡֵ0-3
*@param [out] pstVpuframeinfo ��ȡ��ͼ����Ϣָ��
*@return �Ƿ�ɹ�
* - RETURN_OK(0):  �ɹ�
* - ������ʧ��,������������
*/
FH_SINT32 FH_VPSS_GetChnFrame(FH_UINT32 chan,FH_VPU_STREAM *pstVpuframeinfo);

/**
* FH_VPSS_SendUserPic
*@brief ֧���û�����ͼƬ��Ϣ���б��룬��������Ƶ��ʧʱ�Ĳ����Զ���ͼƬ
*@param [in] pstUserPic �û����͵�ͼƬ��Ϣָ��
*@param [out] ��
*@return �Ƿ�ɹ�
* - RETURN_OK(0):  �ɹ�
* - ������ʧ��,������������
*/
FH_SINT32 FH_VPSS_SendUserPic(const FH_VPU_USER_PIC *pstUserPic);

/**
* FH_VPSS_GetUserPicAddr
*@brief ��ȡ���ڴ���û�ͼƬ���ڴ��ַ����������Ƶ��ʧʱ�Ĳ����Զ���ͼƬ��
*@param [in] ��
*@param [out] pstUserPic �û����͵�ͼƬ��Ϣָ�롣
*@return �Ƿ�ɹ�
* - RETURN_OK(0):  �ɹ�
* - ������ʧ��,������������
*/
FH_SINT32 FH_VPSS_GetUserPicAddr(FH_VPU_USER_PIC *pstUserPic);
 
//FH_SINT32 FH_VPSS_OpenModule(FH_UINT32 moduleenablebit);
//FH_SINT32 FH_VPSS_CloseModule(FH_UINT32 moduleenablebit);
//FH_SINT32 FH_VPSS_QueryModuleBit(FH_UINT32  *enablestatusbit);

/**
* FH_VPSS_SetChnAttr
*@brief ����VPU ͨ������
*@param [in] chan ͨ���ţ�ȡֵ0-3
*@param [in] pstChnconfig VPUͨ��������ָ��
*@param [out] ��
*@return �Ƿ�ɹ�
* - RETURN_OK(0):  �ɹ�
* - ������ʧ��,������������
*/
FH_SINT32 FH_VPSS_SetChnAttr(FH_UINT32  chan,const FH_VPU_CHN_CONFIG *pstChnconfig);

/**
* FH_VPSS_GetChnAttr
*@brief ��ȡVPU ͨ�����õ�����ֵ
*@param [in] chan ͨ���ţ�ȡֵ0-3
*@param [out] pstChnconfig VPUͨ��������ָ��
*@return �Ƿ�ɹ�
* - RETURN_OK(0):  �ɹ�
* - ������ʧ��,������������
*/
FH_SINT32 FH_VPSS_GetChnAttr(FH_UINT32  chan, FH_VPU_CHN_CONFIG *pstChnconfig);

/**
* FH_VPSS_OpenChn
*@brief ��VPUͨ��
*@param [in] chan ͨ���ţ�ȡֵ0-3
*@param [out] ��
*@return �Ƿ�ɹ�
* - RETURN_OK(0):  �ɹ�
* - ������ʧ��,������������
*/
FH_SINT32 FH_VPSS_OpenChn(FH_UINT32  chan);

/**
* FH_VPSS_CloseChn
*@brief �ر�VPU ͨ��
*@param [in] chan ͨ���ţ�ȡֵ0-3
*@param [out] ��
*@return �Ƿ�ɹ�
* - RETURN_OK(0):  �ɹ�
* - ������ʧ��,������������
*/
FH_SINT32 FH_VPSS_CloseChn(FH_UINT32  chan);

/**
* FH_VPSS_VOU_SetAttr
*@brief ����VOU������Դ��ͨ������
*@param [in] pstChnconfig VPUͨ��������ָ��
*@param [out] ��
*@return �Ƿ�ɹ�
* - RETURN_OK(0):  �ɹ�
* - ������ʧ��,������������
*/
FH_SINT32 FH_VPSS_VOU_SetAttr(const FH_VPU_CHN_CONFIG *pstChnconfig);

/**
* FH_VPSS_VOU_GetAttr
*@brief ��ȡVOU������Դ��ͨ������
*@param [in] ��
*@param [out] pstChnconfig VPUͨ��������ָ��
*@return �Ƿ�ɹ�
* - RETURN_OK(0):  �ɹ�
* - ������ʧ��,������������
*/
FH_SINT32 FH_VPSS_VOU_GetAttr(FH_VPU_CHN_CONFIG *pstChnconfig);


/**
* FH_VPSS_SetMask
*@brief ����VPU ͨ����Ƶ��������ͨ�����ò���Enable��Ա������Ч
*@param [in] pstVpumaskinfo  MASK������ָ��
*@param [out] ��
*@return �Ƿ�ɹ�
* - RETURN_OK(0):  �ɹ�
* - ������ʧ��,������������
*/
FH_SINT32 FH_VPSS_SetMask(const FH_VPU_MASK *pstVpumaskinfo);

/**
* FH_VPSS_GetMask
*@brief ��ȡVPU ͨ����Ƶ��������
*@param [in] ��
*@param [out] pstVpumaskinfo  MASK������ָ��
*@return �Ƿ�ɹ�
* - RETURN_OK(0):  �ɹ�
* - ������ʧ��,������������
*/
FH_SINT32 FH_VPSS_GetMask(FH_VPU_MASK *pstVpumaskinfo);

/**
* FH_VPSS_ClearMask
*@brief �����Ӧ����Ƶ��������
*@param [in] maskAreaIdx ��Ƶ���������,ȡֵ0-7
*@param [out] ��
*@return �Ƿ�ɹ�
* - RETURN_OK(0):  �ɹ�
* - ������ʧ��,������������
*/
FH_SINT32 FH_VPSS_ClearMask(FH_UINT32 clearmaskarea);

/**
* FH_VPSS_SetMask
*@brief ����VPUͨ��logo������Ϣ��ͨ�����ò���Enable��Ա������Ч
*@param [in] pstVpugraphinfo logo���Ӳ���ָ��
*@param [out] ��
*@return �Ƿ�ɹ�
* - RETURN_OK(0):  �ɹ�
* - ������ʧ��,������������
*/
FH_SINT32 FH_VPSS_SetGraph(const FH_VPU_LOGO  *pstVpugraphinfo);


/**
* FH_VPSS_GetGraph
*@brief ��ȡVPUͨ��logo������Ϣ
*@param [in] ��
*@param [out] pstVpugraphinfo logo���Ӳ���ָ��
*@return �Ƿ�ɹ�
* - RETURN_OK(0):  �ɹ�
* - ������ʧ��,������������
*/
FH_SINT32 FH_VPSS_GetGraph(FH_VPU_LOGO  *pstVpugraphinfo);

/**
* FH_VPSS_SetOsd
*@brief ����VPU ͨ���ַ�����
*@param [in] pstVpuosdinfo���ַ����Ӳ���ָ��
*@param [out] ��
*@return �Ƿ�ɹ�
* - RETURN_OK(0):  �ɹ�
* - ������ʧ��,������������
*/
FH_SINT32 FH_VPSS_SetOsd(const FH_VPU_OSD  *pstVpuosdinfo);

/**
* FH_VPSS_GetOsd
*@brief ��ȡVPU ͨ���ַ�������Ϣ
*@param [in] ��
*@param [out] pstVpuosdinfo���ַ����Ӳ���ָ��
*@return �Ƿ�ɹ�
* - RETURN_OK(0):  �ɹ�
* - ������ʧ��,������������
*/
FH_SINT32 FH_VPSS_GetOsd(FH_VPU_OSD  *pstVpuosdinfo);

/**
* FH_VPSS_SetRotate
*@brief VPU ͨ���ַ�������ת
*@param [in] Rotate ��ת�Ƕ�����
*@param [out] ��
*@return �Ƿ�ɹ�
* - RETURN_OK(0):  �ɹ�
* - ������ʧ��,������������
*/
FH_SINT32 FH_VPSS_SetRotate(const FH_OSD_ROTATE  Rotate);

/**
* FH_VPSS_SetOsdInvert
*@brief ����VPU ͨ���ַ����ӷ�ɫ������Ϣ
*@param [in] pstOsdinvertctl �ַ����ӵķ�ɫ����λָ��
*@param [out] ��
*@return �Ƿ�ɹ�
* - RETURN_OK(0):  �ɹ�
* - ������ʧ��,������������
*/
FH_SINT32 FH_VPSS_SetOsdInvert(const FH_INVERT_CTRL  *pstOsdinvertctl);

/**
* FH_VPSS_GetOsdInvert
*@brief ��ȡVPU ͨ���ַ����ӷ�ɫ������Ϣ
*@param [in] ��
*@param [out] pstOsdinvertctl �ַ����ӵķ�ɫ����λָ��
*@return �Ƿ�ɹ�
* - RETURN_OK(0):  �ɹ�
* - ������ʧ��,������������
*/
FH_SINT32 FH_VPSS_GetOsdInvert(FH_INVERT_CTRL  *pstOsdinvertctl);

/**
* FH_VPSS_EnableYCmean
*@brief ����YC��ֵͳ��ֵ��
*@param [in] ��
*@param [out] ��
*@return �Ƿ�ɹ�
* - RETURN_OK(0):  �ɹ�
* - ������ʧ��,������������
*/
FH_SINT32 FH_VPSS_EnableYCmean(void);

/**
* FH_VPSS_DisableYCmean
*@brief �ر�YC��ֵͳ��ֵ
*@param [in] ��
*@param [out] ��
*@return �Ƿ�ɹ�
* - RETURN_OK(0):  �ɹ�
* - ������ʧ��,������������
*/
FH_SINT32 FH_VPSS_DisableYCmean(void);

/**
* FH_VPSS_GetYCmean
*@brief ��ȡYC��ֵͳ��ֵ
*@param [in] ��
*@param [out] pstVpuycmeaninfo��YC��ֵ�������ָ��
*@return �Ƿ�ɹ�
* - RETURN_OK(0):  �ɹ�
* - ������ʧ��,������������
*/
FH_SINT32 FH_VPSS_GetYCmean(FH_VPU_YCMEAN *pstVpuycmeaninfo);

/**
* FH_VPSS_SetFramectrl
*@brief ����֡�ʿ��Ʋ���
*@param [in] chan ͨ���ţ�ȡֵ0-3
*@param [in]	pstFramerate��֡�ʿ��Ʋ���ָ��
*@param [out] ��
*@return �Ƿ�ɹ�
* - RETURN_OK(0):  �ɹ�
* - ������ʧ��,������������
*/
FH_SINT32 FH_VPSS_SetFramectrl(FH_UINT32 chan,const FH_FRAMERATE *pstVpuframectrl);

/**
* FH_VPSS_GetFramectrl
*@brief ��ȡ���õ�֡�ʿ��Ʋ���
*@param [in] chan ͨ���ţ�ȡֵ0-3
*@param [out] pstFramerate ֡�ʿ��Ʋ���ָ��
*@return �Ƿ�ɹ�
* - RETURN_OK(0):  �ɹ�
* - ������ʧ��,������������
*/
FH_SINT32 FH_VPSS_GetFramectrl(FH_UINT32 chan, FH_FRAMERATE *pstVpuframectrl);

/**
* FH_VPSS_GetFrameRate
*@brief ��ȡ��ǰͨ��ͳ�Ƶõ���֡��
*@param [in] chan ͨ���ţ�ȡֵ0-3
*@param [out] pstFramerate ֡��
*@return �Ƿ�ɹ�
* - RETURN_OK(0):  �ɹ�
* - ������ʧ��,������������
*/
FH_SINT32 FH_VPSS_GetFrameRate(FH_UINT32 chan,FH_FRAMERATE *fps);

/**
* FH_VPSS_LOW_LATENCY_Enable
*@brief ��������ʱģʽ��ͬʱֻ����һ��ͨ��ʹ�ܵ���ʱģʽ
*@param [in] chan ͨ���ţ�ȡֵ0-2
*@param [out] ��
*@return �Ƿ�ɹ�
* - RETURN_OK(0):  �ɹ�
* - ������ʧ��,������������
*/
FH_SINT32 FH_VPSS_LOW_LATENCY_Enable(FH_UINT32 chan);

/**
* FH_VPSS_LOW_LATENCY_Disable
*@brief ���õ���ʱģʽ
*@param [in] ��
*@param [out] ��
*@return �Ƿ�ɹ�
* - RETURN_OK(0):  �ɹ�
* - ������ʧ��,������������
*/
FH_SINT32 FH_VPSS_LOW_LATENCY_Disable(void);

/**
* FH_VPSS_SetCrop
*@brief ����VPU ͨ���ü����ܲ���
*@param [in] chan ͨ���ţ�ȡֵ0-3
*@param [in] pstVpucropinfo ͨ���ü����ܲ���ָ��
*@param [out] ��
*@return �Ƿ�ɹ�
* - RETURN_OK(0):  �ɹ�
* - ������ʧ��,������������
*/
FH_SINT32 FH_VPSS_SetCrop(FH_UINT32 chan,const FH_VPU_CROP *pstVpucropinfo);

/**
* FH_VPSS_GetCrop
*@brief ��ȡVPU ͨ���ü���������
*@param [in] chan ͨ���ţ�ȡֵ0-3
*@param [out] pstVpucropinfo ͨ���ü����ܲ���ָ��
*@return �Ƿ�ɹ�
* - RETURN_OK(0):  �ɹ�
* - ������ʧ��,������������
*/
FH_SINT32 FH_VPSS_GetCrop(FH_UINT32 chan,FH_VPU_CROP *pstVpucropinfo);


/**
* FH_VPSS_Reset
*@brief VPU ģ��reset,�ú��������¸�λ�ı�־λ������ִ�и�λ����
*				�����ز�����ʾreset��ɡ�
*@param [in] ��
*@param [out] ��
*@return �Ƿ�ɹ�
* - RETURN_OK(0):  �ɹ�
* - ������ʧ��,������������
*/
FH_SINT32  FH_VPSS_Reset(void);


/**
* FH_VPU_GetPkginfo
*@brief ��ȡVPU PKGģʽ�¼Ĵ���������ֵ
*@param [in] ��
*@param [out] pstVpupkginfo PKGģ���¼Ĵ�������ֵ
*@return �Ƿ�ɹ�
* - RETURN_OK(0):  �ɹ�
* - ������ʧ��,������������
*/
FH_SINT32 FH_VPU_GetPkginfo(FH_PKG_INFO *pstVpupkginfo);


/**
* һЩ���Ժ������û��������
*/
FH_SINT32 FH_VPSS_ReadMallocedMem(int intMemSlot, FH_UINT32 offset, FH_UINT32 *pstData);
FH_SINT32 FH_VPSS_WriteMallocedMem(int intMemSlot, FH_UINT32 offset, FH_UINT32 *pstData);
FH_SINT32 FH_VPSS_ImportMallocedMem(int intMemSlot, FH_UINT32 offset, FH_UINT32 *pstSrc, FH_UINT32 size);
FH_SINT32 FH_VPSS_ExportMallocedMem(int intMemSlot, FH_UINT32 offset, FH_UINT32 *pstDst, FH_UINT32 size);

/**
* fh_vpu_init
*@brief VPU������ʼ�������ڲ����ã��û��������Ե���
*@param [in] ��
*@param [out] ��
*@return �Ƿ�ɹ�
* - RETURN_OK(0):  �ɹ�
* - ������ʧ��,������������
*/
FH_SINT32 fh_vpu_init();

/**
* fh_vpu_close
*@brief VPU������ʼ�������ڲ����ã��û��������Ե���
*@param [in] ��
*@param [out] ��
*@return �Ƿ�ɹ�
* - RETURN_OK(0):  �ɹ�
* - ������ʧ��,������������
*/
FH_SINT32 fh_vpu_close();


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
#endif /*__MPI_VO_H__ */
