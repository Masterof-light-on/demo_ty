#include "includes.h"

bool StopWDTFedMake = FALSE;
SOFT_WATCH_DOG_TIMER SoftWatchDogTimerList[MAX_SWDT_ID] = {0};

//初始化软件看门狗
bool SoftWdtInit(SWDT_ID SwdtId, unsigned short TimerTop/*单位s*/)
{
	SOFT_WATCH_DOG_TIMER *SoftWatchDogTimerPtr = SoftWatchDogTimerList;
	unsigned int osTick;

	if(SwdtId >= MAX_SWDT_ID)
	{
		return FALSE;
	}

	SoftWatchDogTimerPtr += SwdtId; 
	osTick = SToOSTicks(TimerTop);//将S时间换算成系统时钟节拍
	SoftWatchDogTimerPtr->watchDogTimeOut = osTick;
	SoftWatchDogTimerPtr->watchDogTime    = osTick;
	SoftWatchDogTimerPtr->watchDogState   = SWDT_STAT_SUSPEN;//默认挂起看门狗

	return TRUE;
}

//软件看门狗喂食
void SoftWdtFed(SWDT_ID SwdtId)
{
	SOFT_WATCH_DOG_TIMER *SoftWatchDogTimerPtr = SoftWatchDogTimerList;

	if(SwdtId >= MAX_SWDT_ID)
	{
		return;
	}
	SoftWatchDogTimerPtr += SwdtId; 
	SoftWatchDogTimerPtr->watchDogTime  = SoftWatchDogTimerPtr->watchDogTimeOut;
	SoftWatchDogTimerPtr->watchDogState = SWDT_STAT_RUN;//使能看门狗
}

//hook
void SoftWdtISR(void)
{
	SOFT_WATCH_DOG_TIMER *SoftWatchDogTimerPtr = SoftWatchDogTimerList;
	unsigned char i;

	//等待看门狗复位
	if(StopWDTFedMake == TRUE)
	{
		return;
	}
	for(i=0; i<MAX_SWDT_ID; i++)
	{
		//对挂起和空闲的看门狗定时器不进行检查获
		if(SoftWatchDogTimerPtr->watchDogState == SWDT_STAT_RUN)
		{
			if(--SoftWatchDogTimerPtr->watchDogTime == 0)
			{				
				LOG("[%d]***%d : %d***\r\n\r\n", i, uxTaskGetStackHighWaterMark(NULL), xPortGetFreeHeapSize());
				StopWDTFedMake = TRUE;
				sys_SoftReset();
				return;
			}
		}
		SoftWatchDogTimerPtr++;
	}	
}

unsigned int SToOSTicks(unsigned short in_val)
{
	return (unsigned int)((in_val * 1000 * configTICK_RATE_HZ)/1000);
}

