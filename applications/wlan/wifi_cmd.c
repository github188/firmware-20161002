#include <rtthread.h>
#include <stdlib.h>
#include <stdio.h>

#include <wifi.h>

#include <lwip/sockets.h>
#include <dfs_posix.h>
#include <cJSON.h>
#include <finsh.h>

static wiced_ssid_t ap_ssid =
{
    .length = 32,
    .value  = {0},
};
static uint8_t ap_key[32] = {0};

int wifi_read_cfg(void)
{
    int fd;
    cJSON *json = RT_NULL;

    fd = open("/setting.json", O_RDONLY, 0);
    if (fd >= 0)
    {
        int length;

        length = lseek(fd, 0, SEEK_END);
        if (length)
        {
            uint8_t *json_str = (uint8_t*) rt_malloc (length);
            if (json_str)
            {
                lseek(fd, 0, SEEK_SET);
                read(fd, json_str, length);

                json = cJSON_Parse(json_str);
                rt_free(json_str);
            }
        }
        close(fd);
    }
    else return -1;

    if (json)
    {
        cJSON *wifi = cJSON_GetObjectItem(json, "wifi");
        cJSON *ssid = cJSON_GetObjectItem(wifi, "SSID");
        cJSON *key  = cJSON_GetObjectItem(wifi, "Key");

        if (ssid)
        {
            rt_kprintf("ssid: %s\n", ssid->valuestring);
            memset(ap_ssid.value, 0x0, sizeof(ap_ssid.value));
            rt_strncpy(ap_ssid.value, ssid->valuestring, sizeof(ap_ssid.value) - 1);
            ap_ssid.length = rt_strlen(ssid->valuestring);
        }

        if (key)
        {
            memset(ap_key, 0x0, sizeof(ap_key));
            rt_strncpy(ap_key, key->valuestring, sizeof(ap_key) - 1);
        }

        cJSON_Delete(json);
    }

    return 0;
}

int wifi_save_cfg(const char* SSID, const char* Key)
{
    int fd;
    cJSON *json = RT_NULL;

    fd = open("/setting.json", O_RDONLY, 0);
    if (fd >= 0)
    {
        int length;

        length = lseek(fd, 0, SEEK_END);
        if (length)
        {
            uint8_t *json_str = (uint8_t*) rt_malloc (length);
            if (json_str)
            {
                lseek(fd, 0, SEEK_SET);
                read(fd, json_str, length);

                json = cJSON_Parse(json_str);
                rt_free(json_str);
            }
        }
        close(fd);
    }
    else
    {
        /* create a new setting.json */

        fd = open("/setting.json", O_WRONLY | O_TRUNC, 0);
        if (fd >= 0)
        {
            json = cJSON_CreateObject();
            if (json)
            {
                cJSON *wifi = cJSON_CreateObject();
                
                if (wifi)
                {
                    uint8_t *json_str;

                    cJSON_AddItemToObject(json, "wifi", wifi);
                    cJSON_AddStringToObject(wifi, "SSID", SSID);
                    cJSON_AddStringToObject(wifi, "Key", Key);

                    json_str = cJSON_Print(json);
                    if (json_str)
                    {
                        write(fd, json_str, rt_strlen(json_str));
                        rt_free(json_str);
                    }
                }
            }
        }
        close(fd);

        return 0;
    }

    if (json)
    {
        cJSON *wifi = cJSON_GetObjectItem(json, "wifi");
		if (!wifi) 
		{
			wifi = cJSON_CreateObject();
			cJSON_AddItemToObject(json, "wifi", wifi);
		}

		if (cJSON_GetObjectItem(wifi, "SSID"))cJSON_ReplaceItemInObject(wifi, "SSID", cJSON_CreateString(SSID));
		else cJSON_AddStringToObject(wifi, "SSID", SSID);
		if (cJSON_GetObjectItem(wifi, "Key")) cJSON_ReplaceItemInObject(wifi, "Key", cJSON_CreateString(Key));
		else cJSON_AddStringToObject(wifi, "Key", Key);

        fd = open("/setting.json", O_WRONLY | O_TRUNC, 0);
        if (fd >= 0)
        {
            uint8_t *json_str = cJSON_Print(json);
            if (json_str)
            {
                write(fd, json_str, rt_strlen(json_str));
                rt_free(json_str);
            }
            close(fd);
        }
        cJSON_Delete(json);
    }

    return 0;
}

void wifi_usage(void)
{
	printf("wifi - to join a saved wlan\n");
	printf("wifi ap AP_SSID AP_PASSWORD\n");
	printf("wifi join SSID PASSWORD\n");
	printf("wifi help\n");
	printf("wifi cfg SSID PASSWORD\n");
}

int wifi(int argc, char** argv)
{
	if (argc == 1)
	{
		if (wifi_read_cfg() == 0)
		{
			wifi_test(0, ap_ssid.value, ap_key);
		}
		else wifi_usage();

		return -1;
	}

	if (strcmp(argv[1], "help") == 0)
	{
		wifi_usage();
		return 0;
	}

	if (strcmp(argv[1], "ap") == 0)
	{
		if (argc != 4)
			wifi_usage();
		else
			wifi_test(1, argv[2], argv[3]);
			//dhcpd_start("ap");
	}
	else if (strcmp(argv[1], "join") == 0)
	{
		if (argc != 4)
			wifi_usage();
		else
			wifi_test(0, argv[2], argv[3]);
	}
	else if (strcmp(argv[1], "cfg") == 0)
	{
		if (argc != 4)
			wifi_usage();
		else
			wifi_save_cfg(argv[2], argv[3]);
	}
	else
	{
		wifi_usage();
	}

	return 0;
}
MSH_CMD_EXPORT(wifi, wifi command);
