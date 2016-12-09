#include <rtdef.h>
#include <rtthread.h>
#include <rtdevice.h>
#include <dfs_posix.h>

#include "pwm.h"

static struct rt_device* pwmDevice;
static	int pwm_fd;

int pwm_func(int percent)
{
	int ret = 0;
//	int dev_id = 0;
//	int duty, period;

	struct pwm_device pwm;

	pwm.id = 0;

	pwm.period_ns = 1000000;

	pwm.counter_ns = percent * pwm.period_ns / 100;

	rt_device_control(pwmDevice, SET_PWM_DUTY_CYCLE, &pwm);	
	rt_kprintf("set duty ratio to %d%\n", percent);
	return ret;
}

void pwm_demo_main(void* param)
{
	rt_kprintf("pwm demo start:\n");
	pwmDevice = rt_device_find("pwm");

	pwm_fd = rt_device_open(pwmDevice, O_RDWR);
	if (pwm_fd == -1)
	{
		rt_kprintf("rt_device_open pwm failed\n");

		return;
	}

	pwm_func(50);

	return;
}

int pwm_demo_init()
{
//	int ret = 0;

	rt_thread_t threadPwm;

	threadPwm = rt_thread_create("pwm",pwm_demo_main, RT_NULL ,10 * 1024, 8, 20);

	if (threadPwm != RT_NULL)
		rt_thread_startup(threadPwm);
	return 0;
}

#ifdef RT_USING_FINSH
#include <finsh.h>
FINSH_FUNCTION_EXPORT(pwm_func,pwm_func(int percent));
#endif
