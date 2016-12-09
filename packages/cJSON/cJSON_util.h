#ifndef __CJSON_UTIL_INCLUDE__
#define __CJSON_UTIL_INCLUDE__

#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "cJSON.h"

extern void cJSON_free(void *ptr);

extern const char * cJSON_item_get_string(cJSON *object,const char *string);
extern int cJSON_item_get_number(cJSON *object, const char *item_name, int * result);

#endif /* __CJSON_UTIL_INCLUDE__ */
