#ifndef _MULTI_SENSOR_H_
#define _MULTI_SENSOR_H_

// #define FH_USING_MULTI_SENSOR
#ifdef FH_USING_MULTI_SENSOR 
#define MULTI_SENSOR       1
#else
#define MULTI_SENSOR       0
#endif

#define FH_ISP_PARA_SIZE (0x000007eC)

#define SENSOR_TYPE_NONE      (0)
#define SENSOR_TYPE_imx138    (1 << 0)
#define SENSOR_TYPE_ov9712    (1 << 1)
#define SENSOR_TYPE_ov9732    (1 << 2)
#define SENSOR_TYPE_ar0130    (1 << 3)
#define SENSOR_TYPE_h42       (1 << 4)
#define SENSOR_TYPE_gc1024    (1 << 5)
//#define SENSOR_TYPE_OV9750    (1 << 6)
//#define SENSOR_TYPE_AR0140    (1 << 7)
#define SENSOR_TYPE_FIXED      (1<<31)

typedef struct 
{
    int SensorModel;
    char *sensorName;
    struct isp_sensor_if* (*sensorCreate)(void);
    struct isp_sensor_if* sensor;
    char *param;
    
}SensorCtrl_t;

#define SENSORCTRL(n) { \
	SENSOR_TYPE_##n,	\
	#n,					\
	Sensor_Create_##n,	\
	0,					\
    isp_param_buff_##n,	}


int Sensor_Probe(void);
int Sensor_Getmodel(void);
SensorCtrl_t * isp_sensor_Create(int sensor_model);

#endif

