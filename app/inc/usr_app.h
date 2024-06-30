#ifndef __USR_APP_H__
#define	__USR_APP_H__

#include "FreeRTOS.h"
#include "Queue.h"


#ifndef FALSE
#define FALSE    0
#endif

#ifndef TRUE
#define TRUE     1
#endif

typedef unsigned char       bool;

//debug开启任务堆栈检测
#define EN_CHECK_SYS_HEAPSIZE	 0


//-------------------------类型声明------------------------------
typedef struct{
	unsigned int sEventType;
	unsigned int param1;
	unsigned int param2;
}OS_EVENT;

#define portTICK_RATE_US			( ( TickType_t ) configTICK_RATE_HZ )

typedef enum
{
	MSG_MAIN_TIME_REQ = 0,
	MSG_MAIN_GSM_RECV_REQ,
	MSG_MAIN_ALARM_INDICATION,
	MSG_MAIN_CANCLE_ALARM_INDICATION,
	MSG_MAIN_GPIO_REQ,

}MAINTASK_EventList;

extern xQueueHandle xMainQueueId;
extern SemaphoreHandle_t xFlashMutex;
extern SemaphoreHandle_t xSendMutex;
extern SemaphoreHandle_t xMsgMutex;
extern SemaphoreHandle_t xLogMutex;

extern SemaphoreHandle_t xGsmSem;
extern SemaphoreHandle_t xSocketSem;


#define NorFlash_LockInit(c) 	c = xSemaphoreCreateMutex()
#define NorFlash_Lock(c) 		xSemaphoreTake(c, portMAX_DELAY)
#define NorFlash_UnLock(c) 		xSemaphoreGive(c)

#define GsmSend_LockInit(c) 	c = xSemaphoreCreateMutex()
#define GsmSend_Lock(c) 		xSemaphoreTake(c, portMAX_DELAY)
#define GsmSend_UnLock(c) 		xSemaphoreGive(c)

#define Msg_LockInit(c) 		c = xSemaphoreCreateMutex()
#define Msg_Lock(c) 			xSemaphoreTake(c, portMAX_DELAY)
#define Msg_UnLock(c) 			xSemaphoreGive(c)

#define Log_LockInit(c) 		c = xSemaphoreCreateMutex()
#define Log_Lock(c) 			xSemaphoreTake(c, portMAX_DELAY)
#define Log_UnLock(c) 			xSemaphoreGive(c)

#define GsmWait_SemInit(c)		c = xSemaphoreCreateBinary()
#define GsmWait_SemTake(c,t)	xSemaphoreTake(c, t)
#define GsmWait_SemGive(c)		xSemaphoreGive(c)

#define SocketWait_SemInit(c)	c = xSemaphoreCreateBinary()
#define SocketWait_SemTake(c,t)	xSemaphoreTake(c, t)
#define SocketWait_SemGive(c)	xSemaphoreGive(c)

void vGetStackHighWaterMark(char *TaskName);
void OS_TaskDelayMs(unsigned short nms);
void OS_TaskDelayUs(unsigned int nus);
void OS_SendMessage(xQueueHandle mType, unsigned int sTask, unsigned int param1, unsigned int param2);
void OS_GetMessage(xQueueHandle mType, OS_EVENT *EventParam);
void OS_SendMessageFromISR(xQueueHandle mType, unsigned int sTask, unsigned int param1, unsigned int param2);
void OS_Timer_Start(xQueueHandle mTask, unsigned int sTask, unsigned int param1, unsigned int param2, unsigned int timerId, unsigned int interval/*10ms单位*/, bool autoRepeat);
void OS_Timer_Stop(unsigned int timerId);
void OS_Timer_ISR(void);

void vMainTaskQueueCreat(void);
void vMainTask(void *pvParameters);


#endif
