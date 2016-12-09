#include <rtthread.h>
#include <rtdevice.h>

#include "lauxlib.h"
#include "lua.h"
#include "lrodefs.h"
#include "common.h"
#include <string.h>


/*iic.setup()*/
static int iic_setup(lua_State* L)
{
	/* nothing */
}

/*iic.read(addr, size)*/
static int iic_read(lua_State* L)
{
	unsigned long addr = luaL_checklong(L, 1);
	unsigned long size = luaL_checklong(L, 2);
	char *buffer = (char*)rt_malloc(size);
	if (buffer == NULL)
		luaL_error(L, "malloc for iic read failed \n");

	rt_device_t iic_dev = rt_device_find("i2c0");
	if (iic_dev == NULL)
		luaL_error(L, "no found iic bus\n");

	rt_device_open(iic_dev, 0);
	int rel = rt_device_read(iic_dev, addr, buffer, size-1);
	rt_device_close(iic_dev);
	lua_pushstring(L, buffer);
	rt_free(buffer);
	return 1;
}

/*iic.write(addr, datastring)*/
static int iic_write(lua_State* L)
{
	int addr = luaL_checkinteger(L, 1);
	int size;
	char *buffer = (char *)luaL_checklstring(L, 2, &size);
	rt_device_t iic_dev = rt_device_find("i2c0");
	if (iic_dev == NULL)
		luaL_error(L, "no found iic bus\n");

	rt_device_open(iic_dev, 0);
	int rel = rt_device_write(iic_dev, addr, buffer, size);
	rt_device_close(iic_dev);

	return 1;
}

static const LUA_REG_TYPE iic_map[]={
	{ LSTRKEY( "read" ),  LFUNCVAL( iic_read ) },
	{ LSTRKEY( "write" ),  LFUNCVAL( iic_write ) },
	{ LNILKEY, LNILVAL }
};

LUALIB_API int luaopen_iic(lua_State* L)
{
	luaL_register(L, "iic", iic_map);
	return 0;
}
