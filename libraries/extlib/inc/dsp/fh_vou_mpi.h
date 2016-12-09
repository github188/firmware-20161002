#ifndef __FH_VOU_MPI_H__
#define __FH_VOU_MPI_H__

#include "fh_vou_mpipara.h"
#include "types/type_def.h"
#include "fh_common.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"
{
#endif
#endif /* __cplusplus */

/**
* FH_VOU_Enable
*@brief ������Ƶ����豸
*@param [in] ��
*@param [out] ��
*@return �Ƿ�ɹ�
* - RETURN_OK(0):  �ɹ�
* - ������ʧ��,������������
*/
FH_SINT32 FH_VOU_Enable();

/**
* FH_VOU_Disable
*@brief ������Ƶ����豸
*@param [in] ��
*@param [out] ��
*@return �Ƿ�ɹ�
* - RETURN_OK(0):  �ɹ�
* - ������ʧ��,������������
*/
FH_SINT32 FH_VOU_Disable();

/**
* FH_VOU_SetConfig
*@brief ������Ƶ����豸���������ԡ�
*@param [in] pstVouconfig�����ò���ָ��
*@param [out] ��
*@return �Ƿ�ɹ�
* - RETURN_OK(0):  �ɹ�
* - ������ʧ��,������������
*/
FH_SINT32 FH_VOU_SetConfig(const FH_VOU_PIC_CONFIG *pstVouconfig);

/**
* FH_VOU_GetConfig
*@brief ��ȡ��Ƶ����豸���������ԡ�
*@param [in] ��
*@param [out] pstVouconfig�����ò���ָ��
*@return �Ƿ�ɹ�
* - RETURN_OK(0):  �ɹ�
* - ������ʧ��,������������
*/
FH_SINT32 FH_VOU_GetConfig(FH_VOU_PIC_SIZE *pstVouconfig);
/**
* FH_VOU_Enable
*@brief ������Ƶ����豸
*@param [in] ��
*@param [out] ��
*@return �Ƿ�ɹ�
* - RETURN_OK(0):  �ɹ�
* - ������ʧ��,������������
*/
FH_SINT32 FH_VOU_SendFrame(const FH_VOU_PIC_INFO *pstVouframeinfo);

/**
* fh_vou_init
*@brief ��Ƶ��ʾ������ʼ�������ڲ����ã��û��������Ե���
*@param [in] ��
*@param [out] ��
*@return �Ƿ�ɹ�
* - RETURN_OK(0):  �ɹ�
* - ������ʧ��,������������
*/
FH_SINT32 fh_vou_init();

/**
* fh_vou_close
*@brief ��Ƶ��ʾ�����˳������ڲ����ã��û��������Ե���
*@param [in] ��
*@param [out] ��
*@return �Ƿ�ɹ�
* - RETURN_OK(0):  �ɹ�
* - ������ʧ��,������������
*/
FH_SINT32 fh_vou_close();

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /*__MPI_VO_H__ */
