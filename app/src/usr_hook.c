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

//ÄÚ´æÐ¹Â©,ÖØÆô
void vApplicationMallocFailedHook(void)
{
	sys_SoftReset();
}

