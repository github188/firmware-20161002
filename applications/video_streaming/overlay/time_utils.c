#include <rtthread.h>
#include "time_utils.h"

#define MS_NUM_PER_TICK (1000 / RT_TICK_PER_SECOND)

static unsigned long long sSysUs = 0ULL;
static unsigned int sSysLastUs = 0;
static unsigned char sWeekInx1970Arr[7] = {4,5,6,0,1,2,3}; // 1970.1.1 为周四
static unsigned char sWeekInx2000Arr[7] = {6,0,1,2,3,4,5}; 

#define s_read_reg(addr)           (*((volatile unsigned int *)(addr)))
static unsigned int _getptsByReadHwReg(void)
{
    #define S_PTS_ADDR  (0xec100040) // PTS_ADDR            (0xec100040)
    #define S_PTS_OVERFLOW_INTERVAL (0x80000000) // PTS_OVERFLOW_INTERVAL     (0x80000000)
    unsigned int ptsHwAddr = S_PTS_ADDR;

    return s_read_reg(ptsHwAddr);
}

static unsigned char _assistGetWeekIdxByDays(unsigned char baseFlag, unsigned int curdays)
{
    unsigned char idx;

    idx = curdays % 7;
    return ((baseFlag == 0) ? sWeekInx1970Arr[idx] : sWeekInx2000Arr[idx]);
}

static unsigned char _tr_IsLeapYear(unsigned short year)
{
    if ((0 == year % 4 && 0 != year % 100) || (0 == year % 400))
        return 1;
    return 0;
}

static unsigned char _tr_GetMonthLen(unsigned short year, unsigned char month)
{
    if (month < 1 || month > 12)
        return 0;

    if ((month <= 7 && 1 == month % 2) || (month >= 8 && 0 == month % 2))
        return 31;

    if (2 == month)
    {
        if (_tr_IsLeapYear(year))
            return 29;
        return 28;
    }

    return 30;
}

unsigned long long GetSystemUSTime(void)
{
    unsigned int curUs;
    unsigned long long nowus;
    
    rt_enter_critical();
    curUs = _getptsByReadHwReg();
    if(curUs < sSysLastUs)
    {
        sSysUs += (0x100000000ULL + (unsigned long long)curUs - (unsigned long long)sSysLastUs);
    }
    else
        sSysUs += (curUs - sSysLastUs);
    sSysLastUs = curUs;
    nowus = sSysUs;
    rt_exit_critical();
    
    return nowus;
}

/*!  
 将系统时间的日历时间格式（以秒为单位，距离1970的秒数）转成分解格式
 \param Sec 距离1970的秒数.
 \param pTime 分解格式的系统时间.
 \return 成功：0， 失败： -1
 \sa MkTime, MkSystemUTime
 */
int MkSystemTime(unsigned int Sec, SYSTEM_TIME_T *pTime)
{
    unsigned long idx;
    unsigned long day;
    unsigned long yearLen;
    unsigned long monthLen;
    
    if (RT_NULL == pTime)
        return -1;

    pTime->second = Sec % 60;  
    pTime->minute = (Sec / 60) % 60;
    pTime->hour = (Sec / 3600) % 24;
    day = Sec / 86400 + 1;  // (24 * 60 * 60)

    for (idx = 1970; ; idx++)
    {
        if (_tr_IsLeapYear(idx))
            yearLen = 366;
        else
            yearLen = 365;
        if (day <= yearLen)
            break;
        day -= yearLen;
    }
    pTime->year = idx;

    for (idx = 1; idx <= 12; idx++)
    {
        monthLen = _tr_GetMonthLen(pTime->year, idx);
        if (day <= monthLen)
            break;
        day -= monthLen;
    }
    pTime->month = idx;
    pTime->day = day;
    return 0;
}

/*!  
 获取当前的系统时间
 \param pTime 存放返回结果.
 \return 0
 \sa SetCurrentSystemTime, GetSystemMSCount, GetSystemUSCount
 */
int GetCurrentSystemTime (SYSTEM_TIME_T *pTime)
{
    unsigned long ret_cur_sec;
    unsigned int curdays;
    SYSTEM_TIME_T systm;

    ret_cur_sec = (unsigned long)(GetSystemUSTime() / 1000000);
    MkSystemTime(ret_cur_sec, &systm);
    curdays = ret_cur_sec / 86400; // (24 * 3600)
    if(pTime)
    {
        pTime->year = systm.year;
        pTime->month = systm.month;
        pTime->day = systm.day;
        pTime->hour = systm.hour; // TBD_WAIT... 时区问题
        pTime->minute = systm.minute;
        pTime->second = systm.second;
        pTime->wday = _assistGetWeekIdxByDays(0,curdays);
    }
    return 0;
}

/*!  
 使当前线程睡眠指定时间
 \param dwMilliSeconds 睡眠的毫秒数
 \return 无
 */
void SystemSleep(unsigned int dwMilliSeconds)
{
    if(dwMilliSeconds < MS_NUM_PER_TICK) // 2015.08.26 add
        dwMilliSeconds = MS_NUM_PER_TICK;
    rt_thread_sleep(dwMilliSeconds / MS_NUM_PER_TICK);
}