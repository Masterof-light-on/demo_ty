#include "includes.h"

static update_info  aw_update_info;
static TFTPCTR tftpctr;
static FW_INFO FW;
static unsigned char nFotaTime=0;
static unsigned char nFotaTimeOutStart=0;
static unsigned char nFileReqTime=0;
static unsigned char nFileReqCount=0;
static unsigned char nTftpCount=0;
static unsigned char nRestartTftp=0;
static unsigned char nLinkStep=0;
static unsigned char ackbuf[6];
static unsigned char gLastGprsState=0;

static unsigned short CRC_calc(unsigned short InitCRC, unsigned char *start, unsigned char *end)
{
	unsigned short crc = InitCRC;
	unsigned char *data= NULL;

	for (data = start; data <= end; data++)
	{
		crc  = (crc >> 8) | (crc << 8);
		crc ^= *data;
		crc ^= (crc & 0xff) >> 4;
		crc ^= crc << 12;
		crc ^= (crc & 0xff) << 5;
	}
	return crc;
}

void update_handle(unsigned char flag,unsigned char *buf,unsigned short len)
{
	if(update_GetUpdateFlag())
	{
		TftpTarsk(flag,buf,len);	
	}
}

//1000ms����
void update_linkTFTP(unsigned char flag)
{
	char buf[128]={0};

	if(nLinkStep == 2)
	{
		nFotaTimeOutStart = 1;//������ʼ��ʱ
	}
	if(nFotaTimeOutStart)
	{
		if(nFotaTime)
		{
			nFotaTime--;
			if(nFotaTime == 0)//������ʱʧ��
			{												
				LOG("[fota] timeout\r\n");

				OS_Timer_Stop(GSM_TIMER_ID);
				modem_send_disconnect(0);

				sys_SetUpdateFlag(2);
				memset_t(&aw_update_info,0,sizeof(aw_update_info));
				return;
			}
		}
	}
	
	switch(nLinkStep)
	{
		case 0:

			IF_CONDITION_A7600C
			{
				modem_send_Atcmd("AT+CIPCLOSE=1","OK",100);
			}
			IF_CONDITION_EC20
			{
				modem_send_Atcmd("AT+QICLOSE=1","OK",100);
			}

			IF_CONDITION_A7600C
			{
				memset_t((char *)buf,0,sizeof(buf));
				sprintf((char*)buf,"AT+CIPOPEN=1,\"UDP\",\"%s\",%d,%d",update_GetIp(),update_GetPort(),update_GetPort());

				if(modem_send_Atcmd(buf,"OK",30) == RET_AT_SUCCESS)
				{
					if(update_GetTFTPFlag())
					{
						nLinkStep = 1;
						nRestartTftp = 1;//�ϵ�����
						nFileReqTime = 0;
						update_SetTFTPFlag(0);
					}
					else
					{
						nLinkStep = 1;
						nFileReqTime = 0;
					}
					break;
				}
			}

			IF_CONDITION_EC20
			{
				memset_t((char *)buf,0,sizeof(buf));
				sprintf((char*)buf,"AT+QIOPEN=1,1,\"UDP\",\"%s\",%d,0,1",update_GetIp(),(unsigned short)update_GetPort());

				g_modem_runtime.nSocketState = RET_AT_WAITING;
				if(modem_send_Atcmd(buf,"OK",30) == RET_AT_SUCCESS)
				{
					SocketWait_SemTake(xSocketSem, (30*1000));
					if(g_modem_runtime.nSocketState == RET_AT_SUCCESS)
					{
						if(modem_send_Atcmd("AT+QISTATE=1,1","+QISTATE:",30) == RET_AT_SUCCESS)
						{
							if(update_GetTFTPFlag())
							{
								nLinkStep = 1;
								nRestartTftp = 1;//�ϵ�����
								nFileReqTime = 0;
								update_SetTFTPFlag(0);
							}
							else
							{
								nLinkStep = 1;
								nFileReqTime = 0;
							}
							break;
						}
					}
					else
					{
						g_modem_runtime.nSocketState = RET_AT_TIMEOUT;
						LOG("+QIOPEN: error/timeout\r\n");
					}
				}
				else
				{
					g_modem_runtime.nSocketState = RET_AT_ERROR;
				}
			}

			nLinkStep = 0;
			break;
		case 1:
			if(nFileReqTime)
			{	
				nFileReqTime--;
				break;
			}
			if(nFileReqCount++ > 3)//�ļ����������
			{
				nFileReqCount = 0;
				nLinkStep = 0;
				break;
			}
			nLinkStep = 2;
			TftpDownQ(flag,update_GetFilename());
			break;
		case 2:
			if((nFotaTime % 5) == 0)//����ʱ�ط�����
			{
				if(update_GetTFTPFlag())
				{
					if(nTftpCount++ > 3)//Ӧ�����ݰ��������
					{
						 nTftpCount = 0;
						 nLinkStep  = 0;
						 break;
					}
					modem_send_gprsData((char*)ackbuf,"OK",30,4);
				}
				else
				{
					nLinkStep = 1;
					nFileReqTime = 5;
				}
			}
			break;
		default:
			break;
	}
}

//Զ���������ֵ�ʵ��
void TftpTarsk(unsigned char flag,unsigned char *inbuf,unsigned short updatlen)	   
{
	unsigned char *pStrat=NULL;
	unsigned char *pEnd=NULL;
	unsigned char updatbuf[5]={0};
	unsigned short acknum=0;
	unsigned short pTftp=0;
	unsigned int datlen=0;
	static unsigned int address=0;
	static unsigned short RqNumb=0;
	static unsigned short fwBags=0;
	static unsigned short crc=0;

	pTftp = inbuf[0];

	switch(pTftp)
	{
		case TFTPRRQ:
			break;
		case TFTPWRQ:
			break;
		case TFTPDATA:			
			datlen = updatlen - 4;
			acknum = (unsigned short)((inbuf[3]<<8) | inbuf[2]); //���ݰ����   
			if(acknum == 0x01) //��һ�����ݰ� 
			{
				if(nRestartTftp == 1)//�ϵ�����
				{
					LOG("[fota] restart request\r\n");
					nRestartTftp  = 0;
					nFotaTime     = 59; //�յ����ݰ���λ��ʱ��
					update_ClearFileReq();
					update_SetTFTPFlag(1);//�ϵ㿪ʼ���� 

					modem_send_gprsData((char*)ackbuf,"OK",10,4);//��������ϵ�����ݰ�

					break;
				}
				RqNumb = 2;
				
				datlen = datlen - 16;
				memcpy_t(FW.INFO_Buf, (inbuf+4), 16);
				pStrat = &FW.INFO_Buf[0];//����crc
				pEnd   = &FW.INFO_Buf[13];
				crc    = CRC_calc(0, pStrat, pEnd);

				if(crc != FW.INFO.head_crc)
				{
					LOG("[fota] head CRC err[%04x  %04x]\r\n", FW.INFO.head_crc, crc);
					break;
				}

				fwBags = ((FW.INFO.fw_length+16)%512) ? ((FW.INFO.fw_length+16)/512 + 1) : ((FW.INFO.fw_length+16)/512);
				LOG("[fota] fw bags[%d]\r\n", fwBags);
				
				update_SetTFTPFlag(1);//��ʼ����
				flash_fota_erase();
				
				flash_fota_write(PAGE_UPDATE_START,datlen,(inbuf+4+16));
				address = PAGE_UPDATE_START + datlen;
				fwBags--;

				pStrat = (inbuf+4+16);//����crc
				pEnd   = (inbuf+4+16+datlen-1);
				crc    = CRC_calc(0, pStrat, pEnd);
			}
			else//�м����ݰ� 
			{
				if(RqNumb == acknum)
				{
					RqNumb++;
					flash_fota_write(address,datlen,(inbuf+4));
					address = address + datlen;
					fwBags--;

					pStrat = (inbuf+4);//����crc
					pEnd   = (inbuf+4+datlen-1);
					crc    = CRC_calc(crc, pStrat, pEnd);
				}
				else//����ʧ��
				{
					break;
				}
			}
			nFotaTime = 59; //�յ����ݰ���λ��ʱ��
			update_ClearFileReq();
			
			ackbuf[1] = 0x00;				
			ackbuf[0] = TFTPACK;			//Ӧ��������
			ackbuf[3] = (unsigned char)(acknum>>8); 	//Ӧ���������
			ackbuf[2] = (unsigned char)(acknum>>0); 	//Ӧ���������
			if(fwBags == 0)//������ݰ�
			{
				datlen = address - PAGE_UPDATE_START;
				if(datlen != FW.INFO.fw_length)
				{
					LOG("[fota] datlen err[%d  %d]\r\n", FW.INFO.fw_length, datlen);
					OS_Timer_Stop(GSM_TIMER_ID);
					modem_send_disconnect(0);
					sys_SetUpdateFlag(2);
					memset_t(&aw_update_info,0,sizeof(aw_update_info));
					break;
				}

				if(crc != FW.INFO.fw_crc)
				{
					LOG("[fota] fw CRC err[%04x  %04x]\r\n", FW.INFO.fw_crc, crc);

					OS_Timer_Stop(GSM_TIMER_ID);
					modem_send_disconnect(0);

					sys_SetUpdateFlag(2);
					memset_t(&aw_update_info,0,sizeof(aw_update_info));
					break;
				}

				updatbuf[0] = 1;//�����ɹ���־
				updatbuf[1] = 1;
				updatbuf[2] = (unsigned char)(acknum>>0);
				updatbuf[3] = (unsigned char)(acknum>>8);
				flash_Write(PAGE_UPDATE_END,4,updatbuf);

				gps_valid_save(&g_gpsLastinfo);
				sys_saveRuning();//������ɱ��浱ǰλ����Ϣ

				LOG("[fota] load app finish\r\n\r\n");

				sys_SoftReset();
			}
			else
			{
				modem_send_gprsData((char*)ackbuf,"OK",10,4);
			}
			break;
		default:
			break;
	}
}

void TftpDownQ(unsigned char flag,void *file)
{
	unsigned char buf[30]={0};	
	unsigned char *ps=NULL;
	unsigned short len;
	
	buf[0] = 0x01;
	buf[1] = 0;
	ps = buf;
	ps += 2;
	sprintf((char*)ps,"%s",(char*)file);
	ps += strlen_t((char*)ps)+1 ;
	sprintf((char*)ps,"%s","octet");	
	len = 2 + strlen_t(file) + 1+5 ;

	modem_send_gprsData((char*)buf,"OK",10,len);

}

//����Զ������IP
unsigned char update_SetIp(unsigned char *ip, unsigned short len)
{	
	if ((ip==NULL) || (len>sizeof(aw_update_info.Ip)))
	{
		return 0;
	}
	
	memset_t((char *)aw_update_info.Ip,0,sizeof(aw_update_info));
	memcpy_t(aw_update_info.Ip,ip,len);
	
	return 1;
}
   
//����Զ�������˿�
unsigned char update_SetPort(unsigned short port)
{
	if (port == 0)
	{
		return 0;
	}
	
	aw_update_info.Port = port;
	
	return 1;
}

//����Զ�������ļ���
unsigned char update_SetFilename(unsigned char *filename)
{
	if ((filename==NULL) || (strlen_t((const char *)filename)>sizeof(aw_update_info.FileName)))
	{
		return 0;
	}
	
	memset_t((char *)aw_update_info.FileName,0,sizeof(aw_update_info.FileName));
	memcpy_t(aw_update_info.FileName,filename,strlen_t((const char *)filename));
	
	return 1;
}

//��ȡԶ��������IP
unsigned char *update_GetIp(void)
{
	return  aw_update_info.Ip;
}

//��ȡԶ�������Ķ˿�
unsigned short update_GetPort(void)
{
   	return aw_update_info.Port;
}

//��ȡԶ���������ļ���
unsigned char *update_GetFilename(void)
{
  	return aw_update_info.FileName;
}
//����Զ��������־
void update_SetUpdateFlag(unsigned char flag)
{
	aw_update_info.bupdate = flag;
	nFotaTime = 59;
	nLinkStep = 0;
	nRestartTftp = 0;
	nFotaTimeOutStart = 0;
	nFileReqTime = 0;
	nFileReqCount = 0;
	nTftpCount = 0;
}
//��ȡԶ��������־
unsigned char update_GetUpdateFlag(void)
{
	return aw_update_info.bupdate;
}
//����tftp��־
void update_SetTFTPFlag(unsigned char flag)
{
	tftpctr.UpDataSt = flag;
}
//��ȡtftp��־
unsigned char update_GetTFTPFlag(void)
{
	return tftpctr.UpDataSt;
}
//�ļ������������
void update_ClearFileReq(void)
{
	nFileReqTime = 0;
	nFileReqCount = 0;
	nTftpCount = 0;
}
//��ȡ�ϴ�gprs state
void update_GetLastState(unsigned char state)
{
	(void)gLastGprsState;
	gLastGprsState = state;
}
//��������ʧ��/��ʱ��������
void update_Reconnect(void)
{
	nFotaTime = 59;
	nLinkStep = 0;
	nRestartTftp = 0;
	nFotaTimeOutStart = 0;
	nFileReqTime = 0;
	nFileReqCount = 0;
	nTftpCount = 0;
}

