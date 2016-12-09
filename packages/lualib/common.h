#ifndef __COMMON_H__
#define __COMMON_H__
#include "rtdef.h"

#define LUA_LIB_DEBUG_ON

#ifdef LUA_LIB_DEBUG_ON
#define LUA_LIB_DEBUG rt_kprintf
#else
#define LUA_LIB_DEBUG
#endif


/*gpio*/
#define PLATFORM_GPIO_INPUT GPIO_DIR_INPUT
#define PLATFORM_GPIO_OUTPUT GPIO_DIR_OUTPUT
#define PLATFORM_GPIO_HIGH 1
#define PLATFORM_GPIO_LOW 0

#define platform_gpio_exists(n)  (n==5 || n==6 || n==8 || n==9)
int platform_gpio_mode(unsigned int pin, unsigned int mode);
int platform_gpio_read(unsigned int pin);
int platform_gpio_write( unsigned int pin, int level);


/*adc*/
#define platform_adc_exists(id) 	\
do{	\
	if(id >= 2 ){	\
		LUA_LIB_DEBUG("adc chn out of range\n");		\
		return 0;	\
	}	\
}while(0)


/*net*/
#define CAM_TCP 0x10
#define CAM_UDP 0x20

enum camconn_type {
    CAMCONN_INVALID    = 0,
    /* CAMCONN_TCP Group */
    CAMCONN_TCP        = 0x10,
    /* CAMCONN_UDP Group */
    CAMCONN_UDP        = 0x20,
};
enum camconn_state {
    CAMCONN_NONE,
    CAMCONN_WAIT,
    CAMCONN_LISTEN,
    CAMCONN_CONNECT,
    CAMCONN_WRITE,
    CAMCONN_READ,
    CAMCONN_CLOSE,
};
typedef struct  {	//tcp状态数据，用于与lwip对接
	int remote_port;
	int local_port;
	rt_uint8_t local_ip[4];
	rt_uint8_t remote_ip[4];
} cam_tcp;

typedef struct  {	//udp状态数据，用于与lwip对接
	int remote_port;
	int local_port;
	rt_uint8_t local_ip[4];
	rt_uint8_t remote_ip[4];
} cam_udp;



struct camconn {				//面向lwip
	/** type of the espconn (TCP, UDP) */
	enum camconn_type type;
	/** current state of the espconn */
	enum camconn_state state;
	union {
		cam_tcp *tcp;
		cam_udp *udp;
	} proto;
	union {
		struct tcp_pcb *tcp_pcb;
		struct udp_pcb *udp_pcb;
	}pcb;
	void *nud;
};
typedef struct lnet_userdata			//面向lua
{
  struct camconn *cam_conn_p;
  int self_ref;
  int cb_connect_ref;		//连接回调(不适于tcp服务端)
  int cb_reconnect_ref;		//重新连接回调
  int cb_disconnect_ref;	//断开连接回调
  int cb_receive_ref;		//接收数据回调
  int cb_send_ref;		//发送完整回调
  int cb_dns_found_ref;		//dns解析完成回调
}lnet_userdata;		//每个socket对应的对象数据类型

#define IP2STR(ipaddr) ip4_addr1_16(ipaddr), \
    ip4_addr2_16(ipaddr), \
    ip4_addr3_16(ipaddr), \
    ip4_addr4_16(ipaddr)


#define IPSTR "%d.%d.%d.%d"
#endif
