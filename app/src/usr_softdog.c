#include "includes.h"

bool StopWDTFedMake = FALSE;
SOFT_WATCH_DOG_TIMER SoftWatchDogTimerList[MAX_SWDT_ID] = {0};

//��ʼ��������Ź�
bool SoftWdtInit(SWDT_ID SwdtId, unsigned short TimerTop/*��λs*/)
{
	SOFT_WATCH_DOG_TIMER *SoftWatchDogTimerPtr = SoftWatchDogTimerList;
	unsigned int osTick;

	if(SwdtId >= MAX_SWDT_ID)
	{
		return FALSE;
	}

	SoftWatchDogTimerPtr += SwdtId; 
	osTick = SToOSTicks(TimerTop);//��Sʱ�任���ϵͳʱ�ӽ���
	SoftWatchDogTimerPtr->watchDogTimeOut = osTick;
	SoftWatchDogTimerPtr->watchDogTime    = osTick;
	SoftWatchDogTimerPtr->watchDogState   = SWDT_STAT_SUSPEN;//Ĭ�Ϲ����Ź�

	return TRUE;
}

//������Ź�ιʳ
void SoftWdtFed(SWDT_ID SwdtId)
{
	SOFT_WATCH_DOG_TIMER *SoftWatchDogTimerPtr = SoftWatchDogTimerList;

	if(SwdtId >= MAX_SWDT_ID)
	{
		return;
	}
	SoftWatchDogTimerPtr += SwdtId; 
	SoftWatchDogTimerPtr->watchDogTime  = SoftWatchDogTimerPtr->watchDogTimeOut;
	SoftWatchDogTimerPtr->watchDogState = SWDT_STAT_RUN;//ʹ�ܿ��Ź�
}

//hook
void SoftWdtISR(void)
{
	SOFT_WATCH_DOG_TIMER *SoftWatchDogTimerPtr = SoftWatchDogTimerList;
	unsigned char i;

	//�ȴ����Ź���λ
	if(StopWDTFedMake == TRUE)
	{
		return;
	}
	for(i=0; i<MAX_SWDT_ID; i++)
	{
		//�Թ���Ϳ��еĿ��Ź���ʱ�������м���
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

