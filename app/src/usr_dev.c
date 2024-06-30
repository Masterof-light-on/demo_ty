#include "includes.h"

/* 队列句柄 */
xQueueHandle xDevQueueId;

//Ring buffer
RINGBUFF_T GSMModemRINGBUFF;
RINGBUFF_T GSMAtRINGBUFF;
RINGBUFF_T GPSRINGBUFF;
RINGBUFF_T Com0RINGBUFF;
RINGBUFF_T Com3RINGBUFF;
RINGBUFF_T Com4RINGBUFF;
RINGBUFF_T USBRINGBUFF;

//GSM
unsigned char gGSMModemBuf[URT_MODEM_RX_MASK]; 
unsigned char gGSMAtBuf[URT_AT_RX_MASK];
//GPS
unsigned char gGPSQueueBuf[URT_GPS_RX_MASK];
//COM0
unsigned char gCom0QueueBuf[URT_COM_RX_MASK];
//COM3
unsigned char gCom3QueueBuf[URT_COM_RX_MASK];
//COM4
unsigned char gCom4QueueBuf[URT_COM_RX_MASK];
//USB
unsigned char gUSBQueueBuf[URT_USB_RX_MASK];


unsigned short sDevPowerVolt[5];
unsigned char  sDevPowerPer  = 0;
unsigned char  sDev232_2time  = 0;
unsigned char rfidtimeout = 0;
dev_info xdevinfo;
//轮询发送时间
unsigned char DevPollSendTime = 0;

void dev_Init(void)
{

//	uart0_init(115200);//232  1
//	uart3_init(115200);//bt
//	uart4_init(9600);//232  2
	serialoil_Init();
}

void queue_init(void)
{
	RingBuffer_Init(&GSMModemRINGBUFF, (unsigned char *)gGSMModemBuf, sizeof(unsigned char), NELEMENTS(gGSMModemBuf));
	RingBuffer_Init(&GSMAtRINGBUFF, (unsigned char *)gGSMAtBuf, sizeof(unsigned char), NELEMENTS(gGSMAtBuf));
	RingBuffer_Init(&GPSRINGBUFF, (unsigned char *)gGPSQueueBuf, sizeof(unsigned char), NELEMENTS(gGPSQueueBuf));
	RingBuffer_Init(&Com0RINGBUFF, (unsigned char *)gCom0QueueBuf, sizeof(unsigned char), NELEMENTS(gCom0QueueBuf));
	RingBuffer_Init(&Com3RINGBUFF, (unsigned char *)gCom3QueueBuf, sizeof(unsigned char), NELEMENTS(gCom3QueueBuf));
	RingBuffer_Init(&Com4RINGBUFF, (unsigned char *)gCom4QueueBuf, sizeof(unsigned char), NELEMENTS(gCom4QueueBuf));
	RingBuffer_Init(&USBRINGBUFF, (unsigned char *)gUSBQueueBuf, sizeof(unsigned char), NELEMENTS(gUSBQueueBuf));
}

/******************************************************************************/
//外部电压采集
void dev_power_proc(void)
{		
	sDevPowerVolt[0] = adc_get_data(0)*0.34375f;
	sDevPowerVolt[1] = adc_get_data(1)*3.4375f;
	sDevPowerVolt[2] = adc_get_data(2)*3.4375f;
	sDevPowerVolt[3] = adc_get_data(3)*0.34375f;
	sDevPowerVolt[4] = adc_get_data(4)*0.34375f;
}

/******************************************************************************/

/******************************************************************************/
void dev_recv(unsigned char com_type, unsigned char *data, unsigned short len)
{
	switch(com_type)
	{
		case COM0:
		{
			RingBuffer_InsertMult(&Com0RINGBUFF, (unsigned char *)data, len);
			OS_SendMessageFromISR(xDevQueueId, MSG_DEV_COM0_UART_REQ, com_type, 0);
		}
		break;
		case COM3:
		{
			RingBuffer_InsertMult(&Com3RINGBUFF, (unsigned char *)data, len);
			OS_SendMessageFromISR(xDevQueueId, MSG_DEV_COM3_UART_REQ, com_type, 0);
		}
		break;
		case COM4:
		{
			RingBuffer_InsertMult(&Com4RINGBUFF, (unsigned char *)data, len);
			OS_SendMessageFromISR(xDevQueueId, MSG_DEV_COM4_UART_REQ, com_type, 0);
		}
		break;
	}
}

void dev_com_recv_proc(unsigned char com_type)
{
	unsigned char *pUartComBuf = NULL;
	unsigned short len;
	switch(com_type)
	{
		case COM0:
		{
			pUartComBuf = (unsigned char *)pvPortMalloc(UART0_MAX_RX);
			if(pUartComBuf == NULL)
			{
				return;
			}
			memset_t(pUartComBuf, 0, UART0_MAX_RX);
		
			len = RingBuffer_GetCount(&Com0RINGBUFF);
			RingBuffer_PopMult(&Com0RINGBUFF, (unsigned char *)pUartComBuf, len);
			handset_SetSchedul(com_type,pUartComBuf, len);
		}
		break;
		case COM3:
		{
			pUartComBuf = (unsigned char *)pvPortMalloc(UART3_MAX_RX);
			if(pUartComBuf == NULL)
			{
				return;
			}
			memset_t(pUartComBuf, 0, UART3_MAX_RX);
			
			len = RingBuffer_GetCount(&Com3RINGBUFF);
			RingBuffer_PopMult(&Com3RINGBUFF, (unsigned char *)pUartComBuf, len);
		}
		break;
		case COM4:
		{
			pUartComBuf = (unsigned char *)pvPortMalloc(UART4_MAX_RX);
			if(pUartComBuf == NULL)
			{
				return;
			}
			memset_t(pUartComBuf, 0, UART4_MAX_RX);
			
			len = RingBuffer_GetCount(&Com4RINGBUFF);
			RingBuffer_PopMult(&Com4RINGBUFF, (unsigned char *)pUartComBuf, len);
			if((pUartComBuf[0] == 'O')&&(pUartComBuf[1] == 'K'))
			{
				sDev232_2time = 5;
			}
			handset_SetSchedul(com_type,pUartComBuf, len);
			//OIL
			serialoil_handle(com_type,pUartComBuf, len);
		}
		break;
		case USBCOM:
		{
			pUartComBuf = (unsigned char *)pvPortMalloc(URT_USB_RX_MASK);
			if(pUartComBuf == NULL)
			{
				return;
			}
			memset_t(pUartComBuf, 0, URT_USB_RX_MASK);
			
			len = RingBuffer_GetCount(&USBRINGBUFF);
			RingBuffer_PopMult(&USBRINGBUFF, (unsigned char *)pUartComBuf, len);
			handset_SetSchedul(com_type,pUartComBuf, len);
		}
		break;
	}	
	vPortFree(pUartComBuf);
}

/******************************************************************************/
void vDevTaskQueueCreat(void)
{
	xDevQueueId = xQueueCreate(64 , sizeof(OS_EVENT));
	if(xDevQueueId == 0)
	{
		LOG("#dev task queue fail\r\n");
	}
}

void vDevTask(void *pvParameters)  
{  
	OS_EVENT Msg;

	OS_Timer_Start(xDevQueueId,MSG_DEV_TIME_REQ,0,0,DEV_TIMER_ID,100,TRUE);// 1s

	LOG("[task] dev start\r\n");
	
	for( ;; )  
	{
		vGetStackHighWaterMark("DEV");

		OS_GetMessage(xDevQueueId, &Msg);		
		switch(Msg.sEventType)
		{
			case MSG_DEV_TIME_REQ://timer 1s
			{
				dev_power_proc();
				LOG("dev run free:%d\r\n",uxTaskGetStackHighWaterMark(NULL));
			}
			break;

			case MSG_DEV_COM0_UART_REQ:
			case MSG_DEV_COM1_UART_REQ:
			case MSG_DEV_COM2_UART_REQ:
			case MSG_DEV_COM3_UART_REQ:
			case MSG_DEV_COM4_UART_REQ:
			case MSG_DEV_COM5_UART_REQ:
			case MSG_DEV_COM_USB_REQ:
			{
				dev_com_recv_proc(Msg.param1);
			}
			break;

			case MSG_DEV_CAN_REQ:
			{				
				
			}
			break;
							
			default:
			break;
		}	  
	}  
}

