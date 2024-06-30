#ifndef __USR_GSM_H__
#define __USR_GSM_H__

#include "usr_msg.h"

typedef struct _G_AT_INFO_
{
	unsigned short times;	  //等待时间
	char* pcmd;   //at命令字符串
	char* erc;    //应答
}g_at_info;

typedef struct _G_WAIT_RSP_
{
	unsigned char cmp_ack[32];
	unsigned char ret_value;
}g_wait_rsp;

typedef struct _G_GPRS_STATE_
{
	unsigned char nModemType;		//模块类型
	unsigned char nModemState; 	//模块状态
	unsigned char nCallState; 		//Call子状态
	unsigned char nCallAtaDelay;	//Call延时接听时间
	unsigned char nCallInNo[20];	//Call号码
	
	unsigned short nMcc;			//mcc
	unsigned short nMnc;			//mnc
	unsigned int nLac;			//lac
	unsigned int nCellid;		//cell id

	unsigned char nSocketState;	//Socket返回状态
	unsigned char nIp1State;		//IP1状态
	unsigned char nIp1Waittime;	//IP1重连等待时间
	unsigned char nIp1ErrCount;	//IP1联网错误次数

	unsigned char nBtScanSw;
	unsigned char nIp2State;		//IP1状态
	unsigned char nIp2Waittime;	//IP2重连等待时间
	unsigned char nIp2ErrCount;	//IP2联网错误次数
	unsigned char nCenterCount;	//center重连次数
	
	unsigned char npppCount;		//ppp重连次数
	unsigned char nLoginCount;		//登陆次数
	unsigned char nLoginCount2;		//登陆次数

	unsigned char nCsq; 			//信号强度值
	unsigned char nCgreg; 			//gprs注册状态
	unsigned char simei[16];		//IMEI
	unsigned char sCcid[32];		//CCID
	unsigned char sCops[32];		//COPS
	unsigned char sNetwork[32];	//网络类型
	unsigned int tick_value;		//tick
	unsigned int tick_value_ip2; //tick
}g_gprs;

typedef struct _ACKMODEM_INFO_
{
	unsigned char ack;				//发送置1    平台应答清零
	unsigned char sendcnt;         //发送次数
	unsigned short cmd;            //发送消息的命令字
}ackmodem_info;

typedef struct _QUEUE_INFO_
{
	unsigned char  buf[DATALEN+1]; //从队列中取得的要发送的数据
	unsigned short len;    		//从队列取得的即将要发送的数据长度
	unsigned int acktimeout;
}g_queue_info;


#define MAX_GSM_UART_RCV_LEN 		1024

typedef enum
{
	MSG_GSM_INIT = 0x00,
	MSG_GSM_POW_ON,
	MSG_GSM_CPIN_READY,
	MSG_GSM_CGREG,
	MSG_GSM_PPP,
	MSG_GSM_IP,
	MSG_GSM_LOGIN,
	MSG_GSM_CENTER,
	MSG_GSM_CLOSEIP,
	MSG_GSM_ACCOFF,
	MSG_GSM_DAILCLOSE,
	MSG_GSM_READ_SOCKET,
	MSG_GSM_RECV_SOCKET,

	MSG_GSM_SEND_ALARM_SMS = 0x10,
	MSG_GSM_SEND_SMS,
	MSG_GSM_READ_SMS,
	MSG_GSM_CHECK_SMS,
	MSG_GSM_DEL_ALL_SMS,

	MSG_GSM_CALL = 0x20,
	MSG_GSM_ATA,
	MSG_GSM_RING,
	MSG_GSM_NO_CARRIER,
	MSG_GSM_NO_ANSWER,
	MSG_GSM_BUSY,

	MSG_GSM_UPDATE_START = 0x30,
	MSG_GSM_UPDATE_ING,
	
	MSG_GSM_IP2 = 0x40,
	MSG_GSM_LOGIN2,
	MSG_GSM_CENTER2,
	
	MSG_GSM_CENTER3 = 0x50,
}GSMTASK_EventList;

typedef enum
{
	GSM_INIT_STATE = 0,
	GSM_CSQ_SATTE,
	GSM_CREG_STATE,
	GSM_PPP_STATE,
	GSM_IP_STATE,
	GSM_LOGIN_STATE,
	GSM_ONLINE_STATE,
	GSM_FOTA_STATE,
	GSM_CALL_STATE,
	GSM_CLOSE_STATE
}GSMSTATE_List;

typedef enum 
{
	RET_AT_TIMEOUT           = 0,
    RET_AT_SUCCESS           = 1,
    RET_AT_ERROR             = 2,
    RET_AT_RING              = 3,
    RET_AT_WAITING         	 = 4,
}Enum_ATRspError;

//模块类型
#define GSM_MODEM_NULL					0
#define GSM_MODEM_QUECTEL_EC20			1
#define GSM_MODEM_SIMCOM_A7600C			2
#define GSM_MODEM_LYNQ_L511C			3

#define IF_CONDITION_EC20			if(g_modem_runtime.nModemType == GSM_MODEM_QUECTEL_EC20)
#define IF_CONDITION_EC2x			if((g_modem_runtime.nModemType == GSM_MODEM_QUECTEL_EC20))
#define IF_CONDITION_A7600C			if(g_modem_runtime.nModemType == GSM_MODEM_SIMCOM_A7600C)
#define IF_CONDITION_LYNQ			if(g_modem_runtime.nModemType == GSM_MODEM_LYNQ_L511C)


#define CALL_NULL		0
#define CALL_IN         1
#define CALL_OUT        2
#define CALL_CONNECT	3
#define CALL_RINGING	4

extern xQueueHandle xGsmQueueId;
extern g_gprs g_modem_runtime;

void modem_init(void);
void modem_uartInit(void);
void modem_powerOn(void);
void modem_reset(void);
void modem_sendChar(unsigned char ch);
void modem_sendStr(unsigned char *p,int len);
void modem_recv(unsigned char *data, unsigned short len);
void modem_clearModBuf(void);
void modem_clearAtBuf(void);
unsigned short modem_getModBufLen(void);
unsigned short modem_getAtBufLen(void);
unsigned char *modem_getAtBuf(void);
unsigned char modem_getModChar(unsigned char *ch);
unsigned char modem_getModStr(unsigned char * pdst,unsigned short n);
void modem_ClearAtErrCnt(void);
void modem_SetAckModemInfo(ackmodem_info *info);
ackmodem_info* modem_GetAckModemInfo(void);
void modem_AckModemInit(void);
void modem_SetAckModem(unsigned char ack);
unsigned char modem_GetAckModem(void);
void modem_SetAckSendInit(unsigned char cmd,unsigned char checksum);
unsigned char modem_GetCpinStatus(void);
void modem_SetImei(unsigned char *buf);
unsigned char *modem_GetImei(void);
void modem_SetCcid(unsigned char *buf);
unsigned char *modem_GetCcid(void);
void modem_SetCgreg(unsigned char cgreg);
unsigned char modem_GetCgreg(void);
void modem_SetCsq(unsigned char csq);
unsigned char modem_GetCsq(void);

void modem_SetMcc(unsigned short mcc);
unsigned short modem_GetMcc(void);
void modem_SetMnc(unsigned short mnc);
unsigned short modem_GetMnc(void);
void modem_SetLac(unsigned int lac);
unsigned int modem_GetLac(void);
void modem_SetCellid(unsigned int cellid);
unsigned int modem_GetCellid(void);

void modem_SetCallState(unsigned char state);
void modem_ClearCallState(void);
unsigned char modem_GetCallState(void);

void modem_HandUpClear(void);
unsigned char modem_Set_Mic(unsigned char volume);
unsigned char modem_Set_Volume(unsigned char volume);
unsigned char modem_HangPhone(void);
void modem_NoiseCtl(void);
unsigned char modem_AnwserPhone(void);
unsigned char modem_CallPhone(unsigned char *data,unsigned char flag);
void modem_ata_delay(void);

void modem_clear_flagCnt(void);
void modem_send_disconnect(unsigned char IsSetParam);
void modem_gsm_disconnect_ip1(void);
void modem_gsm_logout(void);
void modem_gsm_reconnect_ip1(void);
void modem_gsm_ip1_errcheck(void);
void modem_send_AtStr(char* str);
unsigned char modem_send_gprsData(char* data,char* Erc,unsigned short Time,unsigned short len);
unsigned char modem_send_Atcmd(char* Cmd,char* Erc,unsigned short Time);
unsigned char modem_seek_AtAck(char* ack,unsigned short time);
void modem_response(unsigned char* rsp, unsigned short len);
void modem_VersionCheck(void);
void modem_ImeiCheck(void);
void modem_CcidCheck(void);
void modem_UrcportCheck(void);
void modem_CopsCheck(void);
void modem_CsqCheck(void);
void modem_CgregCheck(void);
void modem_QinistatCheck(void);
void modem_recv_proc(unsigned short urclen);
void modem_SetState(unsigned char state);
unsigned char modem_GetState(void);
void modem_queue_send(void);

void vGsmTaskQueueCreat(void);
void vGsmTask(void *pvParameters);

#endif

