#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "cJSON.h"
#include "cJSON_util.h"

const char * cJSON_item_get_string(cJSON *object, const char *item_name)
{
    cJSON *item;
    const char * string;

    item = cJSON_GetObjectItem(object, item_name);

    if(!item)
        return 0;

    if( (item->type != cJSON_String) && (item->type != cJSON_Array) )
        return 0;

    if(item->type == cJSON_Array)
        return item->child->valuestring; // TODO

    return item->valuestring;
}

int cJSON_item_get_number(cJSON *object, const char *item_name, int * result)
{
    cJSON *item;
    const char * string;

    item = cJSON_GetObjectItem(object, item_name);

    if(!item)
        return -1;

    if(item->type != cJSON_Number)
        return -1;

    if(result)
        *result = item->valueint;

    return 0;
}
