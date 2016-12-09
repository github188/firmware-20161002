#include <rtthread.h>
#include <finsh.h>

int lua(int argc, char** argv)
{
	return lua_main(argc, argv);
}
MSH_CMD_EXPORT(lua, lua script interpreter);
