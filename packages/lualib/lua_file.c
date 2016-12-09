// Module for interfacing with file system

#include "lauxlib.h"
#include "lua.h"
#include "lrodefs.h"
#include "dfs_posix.h"
#include "common.h"

#define LUA_READ_BUFFER_SIZE 1000
static volatile int file_fd = 0;
static char read_buffer[LUA_READ_BUFFER_SIZE];

static int mode2flag(char *mode) {
	int flag;
	/*閸忓牆鍨介弬顓狀儑娴滃奔缍呴弰顖氭儊閺堝鏅ラ敍宀�鍔ч崥搴㈢壌閹诡喚顑囨稉锟芥担宥堫啎缂冾喗鐖ｈ箛锟�*/
	switch (*(mode + 1)) {

	/*閸欘亝婀佹稉锟芥担宥呯摟缁楋拷*/
	case RT_NULL: {
		switch (*mode) {
		case 'r':
			flag |= O_RDONLY;
			break;
		case 'w':
			flag |= O_WRONLY | O_CREAT | O_TRUNC;
			break;
		case 'a':
			flag |= O_WRONLY | O_CREAT | O_APPEND;
			break;
		default:
			goto error;
		}
		break;
	} /*case RT_BULL*/

		/*娑撱倓缍呯�涙顑�*/
	case '+': {
		switch (*mode) {
		case 'r':
			flag |= O_RDWR;
			break;
		case 'w':
			flag |= O_RDWR | O_CREAT | O_TRUNC;
			break;
		case 'a':
			flag |= O_RDWR | O_CREAT | O_APPEND;
			break;
		default:
			goto error;
		}
		break;
	}
		/*閸戞椽鏁婇惄瀛樺复鏉╂柨娲�*/
		defalut: goto error;
	}
	return flag;
	error: return -1;
}

// Lua: open(filename, mode)
static int file_open(lua_State* L) {
	size_t len;
	if (file_fd != 0) {
		close(file_fd);
		file_fd = 0;
	}

	const char *fname = luaL_checklstring(L, 1, &len);
	const char *mode = luaL_optstring(L, 2, "r");
	int flag = mode2flag((char*) mode);
	LUA_LIB_DEBUG("mode=%s,flag=%ud,filename=%s\n", mode, flag, fname);

	luaL_argcheck(L, flag != -1, 1, "Invalid mode");
	file_fd = open(fname, flag, RT_NULL);
	if (file_fd < 0) {
		file_fd = 0;
		luaL_error(L, "no such file or directory");
		lua_pushnil(L);
	} else {
		lua_pushboolean(L, 1);
	}
	return 1;
}

/* Lua: close()*/
static int file_close(lua_State* L) {
	if (file_fd != 0) {
		close(file_fd);
		file_fd = 0;
	}
	return 0;
}

/* Lua: read(int size)*/
static int file_read(lua_State* L) {
	int size, len;
	if (file_fd == 0) {
		luaL_error(L, "please open a file first!\n");
		return 0;
	}
	size = luaL_checkinteger(L, 1);
	luaL_argcheck(L, LUA_READ_BUFFER_SIZE>size, 1, "not enough buff space\n");
	len = read(file_fd, read_buffer, size);
	LUA_LIB_DEBUG("expect size=%d read len=%d string=%s\n", size, len,
			read_buffer);
	lua_pushstring(L, read_buffer);
	return 1;
}

// Lua: write("string")
static int file_write(lua_State* L) {
	if (file_fd == 0)
		return luaL_error(L, "open a file first");
	size_t l, rl;
	const char *s = luaL_checklstring(L, 1, &l);
	LUA_LIB_DEBUG("write string=%s\n", s);
	rl = write(file_fd, s, l);
	if (rl == l)
		lua_pushboolean(L, 1);
	else
		lua_pushnil(L);
	return 1;
}

// Lua: list()
extern void ls(const char *pathname);
static int file_list(lua_State* L) {
	char *s = (char*) luaL_optstring(L, 1, "/");
	ls(s);
	return 0;
}

/*file.seek( ("set" or "cur" or "end") , offset)*/
static int file_seek(lua_State *L) {
	if (file_fd == 0)
		return luaL_error(L, "open a file first");
	static const int mode[] = { SEEK_SET, SEEK_CUR, SEEK_END };
	static const char * const modenames[] = { "set", "cur", "end", NULL };
	if (file_fd == 0)
		return luaL_error(L, "open a file first");
	int op = luaL_checkoption(L, 1, "cur", modenames);
	long offset = luaL_optlong(L, 2, 0);
	LUA_LIB_DEBUG("seek offset=%d,mode=%s\n", offset, modenames[op]);
	offset = lseek(file_fd, offset, mode[op]);
	if (offset < 0)
		lua_pushnil(L); /* error */
	else
		lua_pushinteger(L, offset);
	LUA_LIB_DEBUG("now the offst is %d\n", offset);
	return 1;
}

// Lua: remove(filename)
extern void rm(const char *filename);
static int file_remove(lua_State* L) {
	size_t len;
	const char *fname = luaL_checklstring(L, 1, &len);
	rm(fname);
	LUA_LIB_DEBUG("rm filename=%s\n", fname);
	return 0;
}

static int file_rename(lua_State* L) {
	size_t len;
	if (file_fd != 0) {
		close(file_fd);
		file_fd = 0;
	}

	const char *oldname = luaL_checklstring(L, 1, &len);
	const char *newname = luaL_checklstring(L, 2, &len);
	rename(oldname, newname);
	return 1;
}

static int file_flush(lua_State* L) {
	if (file_fd == 0)
		return luaL_error(L, "open a file first");
	if (fsync(file_fd))
		luaL_error(L, "flush error\n");
	return 0;
}

// Module function map
static const LUA_REG_TYPE file_map[] = { { LSTRKEY( "list" ),
		LFUNCVAL( file_list ) }, { LSTRKEY( "open" ), LFUNCVAL( file_open ) }, {
		LSTRKEY( "close" ), LFUNCVAL( file_close ) }, { LSTRKEY( "write" ),
		LFUNCVAL( file_write ) }, { LSTRKEY( "read" ), LFUNCVAL( file_read ) },
		{ LSTRKEY( "remove" ), LFUNCVAL( file_remove ) }, { LSTRKEY( "seek" ),
				LFUNCVAL( file_seek ) }, { LSTRKEY( "rename" ),
				LFUNCVAL( file_rename ) }, { LSTRKEY( "flush" ),
				LFUNCVAL( file_flush ) }, { LNILKEY, LNILVAL } };

LUALIB_API int luaopen_file(lua_State *L) {
	luaL_register(L, "file", file_map);
	return 0;
}
