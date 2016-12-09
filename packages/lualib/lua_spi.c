// Module for interfacing with the SPI interface

#include "lauxlib.h"
#include "lrodefs.h"
#include "common.h"
#include "drivers/spi.h"

/*spi*/
#define SPI_MASTER 	RT_SPI_MASTER
#define	SPI_SLAVE 	RT_SPI_SLAVE
#define SPI_CPHA_LOW 	0
#define SPI_CPHA_HIGH	RT_SPI_CPHA
#define SPI_CPOL_LOW	0
#define SPI_CPOL_HIGH	RT_SPI_CPOL

// Lua: = spi.setup(mode, cpol, cpha, databits, clk )
static int spi_setup( lua_State *L )
{
	int mode        = luaL_checkinteger( L, 1 );
	int cpol        = luaL_checkinteger( L, 2 );
	int cpha        = luaL_checkinteger( L, 3 );
	int databits    = luaL_checkinteger( L, 4 );
	rt_uint32_t clock   = luaL_checkinteger( L, 5 );
	struct rt_spi_configuration config;

	if (mode != SPI_SLAVE && mode != SPI_MASTER) {
		return luaL_error( L, "wrong arg type" );
	}

	if (cpol != SPI_CPOL_LOW && cpol != SPI_CPOL_HIGH) {
		return luaL_error( L, "wrong arg type" );
	}

	if (cpha != SPI_CPHA_LOW && cpha != SPI_CPHA_HIGH) {
		return luaL_error( L, "wrong arg type" );
	}

	if (databits < 0 || databits > 16) {
		return luaL_error( L, "bitwidth out of range" );
	}
	config.mode = mode | cpol | cpha;
	config.data_width = databits;
	config.max_hz = clock;
	rt_device_t spi_dev = rt_device_find("ssi0_1");
	if (spi_dev == NULL )
		luaL_error(L, "can't found spi device\n");
	int rel = rt_spi_configure((struct rt_spi_device*)spi_dev,
			&config);
	lua_pushinteger( L, rel );
	return 1;
}

// Lua: wrote  = spi.send( data1)
static int spi_send( lua_State *L )
{
	int len;
	const char *buffer = (char*)luaL_checklstring(L, 1, &len);
	rt_device_t spi_dev = rt_device_find("ssi0_1");
	if (spi_dev == NULL )
		luaL_error(L, "can't found spi device\n");
	LUA_LIB_DEBUG("spi write data=%s\n", buffer);
	rt_device_open(spi_dev, 0);

	int length = rt_device_write(spi_dev, 0, buffer, len);
	LUA_LIB_DEBUG("writed length =%d\n", length);
	if (length != len)
		luaL_error(L, "write failed\n");
	rt_device_close(spi_dev);
	return 0;
}
/*spi.recv(size)*/ 
static int spi_recv(lua_State* L)
{
	int size = luaL_checkinteger(L, 1);
	char *buffer = rt_malloc(size+1);
	if (buffer == NULL )
		luaL_error(L, "mallo for spi recv failed!\n");
	rt_device_t spi_dev = rt_device_find("ssi0_1");
	if (spi_dev == NULL )
		luaL_error(L, "can't found spi device\n");

	rt_device_open(spi_dev, 0);
	rt_device_read(spi_dev, 0, buffer, size);
	rt_device_close(spi_dev);

	lua_pushstring(L, buffer);
	rt_free(buffer);
}


// Module function map
static const LUA_REG_TYPE spi_map[] = {
  { LSTRKEY( "setup" ),       LFUNCVAL( spi_setup ) },
  { LSTRKEY( "send" ),        LFUNCVAL( spi_send ) },
  { LSTRKEY( "recv" ),        LFUNCVAL( spi_recv ) },
  /*
  { LSTRKEY( "MASTER" ),      LNUMVAL( SPI_MASTER ) },
  { LSTRKEY( "SLAVE" ),       LNUMVAL( SPI_SLAVE) },
  { LSTRKEY( "CPHA_LOW" ),    LNUMVAL( SPI_CPHA_LOW) },
  { LSTRKEY( "CPHA_HIGH" ),   LNUMVAL( SPI_CPHA_HIGH) },
  { LSTRKEY( "CPOL_LOW" ),    LNUMVAL( SPI_CPOL_LOW) },
  { LSTRKEY( "CPOL_HIGH" ),   LNUMVAL( SPI_CPOL_HIGH) },
  */
  { LNILKEY, LNILVAL }
};

LUALIB_API int luaopen_spi(lua_State* L)
{
	luaL_register(L, "spi", spi_map);
	return 0;
}
