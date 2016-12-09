#ifndef __FH_JPEG_MPI_H__
#define __FH_JPEG_MPI_H__

#include "types/type_def.h"
#include "fh_common.h"
#include "fh_jpeg_mpipara.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"
{
#endif
#endif /* __cplusplus */

/**
* FH_JPEG_InitMem
*@brief ����JPEGģ����Ҫ���ڴ�
*@param [in] Jpegwidth��JPEG����Ŀ�ȡ�
*@param [in] Jpeghight��JPEG����ĸ߶ȡ�
*@param [out] ��
*@return �Ƿ�ɹ�
* - RETURN_OK(0):  �ɹ�
* - ������ʧ��,������������
*/
FH_SINT32  FH_JPEG_InitMem(FH_UINT32 Jpegwidth,FH_UINT32 Jpeghight);

/**
* FH_JPEG_Setconfig
*@brief ����JPEG���ò���
*@param [in] pstJpegconfig JPEG�������ò���
*@param [out] ��
*@return �Ƿ�ɹ�
* - RETURN_OK(0):  �ɹ�
* - ������ʧ��,������������
*/
FH_SINT32  FH_JPEG_Setconfig(const FH_JPEG_CONFIG *pstJpegconfig);

/**
* FH_JPEG_Getconfig
*@brief ��ȡJPEG���ò���
*@param [in] ��
*@param [out] pstJpegconfig��JPEG�������ò���
*@return �Ƿ�ɹ�
* - RETURN_OK(0):  �ɹ�
* - ������ʧ��,������������
*/
FH_SINT32  FH_JPEG_Getconfig(FH_JPEG_CONFIG *pstJpegconfig);

/**
* FH_JPEG_Setqp
*@brief ����JPEG�����QPֵ����̬������
*@param [in] QP  JPEG�����QPֵ
*@param [out] ��
*@return �Ƿ�ɹ�
* - RETURN_OK(0):  �ɹ�
* - ������ʧ��,������������
*/
FH_SINT32  FH_JPEG_Setqp(FH_UINT32 QP);

/**
* FH_JPEG_Getqp
*@brief ��ȡJPEG�����QPֵ��
*@param [in] ��
*@param [out] QP  JPEG�����QPֵ
*@return �Ƿ�ɹ�
* - RETURN_OK(0):  �ɹ�
* - ������ʧ��,������������
*/
FH_SINT32  FH_JPEG_Getqp(FH_UINT32 *QP);

/**
* FH_JPEG_Setstream
*@brief �û��ֶ��ύһ֡ͼ���JPEG����
*@param [in] pstJpegframe �����������Ϣ
*@param [out] ��
*@return �Ƿ�ɹ�
* - RETURN_OK(0):  �ɹ�
* - ������ʧ��,������������
*/
FH_SINT32  FH_JPEG_Setstream(const FH_JPEG_FRAME_INFO *pstJpegframe);

/**
* FH_JPEG_Getstream
*@brief ��ȡJPEG�����������Ϣ����������ֻ��һ֡��buf����Ҫ��ͼ����ȫȡ�ߺ󣬲ſ���������һ֡���롣
*@param [in] ��
*@param [out] pstJpegconfig �����������Ϣ
*@return �Ƿ�ɹ�
* - RETURN_OK(0):  �ɹ�
* - ������ʧ��,������������
*/
FH_SINT32  FH_JPEG_Getstream(FH_JPEG_STREAM_INFO  *pstJpegframe);

/**
* FH_JPEG_Getstream_Block
*@brief ��ȡJPEG�����������Ϣ��������ʽ��ֻ��һ֡��buf����Ҫ��ͼ����ȫȡ�ߺ󣬲ſ���������һ֡���롣
*@param [in] ��
*@param [out] pstJpegconfig �����������Ϣ
*@return �Ƿ�ɹ�
* - RETURN_OK(0):  �ɹ�
* - ������ʧ��,������������
*/
FH_SINT32  FH_JPEG_Getstream_Block(FH_JPEG_STREAM_INFO  *pstJpegframe);


/**
* fh_jpeg_init
*@brief JPEG������ʼ�������ڲ����ã��û��������Ե���
*@param [in] ��
*@param [out] ��
*@return �Ƿ�ɹ�
* - RETURN_OK(0):  �ɹ�
* - ������ʧ��,������������
*/
FH_SINT32 fh_jpeg_init();

/**
* fh_jpeg_close
*@brief JPEG������ʼ�������ڲ����ã��û��������Ե���
*@param [in] ��
*@param [out] ��
*@return �Ƿ�ɹ�
* - RETURN_OK(0):  �ɹ�
* - ������ʧ��,������������
*/
FH_SINT32 fh_jpeg_close();

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
#endif /*__MPI_VO_H__ */
