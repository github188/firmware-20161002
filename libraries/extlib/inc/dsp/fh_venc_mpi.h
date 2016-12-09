

#ifndef __FH_VENC_MPI_H__
#define __FH_VENC_MPI_H__

#include "fh_venc_mpipara.h"
#include "types/type_def.h"
#include "fh_common.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"
{
#endif
#endif /* __cplusplus */

/**
* FH_VENC_SysInitMem
*@brief VENCϵͳ�ڴ��ʼ�������ڲ����ã��û�����Ҫ���Ե���
*@param [in] ��
*@param [out] ��
*@return �Ƿ�ɹ�
* - RETURN_OK(0):  �ɹ�
* - ������ʧ��,������������
*/
FH_SINT32  FH_VENC_SysInitMem();


/**
* FH_VENC_CreateChn
*@brief ��������ͨ��������ͨ��������ֵ��
*@param [in] chan ͨ���ţ�ȡֵ0-7
*@param [out] stVencChnAttr ͨ������ָ��
*@return �Ƿ�ɹ�
* - RETURN_OK(0):  �ɹ�
* - ������ʧ��,������������
*/
FH_SINT32  FH_VENC_CreateChn(FH_UINT32 chan,const FH_CHR_CONFIG *stVencChnAttr);

/**
* FH_VENC_StartRecvPic
*@brief ��ʼ����ͼƬ������б��롣
*@param [in] chan ͨ���ţ�ȡֵ0-7
*@param [out] ��
*@return �Ƿ�ɹ�
* - RETURN_OK(0):  �ɹ�
* - ������ʧ��,������������
*/
FH_SINT32  FH_VENC_StartRecvPic(FH_UINT32 chan);

/**
* FH_VENC_StopRecvPic
*@brief ֹͣ���롣
*@param [in] chan ͨ���ţ�ȡֵ0-7
*@param [out] ��
*@return �Ƿ�ɹ�
* - RETURN_OK(0):  �ɹ�
* - ������ʧ��,������������
*/
FH_SINT32  FH_VENC_StopRecvPic(FH_UINT32 chan);

/**
* FH_VENC_Submit_ENC
*@brief VENCû�к�VPU����ͨ���󶨣�ʹ���û��ṩͼƬ���б��롣
*@param [in] chan ͨ���ţ�ȡֵ0-7
*@param [in] pstVencsubmitframe �û��ṩ��ͼƬ��Ϣָ��
*@param [out] ��
*@return �Ƿ�ɹ�
* - RETURN_OK(0):  �ɹ�
* - ������ʧ��,������������
*/
FH_SINT32  FH_VENC_Submit_ENC(FH_UINT32 chan,const FH_ENC_FRAME *pstVencsubmitframe);

/**
* FH_VENC_Query
*@brief ��ѯ������״̬��Ϣ��
*@param [in] ��
*@param [out] pstVencstatus ����״̬��Ϣָ�롣
*@return �Ƿ�ɹ�
* - RETURN_OK(0):  �ɹ�
* - ������ʧ��,������������
*/
FH_SINT32  FH_VENC_Query(FH_SYS_STATUS *pstVencstatus);

/**
* FH_VENC_GetStream
*@brief ��ȡ����ͨ��������������
*@param [in] chan ͨ���ţ�ȡֵ0-7
*@param [out] pstVencstreamAttr ����ͨ��������Ϣָ�롣
*@return �Ƿ�ɹ�
* - RETURN_OK(0):  �ɹ�
* - ������ʧ��,������������
*/
FH_SINT32  FH_VENC_GetStream (FH_UINT32 chan,FH_ENC_STREAM_ELEMENT *pstVencstreamAttr);

/**
* FH_VENC_ReleaseStream
*@brief �ͷ��������档
*@param [in] chan ͨ���ţ�ȡֵ0-7
*@param [out] ��
*@return �Ƿ�ɹ�
* - RETURN_OK(0):  �ɹ�
* - ������ʧ��,������������
*/
FH_SINT32  FH_VENC_ReleaseStream(FH_UINT32 chan);

/**
* FH_VENC_SetChnAttr
*@brief ���ñ���ͨ�����ԡ�ͨ������֮�����ͨ���˽ӿڸı�ͨ�����ԡ�
*@param [in] chan ͨ���ţ�ȡֵ0-7
*@param [in] pstVencChnAttr ����ͨ������ָ��
*@param [out] ��
*@return �Ƿ�ɹ�
* - RETURN_OK(0):  �ɹ�
* - ������ʧ��,������������
*/
FH_SINT32  FH_VENC_SetChnAttr(FH_UINT32 chan,const FH_CHR_CONFIG *pstVencChnAttr);

/**
* FH_VENC_GetChnAttr
*@brief ��ȡ����ͨ���ı������ԡ�
*@param [in] chan ͨ���ţ�ȡֵ0-7
*@param [out] pstVencChnAttr ����ͨ������ָ��
*@return �Ƿ�ɹ�
* - RETURN_OK(0):  �ɹ�
* - ������ʧ��,������������
*/
FH_SINT32  FH_VENC_GetChnAttr(FH_UINT32 chan, FH_CHR_CONFIG *pstVencChnAttr);

/**
* FH_VENC_SetRotate
*@brief ����ͼ����ת��
*@param [in] chan ͨ���ţ�ȡֵ0-7
*@param [in] pstVencrotateinfo ��ת��Ϣָ��
*@param [out] ��
*@return �Ƿ�ɹ�
* - RETURN_OK(0):  �ɹ�
* - ������ʧ��,������������
*/
FH_SINT32  FH_VENC_SetRotate(FH_UINT32 chan,const FH_ROTATE *pstVencrotateinfo);

/**
* FH_VENC_GetRotate
*@brief ����ͼ����ת��
*@param [in] chan ͨ���ţ�ȡֵ0-7
*@param [out] pstVencrotateinfo ��ת��Ϣָ��
*@return �Ƿ�ɹ�
* - RETURN_OK(0):  �ɹ�
* - ������ʧ��,������������
*/
FH_SINT32  FH_VENC_GetRotate(FH_UINT32 chan, FH_ROTATE *pstVencrotateinfo);

/**
* FH_VENC_SetRoiCfg
*@brief ����ͼ����ת��
*@param [in] chan ͨ���ţ�ȡֵ0-7
*@param [in] pstVencroiinfo ROI��Ϣָ��
*@param [out] ��
*@return �Ƿ�ɹ�
* - RETURN_OK(0):  �ɹ�
* - ������ʧ��,������������
*/
FH_SINT32  FH_VENC_SetRoiCfg(FH_UINT32 chan,const FH_ROI *pstVencroiinfo);

/**
* FH_VENC_GetRoiCfg
*@brief ��ȡ����ͼ���ROI��Ϣ
*@param [in] chan ͨ���ţ�ȡֵ0-7
*@param [out] pstVencroiinfo ROI��Ϣָ��
*@return �Ƿ�ɹ�
* - RETURN_OK(0):  �ɹ�
* - ������ʧ��,������������
*/
FH_SINT32  FH_VENC_GetRoiCfg(FH_UINT32 chan, FH_ROI *pstVencroiinfo);

/**
* FH_VENC_ClearRoi
*@brief �������ͨ����ROI��Ϣ���ڵ�һ������ͨ��ROI����Ҫ����ɵ�����ʱ����
*@param [in] chan ͨ���ţ�ȡֵ0-7
*@param [out] ��
*@return �Ƿ�ɹ�
* - RETURN_OK(0):  �ɹ�
* - ������ʧ��,������������
*/
FH_SINT32  FH_VENC_ClearRoi(FH_UINT32 chan);

/**
* FH_VENC_SetH264eRefMode
*@brief ����H.264 ����ͨ����֡�ο�ģʽ��
*@param [in] chan ͨ���ţ�ȡֵ0-7
*@param [in] Vencreferencemode ��֡ѡ��
*@param [out] ��
*@return �Ƿ�ɹ�
* - RETURN_OK(0):  �ɹ�
* - ������ʧ��,������������
*/
FH_SINT32  FH_VENC_SetH264eRefMode(FH_UINT32 chan, FH_REF_MODE_OPS  Vencreferencemode);

/**
* FH_VENC_GetH264eRefMode
*@brief ��ȡH.264 ����ͨ����֡�ο�ģʽ
*@param [in] chan ͨ���ţ�ȡֵ0-7
*@param [out] pVencreferencemode ��֡ѡ��
*@return �Ƿ�ɹ�
* - RETURN_OK(0):  �ɹ�
* - ������ʧ��,������������
*/
FH_SINT32  FH_VENC_GetH264eRefMode(FH_UINT32 chan, FH_REF_MODE_OPS  *pVencreferencemode);

/**
* FH_VENC_SetH264RcIDivP
*@brief ����H.264 ����ͨ����������ʱ��I֡P֡����
*@param [in] chan ͨ���ţ�ȡֵ0-7
*@param [in] pVencrcidivp I֡P֡�����Ĳ���ָ��
*@param [out] ��
*@return �Ƿ�ɹ�
* - RETURN_OK(0):  �ɹ�
* - ������ʧ��,������������
*/
FH_SINT32 FH_VENC_SetH264RcIDivP(FH_UINT32 chan, const FH_RC_I_DIV_P  *pVencrcidivp);

/**
* FH_VENC_GetH264RcIDivP
*@brief ��ȡH.264 ����ͨ����������ʱ��I֡P֡����
*@param [in] chan ͨ���ţ�ȡֵ0-7
*@param [out] pVencrcidivp �ر���Ĳ���ָ��
*@return �Ƿ�ɹ�
* - RETURN_OK(0):  �ɹ�
* - ������ʧ��,������������
*/
FH_SINT32 FH_VENC_GetH264RcIDivP(FH_UINT32 chan, FH_RC_I_DIV_P  *pVencrcidivp);

/**
* FH_VENC_SetH264RcFluctateLevel
*@brief ����H.264 ����ͨ����������ʱ���������ȼ�
*@param [in] chan ͨ���ţ�ȡֵ0-7
*@param [out] fluctuate_level ���������ȼ�0 - 6,ԽС����Խ��ͼ�������仯Խƽ��
*@return �Ƿ�ɹ�
* - RETURN_OK(0):  �ɹ�
* - ������ʧ��,������������
*/
FH_SINT32 FH_VENC_SetH264RcFluctateLevel(FH_UINT32 chan, FH_UINT32 fluctuate_level);

/**
* FH_VENC_GetH264RcFluctateLevel
*@brief ��ȡH.264 ����ͨ����������ʱ���������ȼ�
*@param [in] chan ͨ���ţ�ȡֵ0-7
*@param [out] fluctuate_level ���������ȼ�0 - 6,ԽС����Խ��ͼ�������仯Խƽ��?
*@return �Ƿ�ɹ�
* - RETURN_OK(0):  �ɹ�
* - ������ʧ��,������������
*/
FH_SINT32 FH_VENC_GetH264RcFluctateLevel(FH_UINT32 chan, FH_UINT32 *fluctuate_level);

/**
* FH_VENC_SetH264Entropy
*@brief ����H.264 ����ͨ���ر���ģʽ
*@param [in] chan ͨ���ţ�ȡֵ0-7
*@param [in] pstVencentropy �ر���Ĳ���ָ��
*@param [out] ��
*@return �Ƿ�ɹ�
* - RETURN_OK(0):  �ɹ�
* - ������ʧ��,������������
*/
FH_SINT32  FH_VENC_SetH264Entropy(FH_UINT32 chan,const FH_ENTROPY_OPS *pstVencentropy);

/**
* FH_VENC_GetH264Entropy
*@brief ��ȡH.264 ����ͨ���ر���ģʽ
*@param [in] chan ͨ���ţ�ȡֵ0-7
*@param [out] pstVencentropy �ر���Ĳ���ָ��
*@return �Ƿ�ɹ�
* - RETURN_OK(0):  �ɹ�
* - ������ʧ��,������������
*/
FH_SINT32  FH_VENC_GetH264Entropy(FH_UINT32 chan, FH_ENTROPY_OPS *pstVencentropy);

/**
* FH_VENC_SET_ADV_DEBLOCKING_FILTER
*@brief ����H.264 ����ͨ����·�˲�����
*@param [in] chan ͨ���ţ�ȡֵ0-7
*@param [in] pstVencentropy ��·�˲�����ָ��
*@param [out] ��
*@return �Ƿ�ɹ�
* - RETURN_OK(0):  �ɹ�
* - ������ʧ��,������������
*/
FH_SINT32  FH_VENC_SET_ADV_DEBLOCKING_FILTER(FH_UINT32 chan,const FH_DEBLOCKING_FILTER_PARAM *pstVencfilter);

/**
* FH_VENC_GET_ADV_DEBLOCKING_FILTER
*@brief ��ȡH.264 ����ͨ����·�˲�����
*@param [in] chan ͨ���ţ�ȡֵ0-7
*@param [out] pstVencentropy ��·�˲�����ָ��
*@return �Ƿ�ɹ�
* - RETURN_OK(0):  �ɹ�
* - ������ʧ��,������������
*/
FH_SINT32  FH_VENC_GET_ADV_DEBLOCKING_FILTER(FH_UINT32 chan, FH_DEBLOCKING_FILTER_PARAM *pstVencfilter);

/**
* FH_VENC_SET_ADV_INTERMBSCE
*@brief ����H.264 ����ͨ����ϵ������ģʽ��
*@param [in] chan ͨ���ţ�ȡֵ0-7
*@param [in] Vencintermbsce ��ϵ������ѡ��
*@param [out] ��
*@return �Ƿ�ɹ�
* - RETURN_OK(0):  �ɹ�
* - ������ʧ��,������������
*/
FH_SINT32  FH_VENC_SET_ADV_INTERMBSCE(FH_UINT32 chan, FH_INTERMBSCE_OPS Vencintermbsce);/*���õ�ϵ������*/

/**
* FH_VENC_GET_ADV_INTERMBSCE
*@brief ��ȡH.264 ����ͨ����ϵ������ģʽ��
*@param [in] chan ͨ���ţ�ȡֵ0-7
*@param [out] pVencintermbsce ��·�˲�����ָ��
*@return �Ƿ�ɹ�
* - RETURN_OK(0):  �ɹ�
* - ������ʧ��,������������
*/
FH_SINT32  FH_VENC_GET_ADV_INTERMBSCE(FH_UINT32 chan, FH_INTERMBSCE_OPS *pVencintermbsce);

/**
* FH_VENC_SET_ADV_SLICE_SPLIT
*@brief ����H.264 ����ͨ��Slice�ָ�
*@param [in] chan ͨ���ţ�ȡֵ0-7
*@param [in] pstVencslicesplit Slice�ָ�����ֵ
*@param [out] ��
*@return �Ƿ�ɹ�
* - RETURN_OK(0):  �ɹ�
* - ������ʧ��,������������
*/
FH_SINT32  FH_VENC_SET_ADV_SLICE_SPLIT(FH_UINT32 chan,const FH_SLICE_SPLIT *pstVencslicesplit);	/*����Slice�ָ�*/

/**
* FH_VENC_GET_ADV_SLICE_SPLIT
*@brief ��ȡH.264 ����ͨ��Slice�ָ�
*@param [in] chan ͨ���ţ�ȡֵ0-7
*@param [out] pstVencslicesplit Slice�ָ�����ֵ
*@return �Ƿ�ɹ�
* - RETURN_OK(0):  �ɹ�
* - ������ʧ��,������������
*/
FH_SINT32  FH_VENC_GET_ADV_SLICE_SPLIT(FH_UINT32 chan, FH_SLICE_SPLIT *pstVencslicesplit);


/**
* FH_VENC_RequestIDR
*@brief ����ǿ��I ֡��
*@param [in] chan ͨ���ţ�ȡֵ0-7
*@param [out] ��
*@return �Ƿ�ɹ�
* - RETURN_OK(0):  �ɹ�
* - ������ʧ��,������������
*/
FH_SINT32  FH_VENC_RequestIDR(FH_UINT32 chan);

/**
* FH_VENC_SetCurPts
*@brief ����PTS��ʱ�䣬���ڲ����ã��û��������Ե���
*@param [in] Systemstartpts ���õ�PTSֵ
*@param [out] ��
*@return �Ƿ�ɹ�
* - RETURN_OK(0):  �ɹ�
* - ������ʧ��,������������
*/
FH_SINT32  FH_VENC_SetCurPts(FH_UINT64 Systemstartpts);

/**
* ��ȡ��ǰ��ϵͳPTSֵ
*@brief ����PTS��ʱ�䣬���ڲ����ã��û��������Ե���
*@param [in] ��
*@param [out] Systemcurpts����ǰ��PTSֵ
*@return �Ƿ�ɹ�
* - RETURN_OK(0):  �ɹ�
* - ������ʧ��,������������
*/
FH_SINT32  FH_VENC_GetCurPts(FH_UINT64 *Systemcurpts);


/**
* fh_enc_init
*@brief ������������ʼ�������ڲ����ã��û��������Ե���
*@param [in] ��
*@param [out] ��
*@return �Ƿ�ɹ�
* - RETURN_OK(0):  �ɹ�
* - ������ʧ��,������������
*/
FH_SINT32 fh_enc_init( );

/**
* fh_enc_close
*@brief �����������˳������ڲ����ã��û��������Ե���
*@param [in] ��
*@param [out] ��
*@return �Ƿ�ɹ�
* - RETURN_OK(0):  �ɹ�
* - ������ʧ��,������������
*/
FH_SINT32 fh_enc_close();

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
#endif /*__MPI_VO_H__ */



