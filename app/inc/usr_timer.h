#ifndef __USR_TIMER_H__
#define __USR_TIMER_H__

typedef enum
{
	MAIN_TIMER_ID = 0,
	GSM_TIMER_ID,
	GSM_RECV_TIMER_ID,
	IP2_TIMER_ID,
	GPS_TIMER_ID,
	GPS_RESET_TIMER_ID,
	DEV_TIMER_ID,
	CALL_TIMER_ID,
	MAX_TIMER_ID
	
}TIMERTASK_EventList;

typedef struct _G_TIMER_INFO
{
	volatile xQueueHandle mTask;
	volatile unsigned int  sTask;
	volatile unsigned int  param1;
	volatile unsigned int  param2;
	volatile bool ntime_autorepeat;
	volatile unsigned int ntime_bkpIntval;
	volatile unsigned int ntime_interval;
}g_Timer_info;


extern g_Timer_info g_Timer[MAX_TIMER_ID];

void Timer_IRQHandlerCB(void);


#endif
