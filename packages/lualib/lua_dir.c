// Module for interfacing with file system

#include "lauxlib.h"
#include "lua.h"
#include "lrodefs.h"
#include "dfs_posix.h"
#include "common.h"

static volatile DIR *dir;		

/*dir.chdir("path")*/
static int dir_chdir(lua_State* L)
{
	const char *path=luaL_checkstring(L, 1);
	LUA_LIB_DEBUG("chdir path=%s\n", path);
	luaL_argcheck(L, chdir(path) == 0, 1,"chdir failed!\n");
	return 0;
}

/*dir.mkdir("name")*/
static int dir_mkdir(lua_State* L)
{
	const char *name=luaL_checkstring(L, 1);
	LUA_LIB_DEBUG("mkdir name=%s\n", name);
	mkdir(name, RT_NULL);
	return 0;
}

/*dir.rmdir("name")*/
static int dir_rmdir(lua_State *L)
{
	const char *name=luaL_checkstring(L, 1);
	LUA_LIB_DEBUG("rmdir name=%s\n", name);
	if (rmdir(name)==0)
		luaL_error(L, "rmdir failed!\n");
	return 0;
}

/*dir.getcwd*/
static int dir_getcwd(lua_State* L)
{
	char cwd[100]={0};
	getcwd(cwd, 100);
	lua_pushstring(L, cwd);
	LUA_LIB_DEBUG("getcwd =%s\n",cwd);
	return 1;
}

/*dir.opendir("directory")*/
static int dir_opendir(lua_State* L)
{
	const char *name=luaL_checkstring(L, 1);
	LUA_LIB_DEBUG("opendir name=%s\n", name);
	dir = opendir(name);
	if(dir == RT_NULL)
		luaL_error(L, "opendir failed!");
	return 0;
}

static const LUA_REG_TYPE dir_map[] = {
	{ LSTRKEY( "chdir"),	  LFUNCVAL( dir_chdir)},
	{ LSTRKEY( "mkdir"),	  LFUNCVAL( dir_mkdir)},
	{ LSTRKEY( "rmdir"),	  LFUNCVAL( dir_rmdir)},
	{ LSTRKEY( "getcwd"),	  LFUNCVAL( dir_getcwd)},
//	{ LSTRKEY( "opendir"),	  LFUNCVAL( dir_opendir)},
	{ LNILKEY, LNILVAL }
};

LUALIB_API int luaopen_dir(lua_State *L){
	luaL_register(L, "dir", dir_map);
	return 0;
}
