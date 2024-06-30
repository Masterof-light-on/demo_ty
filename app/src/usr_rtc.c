#include "includes.h"

volatile static unsigned int RTC_Count=0;

void RTC_Init(void)
{	
	
}

//判断是否为闰年
unsigned char is_leap_year(unsigned short year)
{
	if(((year%4==0)&&(year%100!=0))||(year%400 == 0))
	{
		return 1;	
	}
	else
	{
		return 0;
	}
}

void RTC_SetForCounter(unsigned int c)
{
	RTC_Count = c;	
}
unsigned int RTC_GetForCounter(void)
{
	return RTC_Count;
}

const unsigned char mon_table[12]={31,28,31,30,31,30,31,31,30,31,30,31};
const unsigned char week_table[12]={0,3,3,6,1,4,6,2,5,0,3,5}; //月修正数据表
/*
//获得现在是星期几
//输入公历日期得到星期(只允许1901-2099 年)
*/
unsigned char RTC_Get_Week(unsigned short year,unsigned char month,unsigned char day)
{
	unsigned short temp;
	unsigned char yearH,yearL;

	yearH = year/100; 
	yearL = year%100;
	// 如果为21 世纪，年份数加100
	if(yearH>19)
		yearL += 100;
	// 所过闰年数只算1900 年之后的
	temp = yearL+yearL/4;
	temp = temp%7;
	temp = temp+day+week_table[month-1];
	if((yearL%4==0) && (month<3))
		temp--;
	return(temp%7);
}
/*
//设置时钟
//把输入的时钟转换为秒钟
//以1970 年1 月1 日 0时 0分 0秒为基准
*/
unsigned char RTC_Set_Time(TIME *rtc)
{
	unsigned short t;
	unsigned short year,mon,day,hour,min,sec;
	unsigned int secCount = 0;

	year = rtc->year;
	mon  = rtc->month;
	day  = rtc->day;
	hour = rtc->hour;
	min  = rtc->minute;
	sec  = rtc->second;

	if(year < 2018 || year > 2099)
		return 0;
	for(t=1970;t<year;t++)
	{
		if(is_leap_year(t))
			secCount += 31622400;
		else
			secCount += 31536000;
	}
	for(t=0;t<(mon-1);t++)
	{
		secCount += mon_table[t]*86400;
		if(is_leap_year(year) && t==1)//闰年2月份增加一天的秒钟数 
			secCount += 86400;
	}
	secCount += (day-1)*86400;
	secCount += hour*3600;
	secCount += min*60;
	secCount += sec;

	RTC_SetForCounter(secCount);

	return 1;
}
unsigned char RTC_Get_Time(TIME *rtc,unsigned int timeCount)
{
	static unsigned short daycnt = 0;
	unsigned int temp = 0;
	unsigned short time = 0;

	temp = timeCount/86400;
	if(daycnt != temp)//超过一天
	{
		daycnt = temp;
		time = 1970;
		while(temp >= 365)
		{
			if(is_leap_year(time)){
				if(temp >= 366)
					temp -= 366;
				else
					break;
			}
			else{
				temp -= 365;	
			}
			time++;
		}
		rtc->year = time;//取得年份

		time = 0;
		while(temp >= 28)
		{
			if(is_leap_year(rtc->year)&&(time == 1)){
				if(temp >= 29)
					temp -= 29;
				else
					break;
			}
			else{
				if(temp >= mon_table[time])
				   temp -= mon_table[time];
				else
					break;
			}
			time++;
		}
		rtc->month = time+1;//取得月份
		rtc->day   = temp+1;//取得日期
	}
	temp = timeCount%86400;
	rtc->hour    = temp/3600;
	rtc->minute  = (temp%3600)/60;
	rtc->second  = (temp%3600)%60;
	
	return 1; 
}
 
