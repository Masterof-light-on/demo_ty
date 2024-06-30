#include "includes.h"

//通过此队列传递触发的事件
xQueueHandle xMainQueueId;
SemaphoreHandle_t xFlashMutex;
SemaphoreHandle_t xSendMutex;
SemaphoreHandle_t xMsgMutex;
SemaphoreHandle_t xLogMutex;

SemaphoreHandle_t xGsmSem;
SemaphoreHandle_t xSocketSem;

//获取剩余的堆大小
//获取任务剩余的栈大小
//要开启宏定义:INCLUDE_uxTaskGetStackHighWaterMark
void vGetStackHighWaterMark(char *TaskName)
{
#if ( EN_CHECK_SYS_HEAPSIZE == 1 )
	LOG("[%s]FreeStackSize:%d  FreeHeapSize:%d\r\n", TaskName, uxTaskGetStackHighWaterMark(NULL), xPortGetFreeHeapSize());
#endif
}

void OS_TaskDelayMs(unsigned short nms) 
{	  
	portTickType xLastWakeTime;
	 
	xLastWakeTime = xTaskGetTickCount(); 
	vTaskDelayUntil(&xLastWakeTime, ( nms / portTICK_RATE_MS )); 
} 
    	     
void OS_TaskDelayUs(unsigned int nus) 
{	
	portTickType xLastWakeTime;
	 
	xLastWakeTime = xTaskGetTickCount(); 
	vTaskDelayUntil(&xLastWakeTime, ( nus * portTICK_RATE_US )); 
}

//非中断里调用
void OS_SendMessage(xQueueHandle mTask, unsigned int sTask, unsigned int param1, unsigned int param2)
{
	OS_EVENT EventParam;

	EventParam.sEventType = sTask;
	EventParam.param1 = param1;
	EventParam.param2 = param2;

	xQueueSend(mTask, &EventParam, 0);
}
void OS_GetMessage(xQueueHandle mType, OS_EVENT *EventParam)
{
	xQueueReceive(mType, EventParam, portMAX_DELAY);
}
//中断里调用
void OS_SendMessageFromISR(xQueueHandle mTask, unsigned int sTask, unsigned int param1, unsigned int param2)
{
	OS_EVENT EventParam;
	portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;

	EventParam.sEventType = sTask;
	EventParam.param1 = param1;
	EventParam.param2 = param2;

	xQueueSendFromISR(mTask, &EventParam, &xHigherPriorityTaskWoken);
	portEND_SWITCHING_ISR(xHigherPriorityTaskWoken);//立即切换上下文 
}
//开启定时器消息
void OS_Timer_Start(xQueueHandle mTask, unsigned int sTask, unsigned int param1, unsigned int param2, unsigned int timerId, unsigned int interval/*10ms单位*/, bool autoRepeat)
{
	g_Timer[timerId].mTask = mTask;
	g_Timer[timerId].sTask = sTask;
	g_Timer[timerId].param1 = param1;
	g_Timer[timerId].param2 = param2;
	g_Timer[timerId].ntime_interval = interval;
	g_Timer[timerId].ntime_bkpIntval= interval;
	g_Timer[timerId].ntime_autorepeat = autoRepeat;
}

//关闭定时器消息
void OS_Timer_Stop(unsigned int timerId)
{
	g_Timer[timerId].ntime_interval = 0;
	g_Timer[timerId].ntime_bkpIntval= 0;
}

// 10ms中断1次函数
void OS_Timer_ISR(void)
{
	unsigned char i;
	OS_EVENT EventTimer;
	portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
	
	for(i=0; i<MAX_TIMER_ID; i++)
	{
		if(g_Timer[i].ntime_interval)
		{
			g_Timer[i].ntime_interval--;
			if(g_Timer[i].ntime_interval == 0)
			{
				if(g_Timer[i].ntime_autorepeat == TRUE)
				{
					g_Timer[i].ntime_interval = g_Timer[i].ntime_bkpIntval;
				}

				EventTimer.sEventType = g_Timer[i].sTask;
				EventTimer.param1 = g_Timer[i].param1;
				EventTimer.param2 = g_Timer[i].param2;

				xQueueSendFromISR(g_Timer[i].mTask, &EventTimer, &xHigherPriorityTaskWoken);
			}
		}
	}

	portEND_SWITCHING_ISR(xHigherPriorityTaskWoken);//立即切换上下文 
}

void vMainTaskQueueCreat(void)
{
	xMainQueueId = xQueueCreate(8 , sizeof(OS_EVENT));
	if(xMainQueueId == 0)
	{
		LOG("#main task queue fail\r\n");
	}
}

void vMainTask(void *pvParameters)  
{  
	OS_EVENT Msg;

	/* 硬件初始化 */
    bsp_init();

	/* 版本号 */
	app_PrintVersion();
	
	OS_Timer_Start(xMainQueueId,MSG_MAIN_TIME_REQ,0,0,MAIN_TIMER_ID,100,TRUE);// 1s

	LOG("[task] main start\r\n");
	
	for( ;; )  
	{
		vGetStackHighWaterMark("MainTask");

		OS_GetMessage(xMainQueueId, &Msg);		
		switch(Msg.sEventType)
		{
			case MSG_MAIN_TIME_REQ://timer 1s
			{
				modem_gsm_reconnect_ip1();
				modem_reconnect_ip2();
				modem_gsm_reconnect_ip2();
				LOG("main run free:%d-%d\r\n",xPortGetFreeHeapSize(),uxTaskGetStackHighWaterMark(NULL));
			}
			break;
			
			case MSG_MAIN_GSM_RECV_REQ://socket data
			{
				modem_recv_proc(Msg.param1);
			}
			break;
			
			case MSG_MAIN_ALARM_INDICATION:
			{
				aw_send_rfid_msg(Msg.param1);
			}
			break;

			case MSG_MAIN_GPIO_REQ://gpio handle
			{
				gpio_handle(Msg.param1);
			}
			break;
			default:
			break;
		}	  
	}  
}

