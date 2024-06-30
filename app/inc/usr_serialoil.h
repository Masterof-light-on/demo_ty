#ifndef __USR_SERIALOIL_H__
#define __USR_SERIALOIL_H__


#define MAX_OIL_CH		4
typedef struct _G_OIL_BUF
{
	unsigned int nCurOilPercent[MAX_OIL_CH];
	unsigned int nCurOilVolume[MAX_OIL_CH];
	unsigned int nCurOilAD[MAX_OIL_CH];
}g_oil_buf;

typedef struct _G_OIL_CHECK
{
	unsigned char nOffLineAlarmCount[MAX_OIL_CH];	//断线报警计时
	unsigned char nOffLineAlarmSt;					//断线报警状态
}g_oil_check;

void serialoil_Init(void);
void serialoil_PowerOn(void);
void serialoil_PowerOff(void);
void serialoil_UartInit(void);
void serialoil_noinit_clear(void);
unsigned char serialoil_enable_urtfun(unsigned char type);
void serialoil_offalarm_check(void);
void serialoil_abnormal_check(void);
void serialoil_uart_set(unsigned char *data, unsigned char *pOut);
void serialoil_poll_send(void);
unsigned short serialoil_get_percent(unsigned char id);
void serialoil_handle(unsigned char com_type, unsigned char* data, unsigned int len);

#endif

