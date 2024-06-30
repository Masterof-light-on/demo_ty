#ifndef __USR_RTC_H__
#define __USR_RTC_H__	 

#include "usr_gps.h"

void RTC_Init(void);
unsigned char is_leap_year(unsigned short year);
void RTC_SetForCounter(unsigned int c);
unsigned int RTC_GetForCounter(void);
unsigned char RTC_Set_Time(TIME *rtc);
unsigned char RTC_Get_Time(TIME *rtc,unsigned int timeCount);

#endif

