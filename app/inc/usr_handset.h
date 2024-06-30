#ifndef __USR_HANDSET_H__
#define __USR_HANDSET_H__

#define PC_HEARD_MSG			"*F10#"
#define PC_HEARD_LEN			5

#define SAVE_RUN_FLAG			0x01
#define SAVE_SYS_FLAG			0x10

typedef enum
{
	DDP_NULL = 0,
	DDP_ACK_SUCCESS,
	DDP_ACK_TIMEOUT,
}eDDPAck;

typedef enum
{
	PC_NULL = 0,
	PC_DISCONNECT_TCPIP,
	PC_RESTART,
	PC_FACTORY,

}ePcStatus;

extern unsigned char gc_debug;
extern unsigned char gc_watchdog;
extern unsigned char sSave_flag;
extern unsigned char sReppp_flag;
extern unsigned char sReset_flag;
extern const unsigned char gCarLicense_table[][4];

void handset_Init(void);

unsigned char handset_ConfigSet(unsigned char* pdata, unsigned short len, unsigned char *pOut);
void handset_SetSchedul(unsigned char com_type,unsigned char *buf, unsigned short dataLen);
int handset_all_command(unsigned char *str, unsigned short len, unsigned char *out);
int handset_ota_waiting(void);
void handset_handle(unsigned char com_type, unsigned char *data, unsigned short dataLen);

unsigned char handset_SetIP1(unsigned char* pdata);
unsigned char handset_SetIP2(unsigned char* pdata);
unsigned char handset_SetVin(unsigned char *data);
unsigned char handset_SetId(unsigned char *data);
unsigned char handset_SetSn(unsigned char *data);
unsigned char handset_SetApn(unsigned char* pdata);
unsigned char handset_SetUpgrade(unsigned char *data);
void handset_CheckParam(unsigned char *pOut);
void handset_CheckParamAll(unsigned char *pOut);
void handset_CheckRun(unsigned char *pOut);

unsigned char handset_SetAccOnOffInterval(unsigned char *data);
unsigned char handset_SetRedeem(unsigned char *data);
unsigned char handset_SetATAParam(unsigned char *data);
unsigned char handset_SetMileage(unsigned char *data);
unsigned char handset_SetTimeZone(unsigned char *data);
unsigned char handset_SetDevice(unsigned char *data);
void handset_CheckDevice(unsigned char *pOut);
unsigned char handset_SetCanbaud(unsigned char *data);

unsigned char handset_SetAuthorPhone(unsigned char *data);
void handset_gc_mode(void);


#endif
