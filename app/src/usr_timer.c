#include "includes.h"

volatile static unsigned int g_TickCnt=0;
g_Timer_info g_Timer[MAX_TIMER_ID];

//�ϵ��ʼ��ǰι��
//��ֹ��дflash��������
void Timer_PowOnFedDog(void)
{
	if(g_flag.bit.InitFinshFlag == INIT_NULL)
	{
		gpio_watchDog();
	}
}

void Timer_IRQHandlerCB(void)
{
	unsigned int sysRTC_Time=0;
	
	g_TickCnt ++;
 
	// 100ms
	if((g_TickCnt%10) == 0)
	{
		gpio_check();
		gpio_watchDog();
	}
	// 200ms
	if((g_TickCnt%20) == 0)
	{
		Timer_PowOnFedDog();
	}
	if((g_TickCnt%50) == 0)
	{
		gpio_led_proc();
	}
	// 1s
	if((g_TickCnt%100) == 0)
	{
		sysRTC_Time = RTC_GetForCounter();
	    sysRTC_Time++;
		RTC_SetForCounter(sysRTC_Time);

		sys_reset_day();
	}

	OS_Timer_ISR();
}
