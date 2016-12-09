#include <wifi.h>
#include <finsh.h>
#include <lwip/netif.h>

#define WIFI_AP_SEC     WICED_SECURITY_WPA2_MIXED_PSK
#define WIFI_AP_CHANNEL 3

#define WIFI_DEBUG_ON

#ifdef WIFI_DEBUG_ON
#define WIFI_DEBUG         rt_kprintf("[WIFI] ");rt_kprintf
#else
#define WIFI_DEBUG(...)
#endif /* #ifdef WIFI_DEBUG_ON */

extern int bcm_hw_wifi_init(int interface);

#define NULL_MAC( a )  (((a[0])==0)&& \
                        ((a[1])==0)&& \
                        ((a[2])==0)&& \
                        ((a[3])==0)&& \
                        ((a[4])==0)&& \
                        ((a[5])==0))

#define CMP_MAC( a, b )  (((a[0])==(b[0]))&& \
                          ((a[1])==(b[1]))&& \
                          ((a[2])==(b[2]))&& \
                          ((a[3])==(b[3]))&& \
                          ((a[4])==(b[4]))&& \
                          ((a[5])==(b[5])))

const char *result_to_string(wiced_result_t result)
{
    const char *string[] =
    {
        "SUCCESS",
        "PENDING",
        "TIMEOUT",
        "PARTIAL_RESULTS",
        "INVALID_KEY",
        "DOES_NOT_EXIST",
        "NOT_AUTHENTICATED",
        "NOT_KEYED",
        "IOCTL_FAIL",
        "BUFFER_UNAVAILABLE_TEMPORARY",
        "BUFFER_UNAVAILABLE_PERMANENT",
        "WPS_PBC_OVERLAP",
        "CONNECTION_LOST",
    };

    if (result < WICED_SUCCESS || result > WICED_CONNECTION_LOST)
    {
        return "Unknown";
    }

    return string[result];
}

static int
hexval(int32_t chr)
{
    if (chr >= '0' && chr <= '9')
        return chr - '0';
    if (chr >= 'A' && chr <= 'F')
        return chr - 'A' + 10;
    if (chr >= 'a' && chr <= 'f')
        return chr - 'a' + 10;
    return 0;
}

#define CIRCULAR_RESULT_BUFF_SIZE  (5)
static wiced_scan_result_t     result_buff[CIRCULAR_RESULT_BUFF_SIZE];
static uint16_t                result_buff_write_pos = 0;
//static uint16_t                result_buff_read_pos  = 0;
//static host_semaphore_type_t   num_scan_results_semaphore;
#define MAX_SCAN_RESULTS           (100)
static wiced_mac_t             bssid_list[MAX_SCAN_RESULTS]; /* List of BSSID (AP MAC addresses) that have been scanned */
static void scan_results_handler(wiced_scan_result_t **result_ptr, void *user_data, wiced_scan_status_t status)
{
    rt_kprintf("%s\n", __FUNCTION__);

    if (result_ptr == NULL)
    {
        rt_sem_t sem = (rt_sem_t)user_data;
        rt_sem_release(sem);
        return;
    }

    wiced_scan_result_t *record = (*result_ptr);

    /* Check the list of BSSID values which have already been printed */
    wiced_mac_t *tmp_mac = bssid_list;
    while (!NULL_MAC(tmp_mac->octet))
    {
        if (CMP_MAC(tmp_mac->octet, record->BSSID.octet))
        {
            /* already seen this BSSID */
            WIFI_DEBUG("already seen this BSSID\n");
            return;
        }
        tmp_mac++;
    }

    //dump
    rt_kprintf("SSID:%-32.*s\n", record->SSID.length, record->SSID.value);
    rt_kprintf("BSSID:%02X-%02X-%02X-%02X-%02X-%02X\n",
               record->BSSID.octet[0],
               record->BSSID.octet[1],
               record->BSSID.octet[2],
               record->BSSID.octet[3],
               record->BSSID.octet[4],
               record->BSSID.octet[5]);
    rt_kprintf("RSSI:%d\n", record->signal_strength);
    rt_kprintf("max_data_rate:%dMbps\n", record->max_data_rate / 1000);
    rt_kprintf("security:%08X\n", record->security);
    rt_kprintf("channel:%d\n", record->channel);
    rt_kprintf("band:%d\n", record->band);
    rt_kprintf("next:%08X\n", (uint32_t)record->next);
    rt_kprintf("\n");

    /* New BSSID - add it to the list */
    memcpy(&tmp_mac->octet, record->BSSID.octet, sizeof(wiced_mac_t));

    /* Add the result to the list and set the pointer for the next result */
    result_buff_write_pos++;
    if (result_buff_write_pos >= CIRCULAR_RESULT_BUFF_SIZE)
    {
        result_buff_write_pos = 0;
    }
    *result_ptr = &result_buff[result_buff_write_pos];
//    host_rtos_set_semaphore( &num_scan_results_semaphore, WICED_FALSE );

//    wiced_assert( "Circular result buffer overflow", result_buff_write_pos != result_buff_read_pos );
}

static int wifi_scan(void)
{
    wiced_result_t result = WICED_SUCCESS;
    wiced_scan_result_t *result_ptr = (wiced_scan_result_t *) &result_buff;

    rt_sem_t semaphore = rt_sem_create("easy_scan", 0, RT_IPC_FLAG_FIFO);
    RT_ASSERT(semaphore != RT_NULL);

    memset(bssid_list, 0, sizeof(bssid_list));

    result = wwd_wifi_scan(WICED_SCAN_TYPE_ACTIVE,                // scan_type
                           WICED_BSS_TYPE_ANY,                    // bss_type
                           NULL,                                  // optional_ssid
                           NULL,                                  // optional_mac
                           NULL,                                  // optional_channel_list
                           NULL,                                  // optional_extended_params
                           scan_results_handler,                  // callback
                           (wiced_scan_result_t **) &result_ptr,  // result_ptr
                           (void *)semaphore,
                           WWD_STA_INTERFACE) ;                   // user_data


    WIFI_DEBUG("wiced_wifi_scan result:%d\n", result);
    if (result != WICED_SUCCESS) goto _return;

    result = (wiced_result_t)rt_sem_take(semaphore, RT_TICK_PER_SECOND * 3);
    WIFI_DEBUG("rt_sem_take result:%d\n", result);
    if (result != WICED_SUCCESS) goto _return;

    rt_sem_delete(semaphore); // TODO: timeout safe?

_return:
    return result;
}
MSH_CMD_EXPORT(wifi_scan, wifi scan test);
FINSH_FUNCTION_EXPORT(wifi_scan, wifi scan);

wiced_scan_result_t scan_result1, scan_result2;
static void scan_results_handler_join(wiced_scan_result_t **result_ptr, void *user_data, wiced_scan_status_t status)
{
    if (result_ptr == NULL)
    {
        rt_sem_t sem = (rt_sem_t)user_data;
        rt_sem_release(sem);
        return;
    }

    wiced_scan_result_t *record = (*result_ptr);

    if (!NULL_MAC(record->BSSID.octet))
    {
        memcpy(&scan_result2, record, sizeof(wiced_scan_result_t));

        //dump
        rt_kprintf("SSID:%-32.*s\n", record->SSID.length, record->SSID.value);
        rt_kprintf("BSSID:%02X-%02X-%02X-%02X-%02X-%02X\n",
                   record->BSSID.octet[0],
                   record->BSSID.octet[1],
                   record->BSSID.octet[2],
                   record->BSSID.octet[3],
                   record->BSSID.octet[4],
                   record->BSSID.octet[5]);
        rt_kprintf("RSSI:%d\n", record->signal_strength);
        rt_kprintf("max_data_rate:%dMbps\n", record->max_data_rate / 1000);
        rt_kprintf("security:%08X\n", record->security);
        rt_kprintf("channel:%d\n", record->channel);
        rt_kprintf("band:%d\n", record->band);
        rt_kprintf("next:%08X\n", (uint32_t)record->next);
        rt_kprintf("\n");
    }

    *result_ptr = &scan_result1;
}

int wifi_test(rt_uint32_t b_is_ap_mode, char* wifi_ssid, char* wifi_passwd)
{
    wiced_ssid_t ssid;
    wiced_result_t result;
    extern struct netif *bcm_get_netif(int interface);

    result = wwd_management_init(WICED_COUNTRY_CHINA, RT_NULL);
    if (result != WICED_SUCCESS)
    {
        rt_kprintf("start wlan failed:%d\n", result);
        return -1;
    }

    if (b_is_ap_mode)
    {
        bcm_hw_wifi_init(WICED_AP_INTERFACE);

        ssid.length = strlen(wifi_ssid);
        strcpy(ssid.value, wifi_ssid);
        wwd_wifi_start_ap(&ssid, WIFI_AP_SEC, wifi_passwd,
                          rt_strlen(wifi_passwd), WIFI_AP_CHANNEL);

        netif_set_link_up(bcm_get_netif(WICED_AP_INTERFACE));
    }
    else
    {
        bcm_hw_wifi_init(WICED_STA_INTERFACE);

        //scan
        {
            int ssid_len = strlen(wifi_ssid);
            wiced_scan_result_t *result_ptr = (wiced_scan_result_t *) &result_buff;

            rt_sem_t semaphore = rt_sem_create("easy_scan", 0, RT_IPC_FLAG_FIFO);
            RT_ASSERT(semaphore != RT_NULL);

            if (ssid_len > 32)
            {
                WIFI_DEBUG("easy_join bad arg, ssid_len: %d\n", ssid_len);
                ssid_len = 32;
            }

            memcpy(ssid.value, wifi_ssid, ssid_len);
            ssid.length = ssid_len;

            result = wwd_wifi_scan(WICED_SCAN_TYPE_ACTIVE,                // scan_type
                                   WICED_BSS_TYPE_ANY,                   // bss_type
                                   &ssid,                                // optional_ssid
                                   NULL,                                 // optional_mac
                                   NULL,                                 // optional_channel_list
                                   NULL,                                 // optional_extended_params
                                   scan_results_handler_join,       // callback
                                   (wiced_scan_result_t **) &result_ptr, // result_ptr
                                   (void *)semaphore,
                                   WWD_STA_INTERFACE) ;                  // user_data
            WIFI_DEBUG("wiced_wifi_scan result:%s\n", result_to_string(result));
            if (result != WICED_SUCCESS) goto _return;

            result = (wiced_result_t)rt_sem_take(semaphore, RT_TICK_PER_SECOND * 3);
            if (result != WICED_SUCCESS) goto _return;

            rt_sem_delete(semaphore); // TODO: timeout safe?
        }//scan

        // join
        {
            uint32_t i;
            uint8_t key_length;
            uint8_t *security_key = (uint8_t *)wifi_passwd;

            if (NULL_MAC(scan_result2.BSSID.octet))
            {
                WIFI_DEBUG("AP %s not found!\n", wifi_ssid);
                result = WWD_DOES_NOT_EXIST;
                goto _return;
            }

            if (scan_result2.security == WICED_SECURITY_OPEN)
                key_length = 0;
            else if (scan_result2.security & WEP_ENABLED)
            {
                uint8_t *wep_key_buffer = rt_malloc(64);
                RT_ASSERT(wep_key_buffer != RT_NULL);

                wiced_wep_key_t *temp_wep_key = (wiced_wep_key_t *)wep_key_buffer;

                uint8_t *tmp = (uint8_t *)wifi_passwd;
                key_length = strlen(wifi_passwd) / 2;

                /* Setup WEP key 0 */
                temp_wep_key[0].index = 0;
                temp_wep_key[0].length = key_length;

                rt_kprintf("WEP KEY: ");
                for (i = 0; i < key_length; i++)
                {
                    uint8_t hex;
                    hex = hexval(*tmp++);
                    hex <<= 4;
                    hex |= hexval(*tmp++);
                    rt_kprintf(" %02X", hex);
                    temp_wep_key[0].data[i] = hex;
                }
                rt_kprintf(" %dbyte %dbit!\n", key_length, key_length * 8);

                /* Setup WEP keys 1 to 3 */
                memcpy(wep_key_buffer + 1 * (2 + key_length), temp_wep_key, (2 + key_length));
                memcpy(wep_key_buffer + 2 * (2 + key_length), temp_wep_key, (2 + key_length));
                memcpy(wep_key_buffer + 3 * (2 + key_length), temp_wep_key, (2 + key_length));
                wep_key_buffer[1 * (2 + key_length)] = 1;
                wep_key_buffer[2 * (2 + key_length)] = 2;
                wep_key_buffer[3 * (2 + key_length)] = 3;

                security_key = wep_key_buffer;
            }
            else
                key_length = strlen(wifi_passwd);

            // dump
            if (1)
            {
                wiced_scan_result_t *record = &scan_result2;

                rt_kprintf("\nSCAN & JOIN DUMP\n");
                rt_kprintf("SSID:%-32.*s\n", record->SSID.length, record->SSID.value);
                rt_kprintf("BSSID:%02X-%02X-%02X-%02X-%02X-%02X\n",
                           record->BSSID.octet[0],
                           record->BSSID.octet[1],
                           record->BSSID.octet[2],
                           record->BSSID.octet[3],
                           record->BSSID.octet[4],
                           record->BSSID.octet[5]);
                rt_kprintf("RSSI: %d\n", record->signal_strength);
                rt_kprintf("max_data_rate: %dMbps\n", record->max_data_rate / 1000);
                rt_kprintf("security: %08X\n", record->security);
                rt_kprintf("channel: %d\n", record->channel);
                rt_kprintf("band: %d\n", record->band);
                rt_kprintf("next: 0x%08X\n", (uint32_t)record->next);
                rt_kprintf("\n");
            }

            result = wwd_wifi_join_specific(&scan_result2,                // ap
                                            security_key,                // security_key
                                            key_length,                  // key_length
                                            NULL,
                                            WWD_STA_INTERFACE);                       // semaphore
            WIFI_DEBUG("wiced_wifi_join_specific result:%s\n", result_to_string(result));

            if (result != WICED_SUCCESS) goto _return;

            if (scan_result2.security & WEP_ENABLED)
                rt_free((void *)security_key);

            /* link up for network interface */
            netif_set_link_up(bcm_get_netif(WICED_STA_INTERFACE));
        } // join
    }

_return:
    return 0;
}
FINSH_FUNCTION_EXPORT(wifi_test, wifi test e.g:wifi_test(0, "TPLINK333", "12345678"));

int wifi_ver(void)
{
    char ver[64];

    //wiced_wifi_get_wifi_version(ver, sizeof(ver) - 1);
    rt_kprintf("wifi ver: %s\n", ver);
    return 0;
}
FINSH_FUNCTION_EXPORT(wifi_ver, get wifi version);
