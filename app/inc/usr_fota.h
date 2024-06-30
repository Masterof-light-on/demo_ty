#ifndef __USR_FOTA_H__
#define __USR_FOTA_H__

#define TFTPRRQ   0x0001//请求读取远程系统	   
#define TFTPWRQ   0x0002//请求写入远程系统	   
#define TFTPDATA  0x0003//数据	  
#define TFTPACK   0x0004//确认	 
#define TFTPERR   0x0005//错误 

typedef struct _UPDATE_INFO_
{
	unsigned char  bupdate;  //升级标志
	unsigned char  checksum; //升级文件的校验和
	unsigned short Port;
	unsigned char  Ip[16];
	unsigned char  FileName[20]; 
}update_info;

typedef struct __TFTPCTR
{
	unsigned int  RqNumb;      // 应该接收到的包序号
	unsigned int  DatLen;      // 已经收到的数据包长度
	unsigned int  EepAddr;     // 
	unsigned char UpDataSt;    // 远程服务器开始数据传输
}TFTPCTR;

typedef union _FW_INFO
{
	__packed struct _INFO
	{
		unsigned short ver_num;	//软件版本
		unsigned int fw_length;	//固件长度
		unsigned short fw_crc;		//固件CRC校验值
		unsigned char Rev[6];		//保留0xFF
		unsigned short head_crc;	//前面这14个字节的校验头
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
