/*
 * multi_sensor.c
 *
 *  Created on: 2015
 *      Author: duoabao
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <rtthread.h>

#include "multi_sensor.h"
#include "isp_sensor_if.h"

static int sensor_model = SENSOR_TYPE_NONE;

#ifdef FH_USING_MULTI_SENSOR
#define ISP_PARAM_BUFF(n)  isp_param_buff_##n
#define SENSOR_CREATE(n)  Sensor_Create_##n

#ifdef RT_USING_AR0130
extern char ISP_PARAM_BUFF(ar0130)[FH_ISP_PARA_SIZE];
extern struct isp_sensor_if* SENSOR_CREATE(ar0130)();
#endif

#ifdef RT_USING_BF3016
extern char ISP_PARAM_BUFF(bf3016)[FH_ISP_PARA_SIZE];
extern struct isp_sensor_if* SENSOR_CREATE(bf3016)();
#endif

#ifdef RT_USING_GC1024
extern char ISP_PARAM_BUFF(gc1024)[FH_ISP_PARA_SIZE];
extern struct isp_sensor_if* SENSOR_CREATE(gc1024)();
#endif

#ifdef RT_USING_H42
extern char ISP_PARAM_BUFF(h42)[FH_ISP_PARA_SIZE];
extern struct isp_sensor_if* SENSOR_CREATE(h42)();
#endif

#ifdef RT_USING_IMX138
extern char ISP_PARAM_BUFF(imx138)[FH_ISP_PARA_SIZE];
extern struct isp_sensor_if* SENSOR_CREATE(imx138)();
#endif

#ifdef RT_USING_OV9712
extern char ISP_PARAM_BUFF(ov9712)[FH_ISP_PARA_SIZE];
extern struct isp_sensor_if* SENSOR_CREATE(ov9712)();
#endif
    
#ifdef RT_USING_OV9732
extern char ISP_PARAM_BUFF(ov9732)[FH_ISP_PARA_SIZE];
extern struct isp_sensor_if* SENSOR_CREATE(ov9732)();
#endif

static SensorCtrl_t SensorCtrls[] = {
    #ifdef RT_USING_AR0130
        SENSORCTRL(ar0130),
    #endif

    #ifdef RT_USING_BF3016
        SENSORCTRL(bf0130),
    #endif

    #ifdef RT_USING_GC1024
        SENSORCTRL(gc1024),
    #endif

    #ifdef RT_USING_H42
        SENSORCTRL(h42),
    #endif

    #ifdef RT_USING_IMX138
        SENSORCTRL(imx138),
    #endif

    #ifdef RT_USING_OV9712
        SENSORCTRL(ov9712),
    #endif

    #ifdef RT_USING_OV9732
        SENSORCTRL(ov9732),
    #endif

    {
        SENSOR_TYPE_NONE,
        NULL,
        NULL,
        NULL,
        NULL,
    }
};
#else

extern char isp_param_buff[FH_ISP_PARA_SIZE];
extern struct isp_sensor_if* Sensor_Create();
static SensorCtrl_t SensorCtrls[] = {
    {
        SENSOR_TYPE_FIXED,
        "fixed",
        Sensor_Create,
        NULL,
        isp_param_buff,
    }
};

#endif


int Sensor_Probe(void)
{
    int i;
    struct isp_sensor_if* sensor;
    printf("sensor probe...\n");
    for(i = 0; i < sizeof(SensorCtrls)/sizeof(SensorCtrl_t); i ++)
    {
        if(SensorCtrls[i].SensorModel != SENSOR_TYPE_NONE)
        {
            if(SensorCtrls[i].sensorCreate == NULL)
            {
                printf("sensorCreate callback not found\n");
            }
            sensor = SensorCtrls[i].sensorCreate();
            if (sensor != 0)
            {
                printf("sensor: %s (%s)\n", SensorCtrls[i].sensorName, sensor->name);
                SensorCtrls[i].sensor = sensor;
                sensor_model = SensorCtrls[i].SensorModel;
                return sensor_model;
            }
        }
    }

    return -1;
}

int Sensor_Getmodel(void)
{
    return sensor_model;
}

SensorCtrl_t * isp_sensor_Create(int sensor_model)
{
    int i;

    if(sensor_model == SENSOR_TYPE_NONE)
        return NULL;
    
    for(i = 0; i < sizeof(SensorCtrls) / sizeof(SensorCtrl_t); i ++)
    {
        if(sensor_model == SensorCtrls[i].SensorModel)
        {
            return &SensorCtrls[i];
        }
    }

    return NULL;
}

