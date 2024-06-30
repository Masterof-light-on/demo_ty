#ifndef __USR_CAN_H__
#define __USR_CAN_H__

#define CAN_MAX_QUEUE	64

typedef enum
{
	CAN_CH1 = 0,
	CAN_CH2,
}CAN_CH;

typedef struct _CAN_INFO
{
	unsigned char  TEMP1;									//水温
	unsigned char  TEMP2;									//表温
	unsigned char  TEMP3;									//排温							
	unsigned char  TEMP4;									//油温
	unsigned short GAS;									//瓦斯浓度
	unsigned short VSS;									//车速
	unsigned short RPM;									//转速
	unsigned int MILEAGE;								//里程
}CAN_INFO;

typedef struct
{
    unsigned int id;
    unsigned char data[8];
}CAN_MSG;

extern CAN_INFO g_obd;

void can_ringbuffer_Init(void);
void can_STB_Init(void);
void can_recv(unsigned char c, can_receive_message_struct *RxMessage);
void can_recv_proc(unsigned char c);
void can_engine_check(void);
void can_stddata_proc(unsigned int id, unsigned char *data);
void can_extdata_proc(unsigned int id, unsigned char *data);
void can_timeout(void);
unsigned char can_getTimeout(void);

#endif
