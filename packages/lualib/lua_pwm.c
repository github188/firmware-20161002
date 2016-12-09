#include <rtthread.h>
#include <rtdevice.h>

#include "gpio.h"
#include "common.h"
#include "lua.h"
#include "lauxlib.h"
#include "lrodefs.h"
#include "pwm.h"

static rt_device_t pwm_dev;
static int pwm(int id, int zkb)
{
	struct pwm_device pwm;
	pwm.id = id;
	pwm.period_ns = 1000000;
	pwm.counter_ns = zkb * pwm.period_ns / 100;

	int ret = rt_device_control(pwm_dev, SET_PWM_DUTY_CYCLE, &pwm);	
	return ret;
}

/*pwm.set(id, zkb) 0<zkb<100*/
static int lua_pwm(lua_State* L)
{
	int id = luaL_checkinteger(L, 1);
	int zkb = luaL_checkinteger(L, 2);

	luaL_argcheck(L, id >= 0 && id <= 2, 1, "bad pwm device id");
	luaL_argcheck(L, zkb <= 100 && zkb >= 0, 2, "dutyfactor out of range");

	pwm_dev = rt_device_find("pwm");
	int pwm_fd = rt_device_open(pwm_dev, RT_DEVICE_OFLAG_RDWR);
	if (pwm_fd == -1)
	{
		rt_kprintf("rt_device_open pwm failed\n");
		return;
	}

	pwm(id, zkb);
	rt_device_close(pwm_dev);
	return;
}

static const LUA_REG_TYPE pwm_map[] = {
  { LSTRKEY( "set" ),       LFUNCVAL( lua_pwm ) },
  { LNILKEY, LNILVAL }
};

LUALIB_API int luaopen_pwm(lua_State* L)
{
	luaL_register(L, "pwm", pwm_map);

	fh_iomux_pin_switch(50, 1);
	fh_iomux_pin_switch(51, 1);

	return 0;
}
