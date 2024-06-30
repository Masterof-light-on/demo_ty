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
	SWDT_STAT_IDLE,               	//������Ź�����
	SWDT_STAT_SUSPEN,               //������Ź�����
	SWDT_STAT_RUN                   //������Ź�����
}SWDT_STAT;

typedef struct soft_wach_dog_timer
{
	unsigned int watchDogTimeOut;            //���Ź�������ʱ��ֵ
	unsigned int watchDogTime;               //���Ź���ʱ��
	SWDT_STAT  watchDogState;       //���Ź���ʱ��״̬
}SOFT_WATCH_DOG_TIMER;

bool SoftWdtInit(SWDT_ID SwdtId, unsigned short TimerTop);
void SoftWdtFed(SWDT_ID SwdtId);
void SoftWdtISR(void);
unsigned int SToOSTicks(unsigned short in_val);

#endif

