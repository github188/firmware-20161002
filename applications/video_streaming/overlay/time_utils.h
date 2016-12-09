#ifndef _OASL_BASEAPI_H_
#define _OASL_BASEAPI_H_

typedef struct
{
    int  year;      /*!< year 1970 ~2039 现加了1900*/
    int  month;     /*!< month (1-12) 本为0起 现加了1*/
    int  day;       /*!< day of the month(1-31)*/
    int  wday;      /*!< day of week (0-6)*/
    int  hour;      /*!< hours (0 - 23)    */
    int  minute;    /*!< minutes (0 - 59)*/
    int  second;    /*!< seconds (0 - 59)*/
}SYSTEM_TIME_T;

int   GetCurrentSystemTime(SYSTEM_TIME_T *pTime);
void  SystemSleep(unsigned int dwMilliSeconds);

#endif //_OASL_BASEAPI_H_

