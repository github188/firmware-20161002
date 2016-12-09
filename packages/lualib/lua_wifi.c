#include "gpio.h"
#include "common.h"
#include "lua.h"
#include "lauxlib.h"
#include "lrodefs.h"
#include <string.h>
#include <wifi.h>

#define WIFI_AP_SEC     WICED_SECURITY_WPA2_MIXED_PSK
#define WIFI_AP_CHANNEL 3

static int is_out_of_range(char *s) {
	int num = atoi(s);
	if (num > 255)
		return -1;
	return 0;
}
static int is_valid_ip(const char *ip)  //妫�娴嬫槸鍚︿负鍚堣鐨刬p
{
	int i, j;
	char *tmp = (char *) ip;
	char net[4];
	for (i = 0; i < 4; i++) {	//鍒嗘垚鍥涚墖
		j = 0;
		memset((char*) net, 0, 4);
		while (*tmp != 0) {	//姣忕墖涓嶈秴杩�3涓瓧绗�,浠ULL鍜�.浣滀负缁撳熬
			if (*tmp == '.') {
				tmp++;
				break;
			}
			if (j > 2)	//澶т簬3瀛楃锛岄��鍑�
				return -1;
			net[j] = *tmp;
			tmp++;
			j++;
		}
		int rel = is_out_of_range(net);
		if (rel)
			return rel;
		if (j == 0)
			return -1;
	}
	return 0;
}
/*cfg={}
 cfg.ssid=""
 cfg.pwd=""
 cfg.ip (optional,default:11.11.11.1)
 cfg.netmask(optional,default:255.255.255.0)
 cfg.gateway(optional,default:11.11.11.1)
 cfg.dnsSrv(optional,default:11.11.11.1)
 cfg.retry_interval(optional,default:1000ms)
 wifi.startap(cfg,function(optional))*/
static int lwifi_startap(lua_State* L) {
	int len;
	const char* ssid_p, *pwd, *ip, *netmask, *gateway, *dnssrv;
	wiced_ssid_t ssid;
	wiced_result_t result;
//	if (!lua_istable(L, 1))
//		return luaL_error( L, "table arg needed" );
	//ssid  
	lua_getfield(L, 1, "ssid");
	if (lua_isstring(L, -1))   // deal with the string
			{
		ssid_p = luaL_checklstring(L, 1, &len);
		if (len > 32)
			return luaL_error(L, "ssid:<32");
	} else
		return luaL_error(L, "wrong arg type:ssid");
	//pwd
	lua_getfield(L, 1, "pwd");
	if (!lua_isnil(L, -1)) { /* found? */
		if (lua_isstring(L, -1))   // deal with the string
				{
			const char *pwd = luaL_checklstring(L, -1, &len);
			if (len > 64)
				return luaL_error(L, "pwd:<64");
			if (len > 0)
				;
			else
				pwd = "";
		} else
			return luaL_error(L, "wrong arg type:pwd");
	} else
		return luaL_error(L, "arg: pwd needed");

	bcm_hw_wifi_init(WICED_AP_INTERFACE);
	ssid.length = strlen(ssid_p);
	strncpy(ssid.value, ssid_p, ssid.length);
	wwd_wifi_start_ap(&ssid, WIFI_AP_SEC, pwd, rt_strlen(pwd), WIFI_AP_CHANNEL);

	//ip
	lua_getfield(L, 1, "ip");
	if (!lua_isnil(L, -1)) { /* found? */
		if (lua_isstring(L, -1))   // deal with the ssid string
				{
			const char *ip = luaL_checklstring(L, -1, &len);
			if (len > 16)
				return luaL_error(L, "ip:<16");
			if (is_valid_ip(ip) == -1)
				return luaL_error(L, "ip invalid");
		} else
			return luaL_error(L, "wrong arg type:ip");
	} else {

	}
	//netmask
	lua_getfield(L, 1, "netmask");
	if (!lua_isnil(L, -1)) { /* found? */
		if (lua_isstring(L, -1))   // deal with the ssid string
				{
			const char *netmask = luaL_checklstring(L, -1, &len);
			if (len > 16)
				return luaL_error(L, "netmask:<16");
			if (is_valid_ip(netmask) == -1)
				return luaL_error(L, "netmask invalid");
		} else
			return luaL_error(L, "wrong arg type:netmask");
	} else {
	}
	//gateway
	lua_getfield(L, 1, "gateway");
	if (!lua_isnil(L, -1)) { /* found? */
		if (lua_isstring(L, -1))   // deal with the ssid string
				{
			const char *gateway = luaL_checklstring(L, -1, &len);
			if (len > 16)
				return luaL_error(L, "gateway:<16");
			if (is_valid_ip(gateway) == -1)
				return luaL_error(L, "gateway invalid");
		} else
			return luaL_error(L, "wrong arg type:gateway");
	} else {
	}
	//dnsSrv
	lua_getfield(L, 1, "dnsSrv");
	if (!lua_isnil(L, -1)) { /* found? */
		if (lua_isstring(L, -1))   // deal with the ssid string
				{
			const char *dnsSrv = luaL_checklstring(L, -1, &len);
			if (len > 16)
				return luaL_error(L, "dnsSrv:<16");
			if (is_valid_ip(dnsSrv) == -1)
				return luaL_error(L, "dnsSrv invalid");
		} else
			return luaL_error(L, "wrong arg type:dnsSrv");
	} else {
	}
	//retry_interval
	signed retry_interval = 0;
	lua_getfield(L, 1, "retry_interval");
	if (!lua_isnil(L, -1)) { /* found? */
		retry_interval = luaL_checknumber(L, -1);
		if (retry_interval <= 0)
			return luaL_error(L, "retry_interval:>0ms");
	} else
		retry_interval = 1000;

}

const LUA_REG_TYPE wifi_map[] = { { LSTRKEY( "startap" ),
		LFUNCVAL( lwifi_startap ) },
//	{ LSTRKEY( "startsta" ), LFUNCVAL( lwifi_startsta )},
//	{ LSTRKEY( "smartconfig" ), LFUNCVAL( lwifi_smartconfig )},
//	{ LSTRKEY( "stopsmartconfig" ), LFUNCVAL( lwifi_stopsmartconfig )},
//	{ LSTRKEY( "sethostname" ), LFUNCVAL( lwifi_sethostname )},
//	{ LSTRKEY( "gethostname" ), LFUNCVAL( lwifi_gethostname )},
//	{ LSTRKEY( "scan" ), LFUNCVAL( lwifi_scan ) },
//	{ LSTRKEY( "stop" ), LFUNCVAL( lwifi_stop ) },
//	{ LSTRKEY( "powersave" ), LFUNCVAL( lwifi_powersave ) },
//#if LUA_OPTIMIZE_MEMORY > 0
//	{ LSTRKEY( "sta" ), LROVAL( wifi_station_map ) },
//	{ LSTRKEY( "ap" ), LROVAL( wifi_ap_map ) },
//#endif        
		{ LNILKEY, LNILVAL } };

LUALIB_API int luaopen_wifi(lua_State* L) {
	luaL_register(L, "wifi", wifi_map);
	return 0;
}
