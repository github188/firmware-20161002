#ifndef __WWD_RTOS_H__
#define __WWD_RTOS_H__

#include <rtthread.h>

#define RTOS_LOWEST_PRIORITY             (0)
#define RTOS_HIGHEST_PRIORITY            (10)
#define RTOS_DEFAULT_THREAD_PRIORITY     (1)
#define RTOS_HIGHER_PRIORTIY_THAN(x)     (x < RTOS_HIGHEST_PRIORITY ? x+1 : RTOS_HIGHEST_PRIORITY)
#define RTOS_LOWER_PRIORTIY_THAN(x)      (x > RTOS_LOWEST_PRIORITY ? x-1 : RTOS_LOWEST_PRIORITY)

#define WICED_END_OF_THREAD(thread)
#define WWD_THREAD_STACK_SIZE          1024
#define WICED_STARTUP_DELAY              10

typedef rt_thread_t                      host_thread_type_t;
typedef rt_sem_t                         host_semaphore_type_t;
typedef rt_mq_t                          host_queue_type_t;

#define RTOS_USE_DYNAMIC_THREAD_STACK

#endif
