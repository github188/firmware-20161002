#include <rtdef.h>
#include "gpio.h"
#include "common.h"
#include "lua.h"
#include "lauxlib.h"
#include "lrodefs.h"
static int callback;

/*key.register(function()...end)*/
static int register_inteerupt(lua_State* L)
{
	if (lua_type(L, 1) != LUA_TFUNCTION && lua_type(L, 1) != LUA_TLIGHTFUNCTION)
		luaL_error(L, "no callbackfunction");
	lua_pushvalue(L, 1);
	callback = luaL_ref(L, LUA_REGISTRYINDEX);
}
