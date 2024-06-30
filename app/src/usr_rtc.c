#include "includes.h"

volatile static unsigned int RTC_Count=0;

void RTC_Init(void)
{	
	
}

//�ж��Ƿ�Ϊ����
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
const unsigned char week_table[12]={0,3,3,6,1,4,6,2,5,0,3,5}; //���������ݱ�
/*
//������������ڼ�
//���빫�����ڵõ�����(ֻ����1901-2099 ��)
*/
unsigned char RTC_Get_Week(unsigned short year,unsigned char month,unsigned char day)
{
	unsigned short temp;
	unsigned char yearH,yearL;

	yearH = year/100; 
	yearL = year%100;
	// ���Ϊ21 ���ͣ��������100
	if(yearH>19)
		yearL += 100;
	// ����������ֻ��1900 ��֮���
	temp = yearL+yearL/4;
	temp = temp%7;
	temp = temp+day+week_table[month-1];
	if((yearL%4==0) && (month<3))
		temp--;
	return(temp%7);
}
/*
//����ʱ��
//�������ʱ��ת��Ϊ����
//��1970 ��1 ��1 �� 0ʱ 0�� 0��Ϊ��׼
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
		if(is_leap_year(year) && t==1)//����2�·�����һ��������� 
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
	if(daycnt != temp)//����һ��
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
		rtc->year = time;//ȡ�����

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
		rtc->month = time+1;//ȡ���·�
		rtc->day   = temp+1;//ȡ������
	}
	temp = timeCount%86400;
	rtc->hour    = temp/3600;
	rtc->minute  = (temp%3600)/60;
	rtc->second  = (temp%3600)%60;
	
	return 1; 
}
 
