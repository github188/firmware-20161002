#ifndef DI_CONFIG_H_
#define DI_CONFIG_H_

#include "types/type_def.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"
{
#endif
#endif /* __cplusplus */

typedef enum{
	DIT_ETH,
	DIT_UART
}DI_TYPE;

typedef struct tagDI_Config
{
	DI_TYPE type;
	union {
		struct {
			int baudrate;
			int data_bits;
		}uart;

		struct {
			char debug_ip[16];
			FH_UINT16 debug_port;
		}eth;
	}conf;
}DI_Config;


void DebugInterface_Init(DI_Config* di);
void DebugInterface_Start();
void* DebugInterface_MainLoop(void* param);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* DI_CONFIG_H_ */
