#ifndef __USR_MSGGSM_H__
#define __USR_MSGGSM_H__

//ʹ��λ����Ϣ�Ƿ�ʱ��˳���� 1:��  0:��
#define MSG_ACCORD_TO_TIME_SEND		0

#define MSGCOUNT     2      //һ�α���ı�������
#define DATALEN      255     //һ�����ݵĴ洢����
#define PAGECOUNT	 16		//һҳ����ı������� 4096/sizeof(msg_buf)

#define FLASHMAXCNT  2500 //flash�б�����������


#define WRITE_BLIND_SIZE    ((DATALEN+1)*MSGCOUNT)
#define READ_BLIND_SIZE     (DATALEN+1)
#define FLASH_BLIND_SIZE    (64*1024)

typedef struct _MSG_FLASH_INFO
{
	unsigned int flashin; //FLASH��ä���洢���α� 
	unsigned int flashout; 	
}msg_flash_info;

typedef struct _MSG_BUF
{
	unsigned char len;
	unsigned char data[DATALEN];
}msg_buf;


typedef struct _MSG_INFO
{
	msg_buf Buf[MSGCOUNT]; //������ 
	unsigned char In;                 //�����α�
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
