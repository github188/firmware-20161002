#ifndef __LUAOPEN_H__
#define __LUAOPEN_H__

LUALIB_API int luaopen_gpio(lua_State *L);
LUALIB_API int luaopen_adc(lua_State *L);
LUALIB_API int luaopen_file(lua_State *L);
LUALIB_API int luaopen_dir(lua_State *L);
LUALIB_API int luaopen_net(lua_State *L);
LUALIB_API int luaopen_spi(lua_State *L);
LUALIB_API int luaopen_iic(lua_State* L);
LUALIB_API int luaopen_pwm(lua_State* L);
LUALIB_API int luaopen_wifi(lua_State* L);


#endif
