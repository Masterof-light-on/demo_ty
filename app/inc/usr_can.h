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
	unsigned char  TEMP1;									//ˮ��
	unsigned char  TEMP2;									//����
	unsigned char  TEMP3;									//����							
	unsigned char  TEMP4;									//����
	unsigned short GAS;									//��˹Ũ��
	unsigned short VSS;									//����
	unsigned short RPM;									//ת��
	unsigned int MILEAGE;								//���
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
