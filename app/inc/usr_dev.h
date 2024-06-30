#ifndef __USR_DEV_H__
#define __USR_DEV_H__

#include "usr_ring_buffer.h"

#define DEV_POWER_OFF_VOLT		500

#define URT_MODEM_RX_MASK 	(1024)
#define URT_AT_RX_MASK 	  	(1024)
#define URT_GPS_RX_MASK	  	(1024)
#define URT_COM_RX_MASK	  	(512)
#define URT_USB_RX_MASK 	(512)

extern RINGBUFF_T GSMModemRINGBUFF;
extern RINGBUFF_T GSMAtRINGBUFF;
extern RINGBUFF_T GPSRINGBUFF;
extern RINGBUFF_T Com0RINGBUFF;
extern RINGBUFF_T Com3RINGBUFF;
extern RINGBUFF_T Com4RINGBUFF;
extern RINGBUFF_T USBRINGBUFF;

//COM口
typedef enum
{
	COM0,
	COM1,
	COM2,
	COM3,
	COM4,
	COM5,
	USBCOM,
	ALL,
	
}e_dev_urt;

typedef enum
{
	MSG_DEV_TIME_REQ = 0,
	MSG_DEV_COM0_UART_REQ,
	MSG_DEV_COM1_UART_REQ,
	MSG_DEV_COM2_UART_REQ,
	MSG_DEV_COM3_UART_REQ,
	MSG_DEV_COM4_UART_REQ,
	MSG_DEV_COM5_UART_REQ,
	MSG_DEV_COM_USB_REQ,
	MSG_DEV_CAN_REQ,

}DEVTASK_EventList;

typedef struct _DEV_OIL_INFO_
{
	unsigned char	nStealOil;						//偷油百分比
	unsigned char	nLowOil;						//低油百分比
	unsigned short nOilVolume[4]; 					//油耗总容量
}dev_oil_info;

//外设类型
typedef enum
{
	NUL = 0,
	JT,
	CR,
	XD,
	
}e_dev_type;

//485外设轮询时间
typedef enum
{
	DEV_OIL_CH1_TIME 	= 3,
	DEV_OIL_CH2_TIME 	= 6,
	DEV_OIL_CH3_TIME 	= 9,
	DEV_OIL_CH4_TIME 	= 12,
	DEV_TEMP_CH_TIME 	= 15,
	DEV_MAX_POLL_TIME 	= 15,
		
}e_dev_poll_time;


typedef enum
{
	MAIN_POWOFF = 0,
	MAIN_POWON,
}MAINPOW_STATIS;

typedef struct
{
	int sKeyfor;
	unsigned short tempbt;
	dev_oil_info  oilinfo;				//油耗传感器
}dev_info;

extern xQueueHandle xDevQueueId;
extern unsigned short sDevPowerVolt[5];
extern unsigned char  sDev232_2time;

extern dev_info xdevinfo;
extern unsigned char rfidtimeout;
extern unsigned char DevPollSendTime;

void dev_Init(void);
void queue_init(void);
void dev_recv(unsigned char com_type, unsigned char *data, unsigned short len);

void vDevTaskQueueCreat(void);
void vDevTask(void *pvParameters);

#endif

