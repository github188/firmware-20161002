#include <rtdef.h>
#include <rtthread.h>
#include <rtdevice.h>
#include <dfs_posix.h>

#include "pwm.h"

// #define VPWM_TEST
#ifdef VPWM_TEST

static struct rt_device* vpwmDevice;
static int vpwm_fd;

int vpwm_func(int percent, int id)
{
    int ret    = 0;
    int dev_id = 0;
    int duty, period;

    struct pwm_device pwm;
    rt_kprintf("pwm func start\n");
    pwm.id         = 3;
    pwm.period_ns  = 200000;
    pwm.counter_ns = percent * pwm.period_ns / 100;
    rt_device_control(vpwmDevice, SET_PWM_DUTY_CYCLE, &pwm);
    rt_thread_delay(200);

    pwm.id         = 5;
    pwm.period_ns  = 2000000;
    pwm.counter_ns = percent * pwm.period_ns / 100;
    rt_device_control(vpwmDevice, SET_PWM_DUTY_CYCLE, &pwm);
    rt_thread_delay(200);

    pwm.id         = 3;
    pwm.period_ns  = 500000;
    pwm.counter_ns = percent * pwm.period_ns / 100;
    rt_device_control(vpwmDevice, SET_PWM_DUTY_CYCLE, &pwm);
    rt_thread_delay(200);
    pwm.id         = 4;
    pwm.period_ns  = 500000;
    pwm.counter_ns = percent * pwm.period_ns / 100;
    rt_device_control(vpwmDevice, SET_PWM_DUTY_CYCLE, &pwm);
    rt_thread_delay(200);

    /******************  teset  close&open device ******************/
    // close device
    vpwm_fd = rt_device_close(vpwmDevice);
    if (vpwm_fd == -1)
    {
        rt_kprintf("rt_device_close pwm failed\n");
    }
    rt_kprintf("closed pwm\n");

    // open device
    vpwm_fd = rt_device_open(vpwmDevice, O_RDWR);
    if (vpwm_fd == -1)
    {
        rt_kprintf("rt_device_open pwm failed\n");
    }
    rt_kprintf("reopend pwm\n");
    pwm.id         = 3;
    pwm.period_ns  = 100000;
    pwm.counter_ns = percent * pwm.period_ns / 100;
    rt_device_control(vpwmDevice, SET_PWM_DUTY_CYCLE, &pwm);
    rt_thread_delay(200);
    pwm.id         = 5;
    pwm.period_ns  = 100000;
    pwm.counter_ns = percent * pwm.period_ns / 100;
    rt_device_control(vpwmDevice, SET_PWM_DUTY_CYCLE, &pwm);
    rt_thread_delay(200);

    // change period to 300000 ns
    pwm.id         = 3;
    pwm.period_ns  = 300000;
    pwm.counter_ns = percent * pwm.period_ns / 100;
    rt_device_control(vpwmDevice, SET_PWM_DUTY_CYCLE, &pwm);
    rt_thread_delay(200);

    return ret;
}

void vpwm_demo_main(void* param)
{
    vpwmDevice = rt_device_find("pwm");

    vpwm_fd = rt_device_open(vpwmDevice, O_RDWR);
    if (vpwm_fd == -1)
    {
        rt_kprintf("rt_device_open pwm failed\n");

        return;
    }
    vpwm_func(70, 3);
    return;
}

int vpwm_demo_init()
{
    int ret = 0;

    rt_thread_t threadPwm;

    threadPwm =
        rt_thread_create("vpwm", vpwm_demo_main, RT_NULL, 10 * 1024, 8, 20);

    if (threadPwm != RT_NULL) rt_thread_startup(threadPwm);
}

#endif