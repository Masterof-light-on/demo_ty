#ifndef __USR_MSGGSM_H__
#define __USR_MSGGSM_H__

//使能位置信息是否按时间顺序发送 1:是  0:否
#define MSG_ACCORD_TO_TIME_SEND		0

#define MSGCOUNT     2      //一次保存的保存条数
#define DATALEN      255     //一条数据的存储长度
#define PAGECOUNT	 16		//一页保存的保存条数 4096/sizeof(msg_buf)

#define FLASHMAXCNT  2500 //flash中保存的最大条数


#define WRITE_BLIND_SIZE    ((DATALEN+1)*MSGCOUNT)
#define READ_BLIND_SIZE     (DATALEN+1)
#define FLASH_BLIND_SIZE    (64*1024)

typedef struct _MSG_FLASH_INFO
{
	unsigned int flashin; //FLASH中盲区存储的游标 
	unsigned int flashout; 	
}msg_flash_info;

typedef struct _MSG_BUF
{
	unsigned char len;
	unsigned char data[DATALEN];
}msg_buf;


typedef struct _MSG_INFO
{
	msg_buf Buf[MSGCOUNT]; //缓冲区 
	unsigned char In;                 //进出游标
	unsigned char Out;	
}msg_info;


void MsgGsm_Init(void);
void MsgGsm_Clear(void);
void MsgGsm_SaveBlindData(unsigned int index,unsigned char *buf);
void MsgGsm_ReadBlindData(unsigned int index,unsigned char *buf);
unsigned char MsgGsm_InputMsg(unsigned char *msg,unsigned short len);
unsigned char MsgGsm_OutputMsg(unsigned char *msg);
unsigned char MsgGsm_SaveFlash(unsigned char *buf);
unsigned char MsgGsm_ReadFlash(unsigned char *buf);
void MsgGsm_BlindPost_clear(void);
void MsgGsm_BlindIndex_clear(void);
void MsgGsm_BlindOutdex_clear(void);
void MsgGsm_BlindIndex_save(unsigned int indexPost);
void MsgGsm_BlindIndex_read(void);
void MsgGsm_BlindOutdex_save(unsigned int outdexPost);
void MsgGsm_BlindOutdex_read(void);
unsigned int MsgGsm_GetFlashIn(void);
unsigned int MsgGsm_GetFlashOut(void);

#endif
