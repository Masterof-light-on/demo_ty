#ifndef __USR_FOTA_H__
#define __USR_FOTA_H__

#define TFTPRRQ   0x0001//�����ȡԶ��ϵͳ	   
#define TFTPWRQ   0x0002//����д��Զ��ϵͳ	   
#define TFTPDATA  0x0003//����	  
#define TFTPACK   0x0004//ȷ��	 
#define TFTPERR   0x0005//���� 

typedef struct _UPDATE_INFO_
{
	unsigned char  bupdate;  //������־
	unsigned char  checksum; //�����ļ���У���
	unsigned short Port;
	unsigned char  Ip[16];
	unsigned char  FileName[20]; 
}update_info;

typedef struct __TFTPCTR
{
	unsigned int  RqNumb;      // Ӧ�ý��յ��İ����
	unsigned int  DatLen;      // �Ѿ��յ������ݰ�����
	unsigned int  EepAddr;     // 
	unsigned char UpDataSt;    // Զ�̷�������ʼ���ݴ���
}TFTPCTR;

typedef union _FW_INFO
{
	__packed struct _INFO
	{
		unsigned short ver_num;	//����汾
		unsigned int fw_length;	//�̼�����
		unsigned short fw_crc;		//�̼�CRCУ��ֵ
		unsigned char Rev[6];		//����0xFF
		unsigned short head_crc;	//ǰ����14���ֽڵ�У��ͷ
	}INFO;
	
	unsigned char INFO_Buf[16];
}FW_INFO;

void update_handle(unsigned char flag,unsigned char *buf,unsigned short len);
void update_linkTFTP(unsigned char flag);
void TftpTarsk(unsigned char flag,unsigned char *inbuf,unsigned short updatlen);
void TftpDownQ(unsigned char flag,void *file);
unsigned char update_SetIp(unsigned char *ip, unsigned short len);
unsigned char update_SetPort(unsigned short port);
unsigned char update_SetFilename(unsigned char *filename);
unsigned char *update_GetIp(void);
unsigned short update_GetPort(void);
unsigned char *update_GetFilename(void);
void update_SetUpdateFlag(unsigned char flag);
unsigned char update_GetUpdateFlag(void);
void update_SetTFTPFlag(unsigned char flag);
unsigned char update_GetTFTPFlag(void);
void update_ClearFileReq(void);
void update_GetLastState(unsigned char state);
void update_Reconnect(void);


#endif
