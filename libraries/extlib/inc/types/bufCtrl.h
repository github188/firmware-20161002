#ifndef _BUFCTRL_H_
#define _BUFCTRL_H_
#include "dsp/fh_common.h"

typedef struct fhMEM_DESC_
{
	unsigned int 			base;
	void 					*vbase;
	unsigned int 			size;
	unsigned int 			align;
}MEM_DESC;

#define   mem_desc   fhMEM_DESC_


int bufferInit(unsigned char* pAddr, unsigned int bufSize);
int buffer_malloc(MEM_DESC *mem, int size, int align);
int buffer_malloc_withname(MEM_DESC *mem, int size, int align,  char* name);


#endif
