// Module for interfacing with adc

#include <rtthread.h>
#include <rtdevice.h>

#include "lauxlib.h"
#include "lua.h"
#include "lrodefs.h"
#include "common.h"
#include "drivers/sadc.h"

/*adc.open()*/
static int adc_open(lua_State *L){
	rt_device_t adc_dev;
	adc_dev = rt_device_find("sadc");
	if (adc_dev == RT_NULL){
		LUA_LIB_DEBUG("no found sadc device,exit...\n");
		return 0;
	}
	if (rt_device_open(adc_dev, 0)){
		LUA_LIB_DEBUG("adc open failed!\n");
	}else{
		LUA_LIB_DEBUG("adc open succeed!\n");
	}
	return 0;
}

/*adc.read(int chn) chn锛氶�氶亾 */
static int adc_read(lua_State *L){
	rt_uint32_t chn = luaL_checkinteger(L, 1);
	rt_device_t adc_dev;
	SADC_INFO info;
	info.channel = chn;
	info.sadc_data = 0;
	
	platform_adc_exists(chn);
	adc_dev = rt_device_find("sadc");
	if (adc_dev == RT_NULL){
		LUA_LIB_DEBUG("no found sadc device,exit...\n");
		return 0;
	}
	rt_device_control(adc_dev, SADC_CMD_READ_RAW_DATA, &info);
	lua_pushinteger(L, info.sadc_data);
	return 1;
}

/*adc.close()*/
static int adc_close(lua_State *L){
	rt_device_t adc_dev;

	adc_dev = rt_device_find("sadc");
	if (adc_dev == RT_NULL){
		LUA_LIB_DEBUG("no found sadc device,exit...\n");
		return 0;
	}
	rt_device_close(adc_dev);
	return 0;
}

static const LUA_REG_TYPE adc_map[] = {
  { LSTRKEY( "open" ),      	LFUNCVAL( adc_open ) },
  { LSTRKEY( "read" ),		LFUNCVAL( adc_read ) },
  { LSTRKEY( "close" ), 	LFUNCVAL( adc_close ) },
  /*
  { LSTRKEY( "INIT_ADC" ),  LNUMVAL( 0x00 ) },
  { LSTRKEY( "INIT_VDD33" ),LNUMVAL( 0xff ) },
  */
  { LNILKEY, LNILVAL }
};

LUALIB_API int luaopen_adc(lua_State *L){
	luaL_register(L, "adc", adc_map);
	return 0;
}
