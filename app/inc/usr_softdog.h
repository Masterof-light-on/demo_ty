#ifndef __USR_SOFTDOG_H__
#define __USR_SOFTDOG_H__	 

typedef enum
{
	MAIN_TASK_SWDT_TIME = 30,
	GSM_TASK_SWDT_TIME  = 180,
	GPS_TASK_SWDT_TIME  = 30,
	DEV_TASK_SWDT_TIME  = 30,
}SWDT_TIME;

typedef enum
{
	MAIN_TASK_SWDT_ID = 0,
	GSM_TASK_SWDT_ID,
	GPS_TASK_SWDT_ID,
	DEV_TASK_SWDT_ID,
	MAX_SWDT_ID
}SWDT_ID;

typedef enum
{
	SWDT_STAT_IDLE,               	//软件看门狗空闲
	SWDT_STAT_SUSPEN,               //软件看门狗挂起
	SWDT_STAT_RUN                   //软件看门狗运行
}SWDT_STAT;

typedef struct soft_wach_dog_timer
{
	unsigned int watchDogTimeOut;            //看门狗计数超时初值
	unsigned int watchDogTime;               //看门狗定时器
	SWDT_STAT  watchDogState;       //看门狗定时器状态
}SOFT_WATCH_DOG_TIMER;

bool SoftWdtInit(SWDT_ID SwdtId, unsigned short TimerTop);
void SoftWdtFed(SWDT_ID SwdtId);
void SoftWdtISR(void);
unsigned int SToOSTicks(unsigned short in_val);

#endif

