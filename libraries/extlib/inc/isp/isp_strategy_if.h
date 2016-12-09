#ifndef _ISP_STRATEGY_IF_H_
#define _ISP_STRATEGY_IF_H_

#include "types/type_def.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"
{
#endif
#endif /* __cplusplus */

/**
 * @brief		ISP����ģ����࣬ÿ������ģ��Ӧ�ü̳и��࣬����ʼ����ͨ����װ���ԣ�
 *				�������ISP Core�С�
 */
struct isp_strategy		// module interface
{
	FH_SINT8*	name;			/*!< �������� */
	FH_VOID	(*run)(FH_VOID);	/*!< �������к���ָ�� */
};

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* _ISP_STRATEGY_H_ */
