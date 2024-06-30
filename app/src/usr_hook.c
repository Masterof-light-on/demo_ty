#include "includes.h"

void vApplicationTickHook(void)
{
	SoftWdtISR();
}

void vApplicationIdleHook(void)
{
	for( ;; )  
	{
		if(g_flag.bit.InitFinshFlag == INIT_OK)
		{
			sys_auto_backupApp();
		}
	}
}

//�ڴ�й©,����
void vApplicationMallocFailedHook(void)
{
	sys_SoftReset();
}

