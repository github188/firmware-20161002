#ifndef __FH_SYSTEM_MPI_H__
#define __FH_SYSTEM_MPI_H__

#include "types/type_def.h"
#include "fh_common.h"


#ifdef __cplusplus
#if __cplusplus
extern "C"
{
#endif
#endif /* __cplusplus */

/**
* FH_SYS_Init
*@brief DSP ϵͳ��ʼ������ɴ������豸������ϵͳ�ڴ�
*@param [in] ��
*@param [out] ��
*@return �Ƿ�ɹ�
* - RETURN_OK(0):  �ɹ�
* - ������ʧ��,������������
*/
FH_SINT32  FH_SYS_Init();

/**
* FH_SYS_Exit
*@brief DSP ϵͳ�˳�
*@param [in] ��
*@param [out] ��
*@return �Ƿ�ɹ�
* - RETURN_OK(0):  �ɹ�
* - ������ʧ��,������������
*/
FH_SINT32  FH_SYS_Exit();


/**
* FH_SYS_BindVpu2Enc
*@brief ����Դ�󶨵�H264����ͨ��
*@param [in] srcchn(VPU ͨ��),ȡֵΪ0 - 2
*@param [in] dstschn(ENC ͨ��)��ȡֵΪ0 - 7
*@param [out] ��
*@return �Ƿ�ɹ�
* - RETURN_OK(0):  �ɹ�
* - ������ʧ��,������������
*/
FH_SINT32  FH_SYS_BindVpu2Enc(FH_UINT32 srcchn,FH_UINT32 dstschn);

/**
* FH_SYS_BindVpu2Vou
*@brief ����Դ�󶨵���ʾͨ��
*@param [in] ��
*@param [out] ��
*@return �Ƿ�ɹ�
* - RETURN_OK(0):  �ɹ�
* - ������ʧ��,������������
*/
FH_SINT32  FH_SYS_BindVpu2Vou();

/**
* FH_SYS_BindVpu2Jpeg
*@brief ����Դ��JPEG����ͨ����
*@param [in] srcchn(VPU ͨ��),ȡֵΪ0 - 2
*@param [out] ��
*@return �Ƿ�ɹ�
* - RETURN_OK(0):  �ɹ�
* - ������ʧ��,������������
*/
FH_SINT32  FH_SYS_BindVpu2Jpeg(FH_UINT32 srcchn);


/**
* FH_SYS_GetBindbyDest
*@brief ��ȡH264����ͨ���İ󶨶���
*@param [in] dstschn(ENC ͨ��)��ȡֵΪ0 - 7
*@param [out] srcchn(VPU ͨ��),ȡֵΪ0 - 2
*@return �Ƿ�ɹ�
* - RETURN_OK(0):  �ɹ�
* - ������ʧ��,������������
*/
FH_SINT32 FH_SYS_GetBindbyDest(FH_UINT32 *srcchn,FH_UINT32 dstschn);

/**
* FH_SYS_UnBindbySrc
*@brief ������ݰ󶨹�ϵ
*@param [in] srcchn(VPU ͨ��),ȡֵΪ0 - 2
*@param [out] ��
*@return �Ƿ�ɹ�
* - RETURN_OK(0):  �ɹ�
* - ������ʧ��,������������
*/
FH_SINT32  FH_SYS_UnBindbySrc(FH_UINT32 srcchn);

/**
* FH_SYS_GetVersion
*@brief ��ȡDSP�����汾��Ϣ
*@param [in] ��
*@param [out] pstSystemversion �汾��Ϣ
*@return �Ƿ�ɹ�
* - RETURN_OK(0):  �ɹ�
* - ������ʧ��,������������
*/
FH_SINT32  FH_SYS_GetVersion(FH_VERSION_INFO *pstSystemversion);

/**
* FH_SYS_SetReg
*@brief ���üĴ���ֵ
*@param [in] addr �Ĵ��������ַ
*@param [in] value ���üĴ���ֵ
*@param [out] ��
*@return �Ƿ�ɹ�
* - RETURN_OK(0):  �ɹ�
* - ������ʧ��,������������
*/
FH_SINT32 FH_SYS_SetReg(FH_UINT32 addr, FH_UINT32 value);

/**
* FH_SYS_GetReg
*@brief ��ȡ�Ĵ�����ֵ
*@param [in] u32Addr �Ĵ��������ַ
*@param [out] pu32Value �Ĵ�����ֵ
*@return �Ƿ�ɹ�
* - RETURN_OK(0):  �ɹ�
* - ������ʧ��,������������
*/
FH_SINT32 FH_SYS_GetReg(FH_UINT32 u32Addr, FH_UINT32 *pu32Value);


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
#endif /*__MPI_VO_H__ */
