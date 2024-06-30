#include "includes.h"

/* ���о�� */
xQueueHandle xGsmQueueId;

static ackmodem_info AckModem;
static unsigned char ModemAtErr=0;
static unsigned char ModemInit =0;

static unsigned char gModemRecvBuf[MAX_GSM_UART_RCV_LEN];
static unsigned short gModemRecvLen;

g_wait_rsp g_rsp;
g_gprs g_modem_runtime;
g_queue_info g_gsmqueue;

//ģ���ʼ��
void modem_init(void)
{
	modem_powerOn();
	modem_uartInit();
	modem_AckModemInit();
	modem_clearModBuf();
	modem_clearAtBuf();
	modem_ClearAtErrCnt();

	memset_t((char*)&g_modem_runtime,0,sizeof(g_modem_runtime));
	memset_t((char*)&g_gsmqueue, 0, sizeof(g_gsmqueue));
	OS_SendMessage(xGsmQueueId, MSG_GSM_POW_ON, 0, 0);
	g_flag.bit.openIP2Flag = 1;
}

//ģ�鴮�ڳ�ʼ��
void modem_uartInit(void)
{
	uart2_init(115200);
}

//ģ���ϵ�
void modem_powerOn(void)
{
	gpio_Gsm_powerCtrl(1);
}

//ģ���ؿ���
void modem_reset(void)
{
	//GSM���������
	gpio_dir(PB10, 1);
	gpio_set(PB10, 0);
	gpio_dir(PB11, 1);
	gpio_set(PB11, 0);
	
	gpio_Gsm_powerCtrl(0);
	OS_TaskDelayMs(8000);
	gpio_Gsm_powerCtrl(1);
	OS_TaskDelayMs(500);

	//�������³�ʼ��
	modem_uartInit();
}

//���ȿ���
void modem_SetSpk(unsigned char ctl)
{
	
}

//ģ�鷢��һ���ֽ�
void modem_sendChar(unsigned char ch)
{
	GsmSend_Lock(xSendMutex);
    sendchar2(ch);
	GsmSend_UnLock(xSendMutex);
}

//ģ�鷢�Ͷ���ֽ�
void modem_sendStr(unsigned char *p,int len)
{
	GsmSend_Lock(xSendMutex);
	print2(p,len);
	GsmSend_UnLock(xSendMutex);
}

//ģ��д��QUEUE����
void modem_recv(unsigned char *data, unsigned short len)
{
	RingBuffer_InsertMult(&GSMModemRINGBUFF, (unsigned char *)data, len);
	RingBuffer_InsertMult(&GSMAtRINGBUFF, (unsigned char *)data, len);

	OS_Timer_Start(xMainQueueId,MSG_MAIN_GSM_RECV_REQ,RingBuffer_GetCount(&GSMModemRINGBUFF),0,GSM_RECV_TIMER_ID,2,FALSE);
}

//ģ����ջ�����
void modem_clearModBuf(void)
{
	RingBuffer_Clear(&GSMModemRINGBUFF);
}
void modem_clearAtBuf(void)
{
    RingBuffer_Clear(&GSMAtRINGBUFF);
}

//ģ���ȡ����������
unsigned short modem_getModBufLen(void)
{
	return RingBuffer_GetCount(&GSMModemRINGBUFF);
}
unsigned short modem_getAtBufLen(void)
{
	return RingBuffer_GetCount(&GSMAtRINGBUFF);
}

//ģ���ȡָ���ַ
unsigned char *modem_getAtBuf(void)
{
	return RingBuffer_GetAddr(&GSMAtRINGBUFF);
}

//ģ���ȡһ���ֽ�
unsigned char modem_getModChar(unsigned char *ch)
{
	return RingBuffer_Pop(&GSMModemRINGBUFF, (unsigned char *)ch);
}

//GSM��ȡ����ֽ�
unsigned char modem_getModStr(unsigned char * pdst,unsigned short n)
{
	return RingBuffer_PopMult(&GSMModemRINGBUFF, (unsigned char *)pdst, n);
}

//��շ���AT����ʧ�ܴ���
void modem_ClearAtErrCnt(void)
{
   	ModemAtErr = 0;
}

//��ȡCPIN״̬
unsigned char modem_GetCpinStatus(void)
{
	return g_flag.bit.cpinReadyFlag;
}

//����IMEI
void modem_SetImei(unsigned char *buf)
{
	if ((buf==NULL) || ((strlen_t((const char *)buf)>sizeof(g_modem_runtime.simei))))
    {		
         return ;
    }
	memset_t(g_modem_runtime.simei,0,sizeof(g_modem_runtime.simei));
	memcpy_t(g_modem_runtime.simei,buf,strlen_t((const char *)buf));
}
//��ȡIMEI
unsigned char *modem_GetImei(void)
{
	return g_modem_runtime.simei;
}

//����CCID
void modem_SetCcid(unsigned char *buf)
{
	unsigned char i;
	unsigned char iccid[24] = {0};
	
	if ((buf==NULL) || ((strlen_t((const char *)buf)>sizeof(g_modem_runtime.sCcid))))
    {		
         return ;
    }
	for(i=0; i<strlen_t((const char *)buf); i++)
	{
		iccid[i] = Toupper(buf[i]);
	}
	memset_t(g_modem_runtime.sCcid,0,sizeof(g_modem_runtime.sCcid));
	memcpy_t(g_modem_runtime.sCcid,iccid,strlen_t((const char *)iccid));
}
//��ȡCCID
unsigned char *modem_GetCcid(void)
{
	return g_modem_runtime.sCcid;
}

//����COPS
void modem_SetCops(unsigned char *buf)
{
	if ((buf==NULL) || ((strlen_t((const char *)buf)>sizeof(g_modem_runtime.sCops))))
    {		
         return ;
    }
	memset_t(g_modem_runtime.sCops,0,sizeof(g_modem_runtime.sCops));
	memcpy_t(g_modem_runtime.sCops,buf,strlen_t((const char *)buf));
}
//��ȡCOPS
unsigned char *modem_GetCops(void)
{
	return g_modem_runtime.sCops;
}
//���COPS
void modem_ClearCops(void)
{
	memset_t(g_modem_runtime.sCops,0,sizeof(g_modem_runtime.sCops));
}

//����CGREGֵ
void modem_SetCgreg(unsigned char cgreg)
{
    g_modem_runtime.nCgreg = cgreg;   
}
//��ȡCGREGֵ
unsigned char modem_GetCgreg(void)
{
    return g_modem_runtime.nCgreg;
}
//����CSQֵ
void modem_SetCsq(unsigned char csq)
{
	g_modem_runtime.nCsq = csq;   
}
//��ȡCSQֵ
unsigned char modem_GetCsq(void)
{
    return g_modem_runtime.nCsq;
}
//����MCC
void modem_SetMcc(unsigned short mcc)
{
	g_modem_runtime.nMcc = mcc;   
}
//��ȡMCC
unsigned short modem_GetMcc(void)
{
    return g_modem_runtime.nMcc;
}
//����MNC
void modem_SetMnc(unsigned short mnc)
{
	g_modem_runtime.nMnc = mnc;   
}
//��ȡMNC
unsigned short modem_GetMnc(void)
{
    return g_modem_runtime.nMnc;
}
//����LAC
void modem_SetLac(unsigned int lac)
{
	g_modem_runtime.nLac = lac;   
}
//��ȡLAC
unsigned int modem_GetLac(void)
{
    return g_modem_runtime.nLac;
}
//����CELLID
void modem_SetCellid(unsigned int cellid)
{
	g_modem_runtime.nCellid = cellid;   
}
//��ȡCELLID
unsigned int modem_GetCellid(void)
{
    return g_modem_runtime.nCellid;
}
//����ȥ�绹������
void modem_SetCallState(unsigned char state)
{	
	g_modem_runtime.nCallState = state;
}
//���״̬ (���� ȥ��)
void modem_ClearCallState(void)
{
	g_modem_runtime.nCallState = CALL_NULL;  
}
//��ȡ״̬ (����  ȥ��)
unsigned char modem_GetCallState(void)
{
	return  g_modem_runtime.nCallState;
}
//�����������
void modem_SetCallinNo(unsigned char *buf)
{
    if ((buf==NULL)  || (strlen_t((const char *)buf) > sizeof(g_modem_runtime.nCallInNo)))
    {
        return;
    }
    memset_t((char *)g_modem_runtime.nCallInNo,0,sizeof(g_modem_runtime.nCallInNo));
    strcpy_t((char *)g_modem_runtime.nCallInNo,(char *)buf); 
}

//��ȡ�������
unsigned char *modem_GetCallinNo(void)
{
    return  g_modem_runtime.nCallInNo;
}
//���ñ��淢�����ݱ�־/����У����/ID
void modem_SetAckModemInfo(ackmodem_info *info)
{
	memset_t((char*)&AckModem, 0, sizeof(ackmodem_info));
	memcpy_t((char*)&AckModem, (char*)info, sizeof(ackmodem_info));
}
//��ȡ�������ݱ�־/����У����/ID
ackmodem_info* modem_GetAckModemInfo(void)
{
	return &AckModem;
}
//Ӧ�����ʱ�ṹ���ʼ��
void modem_AckModemInit(void)
{
	memset_t((char*)&AckModem, 0, sizeof(AckModem));
}
//����Ӧ�����
void modem_SetAckModem(unsigned char ack)
{
	AckModem.ack = ack;	
}
//��ȡӦ�����
unsigned char modem_GetAckModem(void)
{
	return AckModem.ack;	
}
//��ʼ��Ӧ��ģʽ��������
void modem_SetAckSendInit(unsigned char cmd1,unsigned char cmd2)
{
	AckModem.ack = 1;
	AckModem.cmd = (cmd1<<8)|cmd2;
	AckModem.sendcnt = 0;
}
//Ӧ��ģʽ���ݷ��ʹ���
void modem_SendAckModeData(unsigned char *data, unsigned short datalen)
{	
	if(AckModem.sendcnt > 3)
	{
		AckModem.sendcnt = 0;
		modem_gsm_disconnect_ip1();
	}
	else
	{
		AckModem.sendcnt++;
		modem_send_gprsData((char*)data,"OK",30,datalen);
	}
}

//ģ�鷢��AT�ַ���
void modem_send_AtStr(char* str)
{
	modem_sendStr((unsigned char*)str,strlen_t(str));
}

unsigned char modem_send_Atcmd(char* Cmd,char* Erc,unsigned short Time/*��λ100ms*/)
{
	GsmSend_Lock(xSendMutex);
	LOG("[gsm] %s\r\n",Cmd);
	modem_clearAtBuf();
	GsmSend_UnLock(xSendMutex);
	
	modem_send_AtStr(Cmd);
	modem_send_AtStr("\r\n");
	
	return modem_seek_AtAck(Erc,Time);
}

unsigned char modem_send_gprsData(char* data,char* Erc,unsigned short Time,unsigned short len)
{
	unsigned char buf[128]={0};
	unsigned char ack[16]={0};

	IF_CONDITION_A7600C
	{
		LOG("[gsm] ip1 send:");
		LOGX((unsigned char *)data,len);

		if(modem_GetState() == GSM_FOTA_STATE)
		{
			sprintf((char *)buf,"AT+CIPSEND=1,%d,%s,%d",(unsigned short)len,update_GetIp(),update_GetPort());
			strcpy_t((char *)ack,">");
			modem_send_Atcmd((char*)buf,(char*)ack,30);

			modem_sendStr((unsigned char *)data, len);
			modem_send_AtStr("\r\n");
		}
		else if(sys_GetLinkType() == 1)
		{
			sprintf((char *)buf,"AT+CIPSEND=1,%d",(unsigned short)len);
			strcpy_t((char *)ack,">");
			modem_send_Atcmd((char*)buf,(char*)ack,30);

			modem_sendStr((unsigned char *)data, len);
			modem_send_AtStr("\r\n");
		}
		else
		{
			sprintf((char *)buf,"AT+CIPSEND=1,%d,%s%s,%d",(unsigned short)len,sys_GetIp(),sys_GetUrl(),sys_GetPort());
			strcpy_t((char *)ack,">");
			modem_send_Atcmd((char*)buf,(char*)ack,30);

			modem_sendStr((unsigned char *)data, len);
			modem_send_AtStr("\r\n");
		}
		modem_clearAtBuf();
		
		if(modem_seek_AtAck("+CIPSEND:",Time) != RET_AT_SUCCESS)//��ʱ��error
		{
			LOG("[gsm] send return error\r\n");
			if(modem_GetState() == GSM_FOTA_STATE)
			{
				update_Reconnect();
			}
			else
			{
	        	modem_gsm_disconnect_ip1();
			}
			
	        return 0;
	    }
	}
	IF_CONDITION_EC20
	{
		LOG("[gsm] ip1 send:");
		LOGX((unsigned char *)data,len);
	
		sprintf((char *)buf,"AT+QISEND=1,%d",(unsigned short)len);
		strcpy_t((char *)ack,">");
		modem_send_Atcmd((char*)buf,(char*)ack,30);
		
		modem_sendStr((unsigned char *)data, len);
		modem_send_AtStr("\r\n");
		
		modem_clearAtBuf();
		
		if(modem_seek_AtAck(Erc,Time) != RET_AT_SUCCESS)//��ʱ��error
		{
			LOG("[gsm] send return error\r\n");
        	if(modem_GetState() == GSM_FOTA_STATE)
			{
				update_Reconnect();
			}
			else
			{
        		modem_gsm_disconnect_ip1();
			}
			
	        return 0;
	    }
	}

	return 1;
}

unsigned char modem_seek_AtAck(char* ack,unsigned short time)
{
#define GSMERRMAXCNT      10             //��ʱ��������

	GsmSend_Lock(xSendMutex);
	memset_t(&g_rsp, 0, sizeof(g_rsp));
	//Ĭ�Ϸ���ǰAT��ʱ
	ModemAtErr++;
	g_rsp.ret_value = RET_AT_TIMEOUT;
	strcpy_t((char *)g_rsp.cmp_ack, ack);
	GsmWait_SemTake(xGsmSem, (time*100));
	GsmSend_UnLock(xSendMutex);

	if(g_rsp.ret_value == RET_AT_TIMEOUT)
	{
		memset_t(g_rsp.cmp_ack, 0, sizeof(g_rsp.cmp_ack));
	}
	if(ModemAtErr >= GSMERRMAXCNT)
	{
		ModemAtErr = 0;
		LOG("[gsm] no response timeout...reset\r\n");
		OS_Timer_Stop(GSM_TIMER_ID);
		OS_SendMessage(xGsmQueueId, MSG_GSM_POW_ON, 0, 0);
	}
	return g_rsp.ret_value;
}

void modem_response(unsigned char* rsp, unsigned short len)
{
	if(strlen_t((char *)g_rsp.cmp_ack))
	{
		if(memcmp_t(g_rsp.cmp_ack, "+QISTATE:", strlen_t("+QISTATE:")) == 0)
		{
			int socket_state;
			
			sscanf((char *)rsp, "%*[^,],%*[^,],%*[^,],%*[^,],%*[^,],%d[^,]",&socket_state);
			if(socket_state == 2)
			{
				g_rsp.ret_value = RET_AT_SUCCESS;
			}
			else
			{
				g_rsp.ret_value = RET_AT_ERROR;
			}
			LOG("%s",rsp);
			/*
			0 "Initial" 	connection has not been established
			1 "Opening" 	client is connecting or server is trying to listen
			2 "Connected" 	client/incoming connection has been established
			3 "Listening" 	server is listening
			4 "Closing" 	connection is closing
			*/
			LOG("[socket] state: %d\r\n",socket_state);
			ModemAtErr = 0;
			memset_t(g_rsp.cmp_ack, 0, sizeof(g_rsp.cmp_ack));
			GsmWait_SemGive(xGsmSem);
			
			return;
		}
		if(mystrncmp((char*)rsp, (char*)g_rsp.cmp_ack, len, strlen_t((char *)g_rsp.cmp_ack)) == 0)
		{
			LOG("%s",rsp);
			ModemAtErr = 0;
			g_rsp.ret_value = RET_AT_SUCCESS;
			memset_t(g_rsp.cmp_ack, 0, sizeof(g_rsp.cmp_ack));
			GsmWait_SemGive(xGsmSem);
		}
		else if(mystrncmp((char*)rsp, "ERROR", len, strlen_t("ERROR")) == 0)
		{
			LOG("%s",rsp);
			g_rsp.ret_value = RET_AT_ERROR;
			memset_t(g_rsp.cmp_ack, 0, sizeof(g_rsp.cmp_ack));
			GsmWait_SemGive(xGsmSem);
		}
		else if(mystrncmp((char*)rsp, "RING", len, strlen_t("RING")) == 0)
		{
			if(modem_GetCallState() == CALL_RINGING)
			{
				LOG("%s",rsp);
				ModemAtErr = 0;
				g_rsp.ret_value = RET_AT_RING;
				memset_t(g_rsp.cmp_ack, 0, sizeof(g_rsp.cmp_ack));
				GsmWait_SemGive(xGsmSem);
			}
		}
		else
		{
			LOG("%s",rsp);
		}
	}
}

//AT��ʼ��
g_at_info at_init_tab_quectel[] = 
{
	{10,"AT","OK"},
	{10,"ATE0","OK"},
//	{10,"AT+CMEE=2","OK"},
	{10,"ATE0","OK"}
};

void modem_sendAT_init(unsigned char *cmd,unsigned char* ack,unsigned short time)
{
	unsigned char n = 4;//��ʱ��error�ط�����
	
	while(n)
	{
		n--;
		if(modem_send_Atcmd((char *)cmd, (char *)ack,time) == RET_AT_SUCCESS)
		{
			break;
		}
	}
}
void modem_gsm_init(void)
{
	static unsigned char s_modemResetFlag=0;
	unsigned char i=0,count=0;

	if(s_modemResetFlag)//�����ϵ�ģ��
	{
		LOG("[gsm] reset...\r\n");
		ModemInit = 0;
		modem_SetState(GSM_INIT_STATE);
		modem_clearModBuf();
		modem_clearAtBuf();
		modem_clear_flagCnt();
		modem_reset();
		xQueueReset(xGsmQueueId);
	}
	
	count = sizeof(at_init_tab_quectel)/sizeof(g_at_info);
	for(i=0; i<count; i++)
	{
		modem_sendAT_init((unsigned char *)at_init_tab_quectel[i].pcmd,(unsigned char *)at_init_tab_quectel[i].erc,at_init_tab_quectel[i].times);
	}
	
	s_modemResetFlag = 1;
	modem_VersionCheck();
}

//ATI
void modem_VersionCheck(void)
{
	char *buf=(char*)modem_getAtBuf();

	if(g_modem_runtime.nModemType != GSM_MODEM_NULL)
	{
		OS_SendMessage(xGsmQueueId, MSG_GSM_CGREG, 0, 0);
		
		return;
	}
	
	if(modem_send_Atcmd("ATI","OK",30) != RET_AT_SUCCESS)
	{
		OS_SendMessage(xGsmQueueId, MSG_GSM_POW_ON, 0, 0);
		
		return;
	}

	if(strstr_t(buf, "EC800M") || strstr_t(buf, "BC76") ||strstr_t(buf, "EC2"))//EC20 EC21 EC25
	{
		g_modem_runtime.nModemType = GSM_MODEM_QUECTEL_EC20;
	}
	else if(strstr_t(buf, "L511C"))
	{
		g_modem_runtime.nModemType = GSM_MODEM_LYNQ_L511C;
	}
	else if(strstr_t(buf, "7600") || strstr_t(buf, "7670"))
	{
		g_modem_runtime.nModemType = GSM_MODEM_SIMCOM_A7600C;
	}
	else
	{
		OS_SendMessage(xGsmQueueId, MSG_GSM_POW_ON, 0, 0);
		
		return;
	}

	OS_SendMessage(xGsmQueueId, MSG_GSM_CGREG, 0, 0);
	
	return;	
}

//IMEI
void modem_ImeiCheck(void)
{
	unsigned char i=0;
	char *p=NULL;

	char *buf=(char*)modem_getAtBuf();

	if(strlen_t((char *)modem_GetImei()))
	{
		return;
	}
	if(modem_send_Atcmd("AT+CGSN","OK",10) != RET_AT_SUCCESS)
	{
		return;
	}

	IF_CONDITION_LYNQ
	{
		for(i=0; i<24; i++)
		{
			if((buf[i] == '8')&&(buf[i+1] == '6'))
			{
				break;
			}
		}
		if(i >= 24)
		{
			return;
		}
		p = strchr_t(&buf[i],'\r');
		if(p == NULL)
		{
			return;
		}
		*p = 0;

		modem_SetImei((unsigned char *)&buf[i]);
		LOG("[gsm] imei: %s\r\n",modem_GetImei());
	}
	IF_CONDITION_A7600C
	{
		for(i=0; i<24; i++)
		{
			if((buf[i] == '8')&&(buf[i+1] == '6'))
			{
				break;
			}
		}
		if(i >= 24)
		{
			return;
		}
		p = strchr_t(&buf[i],'\r');
		if(p == NULL)
		{
			return;
		}
		*p = 0;

		modem_SetImei((unsigned char *)&buf[i]);
		LOG("[gsm] imei: %s\r\n",modem_GetImei());
	}
	IF_CONDITION_EC20
	{
		for(i=0; i<24; i++)
		{
			if((buf[i] == '8')&&(buf[i+1] == '6'))
			{
				break;
			}
		}
		if(i >= 24)
		{
			return;
		}
		p = strchr_t(&buf[i],'\r');
		if(p == NULL)
		{
			return;
		}
		*p = 0;

		modem_SetImei((unsigned char *)&buf[i]);
		LOG("[gsm] imei: %s\r\n",modem_GetImei());
	}
	
	return;	
}

//CCID
void modem_CcidCheck(void)
{
	unsigned char i=0;
	char *p=NULL;
	char *buf=(char*)modem_getAtBuf();
	unsigned char CCID[32]={0};

	IF_CONDITION_A7600C
	{
		if(modem_send_Atcmd("AT+CICCID","OK",10) != RET_AT_SUCCESS)
		{
			return;
		}

		p = strstr_t(buf,"+ICCID: ");
		if(p == NULL)
		{
			return ;
		}
		p += strlen_t("+ICCID: ");
		
		while((i < 32) && (*p != '\r'))
		{
			CCID[i++] = *p++;
		}

		ModemInit = 1;
		g_flag.bit.cpinReadyFlag = 1;
		modem_SetCcid(CCID);

		modem_send_Atcmd("AT+CMGF=0","OK",10);
		modem_send_Atcmd("AT+CNMI=2,1","OK",10);
	}

	IF_CONDITION_EC2x
	{
		if(modem_send_Atcmd("AT+QCCID","OK",10) != RET_AT_SUCCESS)
		{
			return;
		}

		p = strstr_t(buf,"+QCCID: ");
		if(p == NULL)
		{
			return ;
		}
		p += strlen_t("+QCCID: ");
		
		while((i < 32) && (*p != '\r'))
		{
			CCID[i++] = *p++;
		}

		ModemInit = 1;
		g_flag.bit.cpinReadyFlag = 1;
		modem_SetCcid(CCID);

		modem_send_Atcmd("AT+CMGF=0","OK",10);
		modem_send_Atcmd("AT+CNMI=2,1","OK",10);
	}
	
	return;	
}

//CSQֵ�Ĵ���	
void modem_CsqCheck(void)
{
	char *p=NULL,*pm=NULL;
	char *buf=(char *)modem_getAtBuf();
	unsigned char csq=0;

	if(modem_send_Atcmd("AT+CSQ","OK",10) != RET_AT_SUCCESS)
	{
		modem_SetCsq(0);
		return;
	}	 

	p = strstr_t(buf,"+CSQ:");
	if(p == NULL)
	{
		return ;
	}

	p += 6;	
	pm = strchr_t(p,',');
	if(pm == NULL)
	{
		return ;
	}
	
	*pm = 0x00;
	csq = (unsigned char)myatoi(p);
	if (csq > 32)
	{
		csq = 0;
	}
	modem_SetCsq(csq); 
	
	return;
}

//CGREGֵ�Ĵ���
void modem_CgregCheck(void)
{
	char *p=NULL;
	char *buf=(char*)modem_getAtBuf();
	unsigned char cgreg=0;

	IF_CONDITION_LYNQ
	{
		if(modem_send_Atcmd("AT+CEREG?","OK",10) != RET_AT_SUCCESS)
		{
			modem_SetCgreg(0);
			return;
		}
		
		p = strstr_t(buf,"+CEREG:");
		if(p == NULL)
		{
			return ;
		}

		p = strchr_t(p,',');
		if(p == NULL)
		{
			return ;
		}

		p++;
		cgreg = (unsigned char)myatoi(p);
		if (cgreg > 5)
		{
			cgreg = 0;
		}
		modem_SetCgreg(cgreg);

	}
	else
	{
		if(modem_send_Atcmd("AT+CGREG?","OK",10) != RET_AT_SUCCESS)
		{
			modem_SetCgreg(0);
			return;
		}
		
		p = strstr_t(buf,"+CGREG:");
		if(p == NULL)
		{
			return ;
		}

		p = strchr_t(p,',');
		if(p == NULL)
		{
			return ;
		}

		p++;
		cgreg = (unsigned char)myatoi(p);
		if (cgreg > 5)
		{
			cgreg = 0;
		}
		modem_SetCgreg(cgreg);
	}
	
  
	return;
}

//COPSֵ�Ĵ���
void modem_CopsCheck(void)
{
	char *ps=NULL;
	char *pe=NULL;
	char *buf=(char*)modem_getAtBuf();
	char cops[32]={0};

	if(modem_send_Atcmd("AT+COPS?","OK",10) != RET_AT_SUCCESS)
	{
		return;
	}
	
	ps = strstr_t(buf,"+COPS:");
	if(ps == NULL)
	{
		return ;
	}

	ps = strchr_t(ps,'"');
	if(ps == NULL)
	{
		modem_ClearCops();
		return ;
	}

	ps ++;
	pe = strchr_t(ps,'"');
	if(pe == NULL)
	{
		modem_ClearCops();
		return ;
	}
	pe[0] = 0;
	if(pe - ps >= 32)
	{
		return;
	}
	
	strcpy_t(cops, ps);
	modem_SetCops((unsigned char *)cops);
	LOG("[gsm] cops: %s\r\n", modem_GetCops());
	
	return;
}

//CCLK
void modem_CclkCheck(void)
{
	char *p=NULL;
	char *buf=(char*)modem_getAtBuf();
	int y,mo,d,h,mi,s;
	TIME rtc;
	static unsigned char first_valid=0;

	if(first_valid == 1)
	{
		return;
	}
	if(modem_send_Atcmd("AT+CCLK?","OK",10) != RET_AT_SUCCESS)
	{
		return;
	}

	//+CCLK: "20/12/03,15:03:02"
	p = strstr_t(buf,"+CCLK:");
	if(p == NULL)
	{
		return ;
	}

	sscanf((char *)p, "%*[^\"]\"%d/%d/%d,%d:%d:%d",&y,&mo,&d,&h,&mi,&s);

	rtc.year  = 2000 + y;
	rtc.month = mo;
	rtc.day   = d;
	rtc.hour  = h;
	rtc.minute= mi;
	rtc.second= s;
	if(sys_state_get_gpsloc() == 0)
	{
		RTC_Set_Time(&rtc);
	}

	first_valid = 1;
	
	return;
}

//SIMCOM
void modem_CpsiCheck(void)
{
	IF_CONDITION_A7600C
	{
		char *p=NULL;
		char *buf=(char *)modem_getAtBuf();
		unsigned char i=0;

		if(modem_send_Atcmd("AT+CPSI?","OK",10) != RET_AT_SUCCESS)
		{
			return;
		}	 

		p = strstr_t(buf,"+CPSI: ");
		if(p == NULL)
		{
			return ;
		}
		p += strlen_t("+CPSI: ");
		
		memset_t(g_modem_runtime.sNetwork, 0, sizeof(g_modem_runtime.sNetwork));
		while((i < 32) && (*p != ','))
		{
			g_modem_runtime.sNetwork[i++] = *p++;
		}
	}
}


//QINISTAT��ѯSIM��״̬
/*
(e.g. 7=1+2+4 means CPIN READY&SMS DONE&PHB DONE)

0 Initial state
1 CPIN READY. Operation like lock/unlock PIN is allowed
2 SMS initialization complete
4 Phonebook initialization complete
*/
void modem_QinistatCheck(void)
{
	char *p=NULL;
	char *buf=(char*)modem_getAtBuf();
	unsigned char stat=0;

	IF_CONDITION_EC2x
	{
		if(modem_send_Atcmd("AT+QINISTAT","OK",10) != RET_AT_SUCCESS)
		{
			return;
		}
		
		p = strstr_t(buf,"+QINISTAT:");
		if(p == NULL)
		{
			return ;
		}

		p += strlen_t("+QINISTAT:");
		p ++;
		
		stat = (unsigned char)myatoi(p);
		if(stat >= 1)
		{
			if(ModemInit == 0)
			{
				modem_CcidCheck();
			}
		}
	}
	else
	{
		if(ModemInit == 0)
		{
			modem_CcidCheck();
		}
	}
	   
	return;
}

//Set URC output port
void modem_UrcportCheck(void)
{

	static unsigned char success_flag = 0;

	IF_CONDITION_EC2x
	{
		if(success_flag == 1)
		{
			return;
		}
		if(modem_send_Atcmd("AT+QURCCFG=\"urcport\",\"uart1\"","OK",10) == RET_AT_SUCCESS)
		{
			success_flag = 1;
		}
	}

	return;

}

//�ж������绹��ȥ��
unsigned char modem_CheckInOut(unsigned char* buf)
{
//+CLCC: 1,1,4,0,0,"13510830918",161         ����
//+CLCC: 1,0,2,0,0           ȥ��	
//+CLCC: 1,0,3,0,0           ��ͨ
//+CLCC: 1,0,0,0,0           ��ͨ

	unsigned char *p=NULL;
	unsigned char i=0;
	
	p = buf;
	for(i=0; i<2; i++)
	{
		p = (unsigned char *)strchr_t((const char *)p,',');
		if(p == NULL)
		{
			return CALL_NULL;
		}
		p++;
	}
	
	if(*p == '4')//����
	{
		return CALL_IN;
	}
	else if(*p == '2')//ȥ��
	{
		return CALL_OUT;
	}
	else if(*p == '3')//��ͨ
	{
		return CALL_OUT;
	}
	else if(*p == '0')//����
	{
		return CALL_CONNECT;
	}
	else
	{
		return CALL_NULL;
	}	 
}

//����MIC  (0-15) 
unsigned char modem_Set_Mic(unsigned char volume)
{ 
	(void)volume;

    return 0;
}
//��������  (0-7) 
unsigned char modem_Set_Volume(unsigned char volume)
{
    char buf[16]={0};

	if (volume > 7)
	{
		volume = 7;
	}
    
    sprintf((char *)buf,"AT+CLVL=%d",(int)volume);
	
    if(modem_send_Atcmd(buf,"OK",10) == RET_AT_SUCCESS)
    { 
        return 1;
    }
    return 0;
}
//�ҵ绰
unsigned char modem_HangPhone(void)
{    
	modem_send_Atcmd("ATH","OK",30);

	modem_SetSpk(0);//�ر�����
	
	return 1;
}
//�绰����������
void modem_NoiseCtl(void)
{	
	modem_Set_Mic(5);
	
	modem_send_Atcmd("AT+QAUDCH=1","OK",10);

	modem_send_Atcmd("AT+QSIDET=0","OK",10);
}

//�ӵ绰
unsigned char modem_AnwserPhone(void)
{
    if(modem_send_Atcmd("ATA","OK",10) == RET_AT_SUCCESS)
    {
		modem_NoiseCtl();

		modem_SetSpk(1);                  		//��������
		modem_Set_Volume(3);        			//����
		return 1;
    }
	
    return 0;
}

//��绰
//����   �绰   ����������ͨ�� (�������ý�����)   1  ����ͨ��  0 ����
unsigned char modem_CallPhone(unsigned char *data,unsigned char flag)
{                          
	char buf[20]={0};
	
	if ((data==NULL) || (strlen_t((const char *)data)>32))
	{
		return 0;
	}
	modem_NoiseCtl();

	memset_t((char *)buf,0,sizeof(buf));
	sprintf((char*)buf,"ATD%s;",data);
	
	if(modem_send_Atcmd(buf,"OK",10) == RET_AT_SUCCESS)
	{			
		if(flag == 1)
		{
			modem_SetSpk(1);           	//�������� 
			modem_Set_Volume(3);   		//����
		}
		return 1;
	}

	return 0;
}

//�����һ�
void modem_HandUpClear(void)
{
	modem_HangPhone();        //�ҵ绰
	
	modem_ClearCallState();   //��״̬  (����ȥ��)   
	
	OS_Timer_Stop(CALL_TIMER_ID);//�ر�call��ʱ��
	
	modem_send_disconnect(0); //��������
}

//�绰����
void modem_ClccCheck(void)
{
	char *p=NULL,*pt=NULL;
	char *buf=(char*)modem_getAtBuf();
	unsigned char ret=0;
	unsigned char tel_no[32]={0};
	unsigned char tel_len=0;

	if(modem_send_Atcmd("AT+CLCC","OK",30) != RET_AT_SUCCESS)
	{
		return;
	}
	
	p = strstr_t(buf,"+CLCC:");
	if(p == NULL)
	{
		modem_HandUpClear();
		
		return;
	}
	else
	{
		ret = modem_CheckInOut((unsigned char*)p);
		if(ret == CALL_IN)
		{
			//�������Ʋ���
			if((sys_GetCallCtrl() == CALLPHONE_FORBID_ATA)||(sys_GetCallCtrl() == CALLPHONE_FORBID_DIAL_ATA))
			{
				modem_HandUpClear();

				return;
			}
			if(modem_GetState() == GSM_CALL_STATE)
			{
				return;
			}
			if(modem_GetCallState() == CALL_IN)
			{
				return;
			}
			LOG("[call] in...\r\n");
			modem_SetState(GSM_CALL_STATE);
			modem_SetCallState(CALL_IN);
		}
		else if(ret == CALL_OUT)
		{
			if(modem_GetCallState() == CALL_OUT)
			{
				return;
			}
			//�������Ʋ���
			LOG("[call] out...\r\n");
			modem_SetState(GSM_CALL_STATE);
			modem_SetCallState(CALL_OUT);

			OS_Timer_Stop(GSM_TIMER_ID);		
			OS_Timer_Start(xGsmQueueId,MSG_GSM_CALL,0,0,CALL_TIMER_ID,500,TRUE);// 5s
			
			return;
		}
		else if(ret == CALL_CONNECT)
		{
			if(modem_GetCallState() == CALL_CONNECT)
			{
				return;
			}
			LOG("[call] connect...\r\n");
			modem_SetState(GSM_CALL_STATE);
			modem_SetCallState(CALL_CONNECT);
			
			return;
		}
		else
		{
			return;	
		}
		//��ȡ�绰����
		memset_t((char *)tel_no,0,sizeof(tel_no));
		p = strchr_t(p,'"');
		if(p == NULL)//δ��ͨ����
		{
			tel_len = strlen_t("δ֪");
		   	memcpy_t(tel_no,"δ֪",tel_len);
		}
		else
		{
			p++;
			pt = strchr_t(p,'"');
			if(pt == NULL)//����
			{
			   	tel_len = strlen_t("δ֪");
		   		memcpy_t(tel_no,"δ֪",tel_len);
			}
			else
			{
	            tel_len = pt-p;
				memcpy_t(tel_no,p,tel_len);//�绰����                         
			}
		}
		LOG("[call] phone:%s\r\n",tel_no);
		modem_SetCallinNo(tel_no);

		if(sys_GetAutoAnswerTime())
		{
			g_modem_runtime.nCallAtaDelay = sys_GetAutoAnswerTime();
			
			OS_Timer_Stop(GSM_TIMER_ID);		
			OS_Timer_Start(xGsmQueueId,MSG_GSM_CALL,0,0,CALL_TIMER_ID,500,TRUE);// 5s
		}
		else
		{
			modem_HandUpClear();
		}
	}

	return;
}

//call��ʱ��������
void modem_ata_delay(void)
{
	if(g_modem_runtime.nCallAtaDelay)
	{
		g_modem_runtime.nCallAtaDelay--;
		if(g_modem_runtime.nCallAtaDelay == 0)
		{
			OS_SendMessage(xGsmQueueId, MSG_GSM_CALL, MSG_GSM_ATA, 0);
		}
	}
}

//csq��cgreg��ѯ
void modem_gsm_cgreg(void)
{
	static unsigned short s_check_timeout=0;
	unsigned char cgreg=0, csq=0;

	modem_ImeiCheck();
	modem_UrcportCheck();
	modem_CsqCheck();
	modem_CgregCheck();

	modem_QinistatCheck();
	modem_CopsCheck();
	modem_CpsiCheck();
	
	cgreg = modem_GetCgreg();
	csq   = modem_GetCsq();

	if(((cgreg == 1)||(cgreg == 5)) &&  ((csq > 0) && (csq <= 31)) && strlen_t((char *)modem_GetCops()))//sim���Ѿ�ע�ᵽ����
	{
		s_check_timeout = 0;
		OS_Timer_Stop(GSM_TIMER_ID);
		OS_SendMessage(xGsmQueueId, MSG_GSM_PPP, 0, 0);
	}
	else
	{
		if(++s_check_timeout > 600)//��ѯ��ʱ,����ģ��
		{
			LOG("[gsm] cgreg timeout...reset\r\n");
			s_check_timeout = 0;
			g_flag.bit.MutexFlag = 0;
			OS_Timer_Stop(GSM_TIMER_ID);
			OS_SendMessage(xGsmQueueId, MSG_GSM_POW_ON, 0, 0);
		}
		else
		{
			if((s_check_timeout%50) == 0)
			{
				modem_send_Atcmd("AT+CFUN=0","OK",150);
				OS_TaskDelayMs(2000);
				modem_send_Atcmd("AT+CFUN=1","OK",150);
			}
			OS_Timer_Start(xGsmQueueId,MSG_GSM_CGREG,0,0,GSM_TIMER_ID,500,FALSE);// 5s
		}
	}
}

//�����Ϸ��Լ��
bool modem_paramJudge(void)
{
	if(gc_debug == 1)//����ģʽ
	{
		return TRUE;
	}
	if(strlen_t((const char *)sys_GetApn()) == 0)
	{
		LOG("[gsm] APN null\r\n");
		return FALSE;
	}
	if((strlen_t((const char *)sys_GetUrl()) == 0) && \
		(strlen_t((const char *)sys_GetIp()) == 0) && \
		(strlen_t((const char *)sys_GetUrlBak()) == 0) && \
		(strlen_t((const char *)sys_GetIpBak()) == 0))
	{
		LOG("[gsm] IP null\r\n");
		return FALSE;
	}

	return TRUE;
}

//PPP
void modem_gsm_ppp(void)
{
	char buf[128]={0};

	g_flag.bit.MutexFlag = 0;

	if(modem_paramJudge() == FALSE)//�����Ϸ����ж�
	{
		OS_SendMessage(xGsmQueueId, MSG_GSM_CGREG, 0, 0);
		return;
	}
	if(g_modem_runtime.npppCount++ > 6)//���Ӵ������࣬����ģ��
	{
		LOG("[gsm] ppp timeout...reset\r\n");
		g_modem_runtime.npppCount = 0;
		OS_SendMessage(xGsmQueueId, MSG_GSM_POW_ON, 0, 0);
		return;
	}
	modem_send_Atcmd("ATE0","OK",10);
	if(modem_send_Atcmd("AT+CGATT?","+CGATT: 1",50) != RET_AT_SUCCESS)
	{
		modem_send_Atcmd("AT+CGATT=1","OK",50);
	}

	IF_CONDITION_A7600C
	{
		memset_t((char *)buf,0,sizeof(buf));
		sprintf((char *)buf,"AT+CGDCONT=1,\"IP\",\"%s\"",sys_GetApn());
		if(modem_send_Atcmd(buf,"OK",30) != RET_AT_SUCCESS)
		{
			modem_send_disconnect(0);
			return;
		}
		memset_t((char *)buf,0,sizeof(buf));
		sprintf((char *)buf,"AT+CGAUTH=1,2,\"%s\",\"%s\"",sys_GetApnUser(),sys_GetApnPasswd());
		if(modem_send_Atcmd(buf,"OK",30) != RET_AT_SUCCESS)
		{
			modem_send_disconnect(0);
			return;
		}
		
		g_modem_runtime.nSocketState = RET_AT_WAITING;
		if(modem_send_Atcmd("AT+NETOPEN","OK",30) != RET_AT_SUCCESS)
		{
			modem_send_disconnect(0);
			return;
		}
		SocketWait_SemTake(xSocketSem, (30*1000));
		if(g_modem_runtime.nSocketState != RET_AT_SUCCESS)
		{
			g_modem_runtime.nSocketState = RET_AT_TIMEOUT;
			modem_send_disconnect(0);
			return;
		}
	}
	IF_CONDITION_EC20
	{
		memset_t((char *)buf,0,sizeof(buf));
		sprintf((char *)buf,"AT+QICSGP=1,1,\"%s\",\"%s\",\"%s\",0",sys_GetApn(),sys_GetApnUser(),sys_GetApnPasswd());
		if(modem_send_Atcmd(buf,"OK",10) != RET_AT_SUCCESS)
		{
			modem_send_disconnect(0);
			return;
		}
		
		if(modem_send_Atcmd("AT+QIACT=1","OK",1500) != RET_AT_SUCCESS)
		{
			modem_send_disconnect(0);
			return;
		}
	}
	
	//PPP��OK
	g_flag.bit.socketPPPFlag = 1;
	OS_SendMessage(xGsmQueueId, MSG_GSM_IP, 0, 0);
}

//����״̬��ѯ
unsigned char modem_gsm_checkppp(void)
{
	char *p=NULL;
	char *buf=(char *)modem_getAtBuf();
	int st=0;
	
	IF_CONDITION_A7600C
	{
		if(modem_send_Atcmd("AT+IPADDR","OK",90) != RET_AT_SUCCESS)
		{
			return 0;
		}

		return 1;
	}
	IF_CONDITION_EC20
	{
		if(modem_send_Atcmd("AT+QIACT?","OK",30) != RET_AT_SUCCESS)
		{
			return 0;
		}
		//+QIACT: 1,1,1,"10.200.57.37"
		p = strstr_t(buf,"+QIACT:");
		if(p == NULL)
		{
			return 0;
		}
		sscanf(p, "%*[^,],%d", &st);
		if(st == 0)
		{
			return 0;
		}

		return 1;
	}

	return 1;
}

//IP
void modem_gsm_ip(void)
{
	char buf[128]={0};
	unsigned char type=0;
	unsigned char *ipbuf=NULL;
	unsigned char linktype=0;
	unsigned short port=0;

	if(modem_gsm_checkppp() == 0)
	{
		modem_send_disconnect(0);
		return;
	}
	modem_send_Atcmd("ATE0","OK",10);

	if(strlen_t((const char *)sys_GetUrl())!=0)
	{
		type = 0;
	}
	if(strlen_t((const char *)sys_GetIp())!=0)
	{
		type = 1;
	}

	if(type == 0)
	{
		ipbuf = sys_GetUrl();
		port = sys_GetPort();
		linktype = sys_GetLinkType();
	}
	else if(type == 1)
	{
		ipbuf = sys_GetIp();
		port = sys_GetPort();
		linktype = sys_GetLinkType();
	}
	else
	{
		;
	}

	IF_CONDITION_A7600C
	{
		if(linktype == 1)//TCP
		{
			sprintf((char*)buf,"AT+CIPOPEN=1,\"TCP\",\"%s\",%d",ipbuf,port);
			g_modem_runtime.nSocketState = RET_AT_WAITING;
			if(modem_send_Atcmd(buf,"OK",30) == RET_AT_SUCCESS)
			{
				SocketWait_SemTake(xSocketSem, (30*1000));
				if(g_modem_runtime.nSocketState == RET_AT_SUCCESS)
				{
					g_modem_runtime.nIp1State = GSM_IP_STATE;
					g_modem_runtime.nIp1Waittime = 0;
					g_modem_runtime.nIp1ErrCount = 0;
					OS_SendMessage(xGsmQueueId, MSG_GSM_LOGIN, 0, 0);

					return;
				}
				else
				{
					g_modem_runtime.nSocketState = RET_AT_TIMEOUT;
					LOG("+CIPOPEN: error/timeout\r\n");
				}
			}
			else
			{
				g_modem_runtime.nSocketState = RET_AT_ERROR;
				//+CIPOPEN: 1,3
				//OK
				SocketWait_SemTake(xSocketSem, (5*1000));
			}
		}
		else//UDP
		{
			sprintf((char*)buf,"AT+CIPOPEN=1,\"UDP\",\"%s\",%d,%d",ipbuf,port,port);
			if(modem_send_Atcmd(buf,"OK",30) == RET_AT_SUCCESS)
			{
				g_modem_runtime.nIp1State = GSM_IP_STATE;
				g_modem_runtime.nIp1Waittime = 0;
				g_modem_runtime.nIp1ErrCount = 0;
				OS_SendMessage(xGsmQueueId, MSG_GSM_LOGIN, 0, 0);

				return;
			}
		}
	}
	IF_CONDITION_EC20
	{
		if(linktype == 1)//TCP
		{
			sprintf((char*)buf,"AT+QIOPEN=1,1,\"TCP\",\"%s\",%d,0,1",ipbuf,port);	
		}
		else//UDP
		{
			sprintf((char*)buf,"AT+QIOPEN=1,1,\"UDP\",\"%s\",%d,0,1",ipbuf,port);
		}

		g_modem_runtime.nSocketState = RET_AT_WAITING;
		if(modem_send_Atcmd(buf,"OK",30) == RET_AT_SUCCESS)
		{
			SocketWait_SemTake(xSocketSem, (30*1000));
			if(g_modem_runtime.nSocketState == RET_AT_SUCCESS)
			{
				if(modem_send_Atcmd("AT+QISTATE=1,1","+QISTATE:",30) == RET_AT_SUCCESS)
				{
					g_modem_runtime.nIp1State = GSM_IP_STATE;
					g_modem_runtime.nIp1Waittime = 0;
					g_modem_runtime.nIp1ErrCount = 0;
					OS_SendMessage(xGsmQueueId, MSG_GSM_LOGIN, 0, 0);

					return;
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
//	IF_CONDITION_LYNQ
//	{
//		if(linktype == 1)//TCP
//		{
//			sprintf((char*)buf,"AT+SKTCONNECT=1,1,\"TCP\",\"%s\",%d,0,1",ipbuf,port);	
//		}
//		else//UDP
//		{
//			sprintf((char*)buf,"AT+SKTCONNECT=1,1,\"UDP\",\"%s\",%d,0,1",ipbuf,port);
//		}

//		g_modem_runtime.nSocketState = RET_AT_WAITING;
//		if(modem_send_Atcmd(buf,"OK",30) == RET_AT_SUCCESS)
//		{
//			SocketWait_SemTake(xSocketSem, (30*1000));
//			if(g_modem_runtime.nSocketState == RET_AT_SUCCESS)
//			{
//				if(modem_send_Atcmd("AT+QISTATE=1,1","+QISTATE:",30) == RET_AT_SUCCESS)
//				{
//					g_modem_runtime.nIp1State = GSM_IP_STATE;
//					g_modem_runtime.nIp1Waittime = 0;
//					g_modem_runtime.nIp1ErrCount = 0;
//					OS_SendMessage(xGsmQueueId, MSG_GSM_LOGIN, 0, 0);

//					return;
//				}
//			}
//			else
//			{
//				g_modem_runtime.nSocketState = RET_AT_TIMEOUT;
//				LOG("+QIOPEN: error/timeout\r\n");
//			}
//		}
//		else
//		{
//			g_modem_runtime.nSocketState = RET_AT_ERROR;
//		}
//	}
	modem_gsm_disconnect_ip1();
	modem_gsm_ip1_errcheck();

	return;
}

//ͨ�ò�ѯ
void modem_general_check(void)
{
#define GSM_GENERAL_TIMER_PERIOD 30

	if((g_modem_runtime.tick_value%GSM_GENERAL_TIMER_PERIOD) == 0)
	{
		modem_CsqCheck();
		modem_CgregCheck();

	}
	if((g_modem_runtime.tick_value%60) == 0)
	{
		IF_CONDITION_A7600C
		{
			modem_send_Atcmd("AT+CPSI?","OK",10);
		}
	}
}

//����
void modem_gsm_disconnect(void)
{
	OS_TaskDelayMs(5000);
	
	IF_CONDITION_A7600C
	{
		modem_send_Atcmd("AT+CIPCLOSE=1","OK",100);
		modem_send_Atcmd("AT+NETCLOSE","+NETCLOSE:",100);
	}
	IF_CONDITION_EC20
	{
		modem_send_Atcmd("AT+QICLOSE=1","OK",100);
		modem_send_Atcmd("AT+QIDEACT=1","OK",100);
	}
	
	OS_SendMessage(xGsmQueueId, MSG_GSM_CGREG, 0, 0);
}

void modem_gsm_logout(void)
{

	if((modem_GetState() != GSM_LOGIN_STATE)&&(modem_GetState() != GSM_ONLINE_STATE))
	{
		return;
	}

}

void modem_gsm_disconnect_ip1(void)
{
	OS_Timer_Stop(GSM_TIMER_ID);

	IF_CONDITION_A7600C
	{
		modem_send_Atcmd("AT+CIPCLOSE=1","OK",100);
	}
	IF_CONDITION_EC20
	{
		modem_send_Atcmd("AT+QICLOSE=1","OK",100);
	}

	g_modem_runtime.nIp1State = GSM_CLOSE_STATE;
	g_modem_runtime.nIp1Waittime = 0;
}

void modem_gsm_reconnect_ip1(void)
{
	if(modem_GetState() == GSM_FOTA_STATE)
	{
		return;
	}
	
	if(g_modem_runtime.nIp1State == GSM_CLOSE_STATE)
	{
		g_modem_runtime.nIp1Waittime ++;
		if(g_modem_runtime.nIp1Waittime >= 30)
		{
			g_modem_runtime.nIp1State = GSM_IP_STATE;
			g_modem_runtime.nIp1Waittime = 0;
			OS_SendMessage(xGsmQueueId, MSG_GSM_IP, 0, 0);
		}
	}
}

void modem_gsm_ip1_errcheck(void)
{
	g_modem_runtime.nIp1ErrCount++;
	if(g_modem_runtime.nIp1ErrCount >= 60)
	{
		g_modem_runtime.nIp1State = GSM_INIT_STATE;
		g_modem_runtime.nIp1Waittime = 0;
		g_modem_runtime.nIp1ErrCount = 0;
		modem_send_disconnect(0);
	}
}

void modem_send_disconnect(unsigned char IsSetParam)
{
	if(modem_GetCallState())//��ͨ���¼���ϵĲ���dail close
	{
		LOG("[gsm] dailclose ring[%d]\r\n",modem_GetCallState());
		return;
	}
	
	if(g_flag.bit.MutexFlag == 1)
	{
		LOG("[gsm] dailclose mutex\r\n");
		return;
	}
	
	g_flag.bit.MutexFlag = 1;

	if(IsSetParam == 1)//��������״̬�·����ò�������������
	{
		if(modem_GetState() >= GSM_CREG_STATE)
		{
			OS_Timer_Stop(GSM_TIMER_ID);
			OS_SendMessage(xGsmQueueId, MSG_GSM_DAILCLOSE, 0, 0);
			LOG("[gsm] dailclose1\r\n");
		}
	}
	else
	{
		OS_Timer_Stop(GSM_TIMER_ID);
		OS_SendMessage(xGsmQueueId, MSG_GSM_DAILCLOSE, 0, 0);
		LOG("[gsm] dailclose0\r\n");
	}
}

//��½����
void modem_gsm_Login(void)
{
	if(g_modem_runtime.nLoginCount >= 6)//��ʱ
	{
		g_modem_runtime.nLoginCount = 0;
		LOG("[gsm] ip1 login timeout\r\n");
		modem_gsm_disconnect_ip1();
	}
	else
	{		
		aw_udp_hangup();//��½
		
		g_modem_runtime.nLoginCount++;
		OS_Timer_Start(xGsmQueueId,MSG_GSM_LOGIN,0,0,GSM_TIMER_ID,1000,FALSE);// 10s
	}
	if(g_modem_runtime.nIp1State == GSM_CLOSE_STATE)
	{
		OS_Timer_Stop(GSM_TIMER_ID);
	}
}

//��������
void modem_gsm_center(void)
{
#define APP_HEART_TIMER_PERIOD 	60

	g_modem_runtime.tick_value++;

	if((g_modem_runtime.tick_value%APP_HEART_TIMER_PERIOD) == 0)
	{
		if(g_modem_runtime.nCenterCount >= 3)//��ʱ
		{
			g_modem_runtime.tick_value   = 0;
			g_modem_runtime.nCenterCount = 0;
			LOG("[gsm] heart timeout\r\n");
			modem_gsm_disconnect_ip1();

			return;
		}
		else
		{
			aw_udp_keepalive();//����
			g_modem_runtime.nCenterCount++;
		}
	}

	app_otaResultReport(1);
	
	modem_queue_send();

	modem_general_check();

	OS_Timer_Start(xGsmQueueId,MSG_GSM_CENTER,0,0,GSM_TIMER_ID,100,FALSE);// 1s

	if(g_modem_runtime.nIp1State == GSM_CLOSE_STATE)
	{
		OS_Timer_Stop(GSM_TIMER_ID);
	}
}

//����
void modem_clear_flagCnt(void)
{
	taskENTER_CRITICAL();
	AckModem.sendcnt = 0;
	g_modem_runtime.tick_value  = 0;
	g_modem_runtime.nLoginCount = 0;
	g_modem_runtime.nIp1State = GSM_INIT_STATE;
	g_modem_runtime.nIp1ErrCount = 0;
	g_modem_runtime.nIp1Waittime = 0;
	g_modem_runtime.nIp2State = GSM_INIT_STATE;
	g_modem_runtime.nIp2ErrCount = 0;
	g_modem_runtime.nIp2Waittime = 0;
	g_flag.bit.connIP2Flag = 0;
	g_flag.bit.socketPPPFlag = 0;
	taskEXIT_CRITICAL();
	
	OS_Timer_Stop(GSM_TIMER_ID);
	modem_SetState(GSM_CLOSE_STATE);
}

//�������ݷ���
void modem_queue_send(void)
{
	if(gc_debug == 1)
	{
		return;
	}
	if(/*sys_GetAckSwitch() == 1*/0)//����Ӧ��ģʽ
	{
		if(AckModem.ack == 0)
		{
			g_gsmqueue.len = MsgGsm_OutputMsg(g_gsmqueue.buf);//�Ӷ�����ȡ����
			if(g_gsmqueue.len)
			{
				g_gsmqueue.acktimeout = 0;
				if(oa_JT_query_filter_msg(g_gsmqueue.buf,g_gsmqueue.len) == 0)//��ѯ�Ƿ�ΪЭ������
				{
					modem_SetAckSendInit(g_gsmqueue.buf[1],g_gsmqueue.buf[2]);
				}
				else
				{
					modem_AckModemInit();
				}
			}
		}
		switch(modem_GetAckModem())
		{
			case 1:
				switch(g_gsmqueue.acktimeout)
				{
					case 0:
						g_gsmqueue.acktimeout++;
						modem_SendAckModeData(g_gsmqueue.buf, g_gsmqueue.len);
						break;
					default:
						g_gsmqueue.acktimeout++;
						if(g_gsmqueue.acktimeout >= 30)//ƽ̨ûӦ��,�ط�
						{
							g_gsmqueue.acktimeout = 1;
							modem_SendAckModeData(g_gsmqueue.buf, g_gsmqueue.len);
						}
						break;
				}
				break;
			case 2://�ն��յ�Ӧ��,����
				modem_AckModemInit();
				break;
			default:
				break;
		}
	}
	else//��Ӧ��ģʽ
	{
		modem_AckModemInit();
	
		g_gsmqueue.len = MsgGsm_OutputMsg(g_gsmqueue.buf);//�Ӷ�����ȡ����
		if(g_gsmqueue.len)
		{
			if(oa_JT_query_filter_msg(g_gsmqueue.buf,g_gsmqueue.len) == 0)//��ѯ�Ƿ�ΪЭ������
			{
				if(modem_send_gprsData((char*)g_gsmqueue.buf,"OK",30,g_gsmqueue.len) == 0)//��ʱ��error����ѹ��������
				{
					MsgGsm_InputMsg(g_gsmqueue.buf,g_gsmqueue.len);
				}
			}
		}
	}
}

//����ģ��״̬
void modem_SetState(unsigned char state)
{
	g_modem_runtime.nModemState = state;
}
//��ȡģ��״̬
unsigned char modem_GetState(void)
{
    return  g_modem_runtime.nModemState;
}

//ģ�鷵�����ݴ���
typedef void (*proc_t)(unsigned char *pStr);


typedef struct {
	char  *p_idstr;
	proc_t p_CallBack;
} notify_proc_t;

void modem_urc_creg(unsigned char *pStr)
{
	char *p=NULL;
	char *pc=NULL;
	char str[12]={0};
	
	p = strstr_t((char *)pStr,"+CREG:");
	if(p == NULL)
	{
		return;
	}
	p += strlen_t("+CREG:");

	memset_t(str,0,sizeof(str));
	GetStringPart((const char **)&p,(char *)str,10);//����

	p++;
	memset_t(str,0,sizeof(str));
	GetStringPart((const char **)&p,(char *)str,10);//lac
	pc = strchr_t(&str[1], '\"');
	if(pc == NULL)
	{
		return;
	}
	pc[0] = 0;
	modem_SetLac(HexString2Dword(&str[1]));

	p++;
	memset_t(str,0,sizeof(str));
	GetStringPart((const char **)&p,(char *)str,10);//cellid
	pc = strchr_t(&str[1], '\"');
	if(pc == NULL)
	{
		return;
	}
	pc[0] = 0;
	modem_SetCellid(HexString2Dword(&str[1]));
}

void modem_urc_cmti(unsigned char *pStr)
{
	unsigned char *p=NULL;
	unsigned int index;

	p = (unsigned char *)strchr_t((char *)pStr, ',');
	if(p == NULL)
	{
		return;
	}
	
	index = myatoi((char *)&p[1]);
	
	OS_SendMessage(xGsmQueueId, MSG_GSM_READ_SMS, index, 0);
}

void modem_urc_clip(unsigned char *pStr)
{
	if(memcmp_t(pStr, "RING", strlen_t("RING")) == 0)
	{
		modem_SetCallState(CALL_RINGING);
		OS_SendMessage(xGsmQueueId, MSG_GSM_CALL, MSG_GSM_RING, 0);
	}
	else if(memcmp_t(pStr, "NO CARRIER", strlen_t("NO CARRIER")) == 0)
	{
		OS_SendMessage(xGsmQueueId, MSG_GSM_CALL, MSG_GSM_NO_CARRIER, 0);
	}
	else if(memcmp_t(pStr, "NO ANSWER", strlen_t("NO ANSWER")) == 0)
	{
		OS_SendMessage(xGsmQueueId, MSG_GSM_CALL, MSG_GSM_NO_ANSWER, 0);
	}
	else if(memcmp_t(pStr, "BUSY", strlen_t("BUSY")) == 0)
	{
		OS_SendMessage(xGsmQueueId, MSG_GSM_CALL, MSG_GSM_BUSY, 0);
	}
	else
	{
		;
	}
}

void modem_urc_cipopen(unsigned char *pStr)
{
	int connectID;
	int err;

	if(g_modem_runtime.nSocketState == RET_AT_WAITING)
	{
		sscanf((char *)pStr, "%*[^:]:%d,%d[^\r\n]",&connectID,&err);
		if((connectID == 1) && (err == 0))
		{
			g_modem_runtime.nSocketState = RET_AT_SUCCESS;
		}
		else if((connectID == 2) && (err == 0))
		{
			g_modem_runtime.nSocketState = RET_AT_SUCCESS;
		}
		else
		{
			g_modem_runtime.nSocketState = RET_AT_ERROR;
		}
		SocketWait_SemGive(xSocketSem);
	}
}

void modem_urc_netopen(unsigned char *pStr)
{
	int st;
	
	if(g_modem_runtime.nSocketState == RET_AT_WAITING)
	{
		sscanf((char *)pStr, "%*[^:]:%d",&st);
		if(st == 0)
		{
			g_modem_runtime.nSocketState = RET_AT_SUCCESS;
			SocketWait_SemGive(xSocketSem);
		}
	}
}

void modem_urc_ipd(unsigned char *pStr)
{
	unsigned char *buf=NULL;
	unsigned char *pFind=NULL;
	int connectID=0;
	int len;
	int port;

	//RECV FROM:117.131.85.139:5253
	//+IPD16
	//data from server
	sscanf((char *)pStr, "%*[^:]:%*[^:]:%d",&port);
	connectID = 1;
	
	pFind = (unsigned char *)strstr_t((char *)pStr, "\r\n");
	if(pFind == NULL)
	{
		return;
	}
	pFind += 2;
	pStr = pFind;
	
	pFind = (unsigned char *)strstr_t((char *)pStr, "+IPD");
	if(pFind == NULL)
	{
		return;
	}
	pFind += 4;
	pStr = pFind;
	len = myatoi((char *)pStr);
	
	pFind = (unsigned char *)strstr_t((char *)pStr, "\r\n");
	if(pFind == NULL)
	{
		return;
	}
	pFind += 2;
	pStr = pFind;
	
	LOG("\r\n[gsm] rcv id:%d len:%d\r\n",connectID,len);
	
	buf = (unsigned char *)pvPortMalloc(len+1);
	if(buf == NULL)
	{
		return;
	}
	memset_t((unsigned char *)buf,0,len+1);
	memcpy_t((unsigned char *)buf, pStr, len);

	OS_SendMessage(xGsmQueueId, MSG_GSM_RECV_SOCKET, (unsigned int)buf, (len<<16)|connectID);
}

void modem_urc_ipclose(unsigned char *pStr)
{
	int connectID;
	int err;

	//+IPCLOSE: 3,1
	sscanf((char *)pStr, "%*[^:]:%d,%d[^\r\n]",&connectID,&err);
	OS_SendMessage(xGsmQueueId, MSG_GSM_CLOSEIP, connectID, 0);
}

void modem_urc_qiopen(unsigned char *pStr)
{
	int connectID;
	int err;

	if(g_modem_runtime.nSocketState == RET_AT_WAITING)
	{
		sscanf((char *)pStr, "%*[^:]:%d,%d[^\r\n]",&connectID,&err);
		if((connectID == 1) && (err == 0))
		{
			g_modem_runtime.nSocketState = RET_AT_SUCCESS;
		}
		else if((connectID == 2) && (err == 0))
		{
			g_modem_runtime.nSocketState = RET_AT_SUCCESS;
		}
		else
		{
			g_modem_runtime.nSocketState = RET_AT_ERROR;
		}
		SocketWait_SemGive(xSocketSem);
	}
}

void modem_urc_qiurc(unsigned char *pStr)
{
	unsigned char *pFind=NULL;
	unsigned char *buf=NULL;
	int connectID;
	int len;
	
	if(strstr_t((char *)pStr, "recv"))
	{
		pStr = (unsigned char *)strstr_t((char *)pStr, "recv");
		sscanf((char *)pStr, "%*[^,],%d,%d[^\r\n]",&connectID,&len);

		pFind = (unsigned char *)strchr_t((char *)pStr, '\n');
		if(pFind == NULL)
		{
			return;
		}
		pFind ++;

		/*******************************************************/
		//���������쳣,ָ����ǰ�ƶ���ԭ��λ��
		if((pFind[len] != 0x0D) && (pFind[len+1] != 0x0A))
		{
			LOG("[gsm] rcv abnormal,again\r\n");
			return;
		}
		/*******************************************************/
		
		LOG("[gsm] rcv id:%d len:%d\r\n",connectID,len);
		
		buf = (unsigned char *)pvPortMalloc(len+1);
		if(buf == NULL)
		{
			return;
		}
		memset_t((unsigned char *)buf,0,len+1);
		memcpy_t((unsigned char *)buf, pFind, len);

		OS_SendMessage(xGsmQueueId, MSG_GSM_RECV_SOCKET, (unsigned int)buf, (len<<16)|connectID);
	}
	else if(strstr_t((char *)pStr, "closed"))
	{
		pStr = (unsigned char *)strstr_t((char *)pStr, "closed");
		sscanf((char *)pStr, "%*[^,],%d[^\r\n]",&connectID);

		LOG("[urc] closed id:%d\r\n",connectID);
		if((modem_GetState() == GSM_ONLINE_STATE)||(modem_GetState() == GSM_LOGIN_STATE))
		{
			OS_SendMessage(xGsmQueueId, MSG_GSM_CLOSEIP, connectID, 0);
		}
	}
	else if(strstr_t((char *)pStr, "pdpdeact"))
	{
		if((modem_GetState() == GSM_ONLINE_STATE)||(modem_GetState() == GSM_LOGIN_STATE))
		{
			LOG("[urc] pdpdeact\r\n");
			modem_send_disconnect(0);
		}
	}
}

static notify_proc_t at_proc[] = {
	{"+CMTI:",			modem_urc_cmti},
	{"RING",			modem_urc_clip},
	{"NO CARRIER",		modem_urc_clip},
	{"NO ANSWER",		modem_urc_clip},
	{"BUSY",			modem_urc_clip},

	{"+NETOPEN:", 	   	modem_urc_netopen},
	{"+CIPOPEN:", 	   	modem_urc_cipopen},
	{"RECV FROM:", 	   	modem_urc_ipd},
	{"+IPCLOSE:",		modem_urc_ipclose},

	{"+QIOPEN:",		modem_urc_qiopen},
	{"+QIURC:",			modem_urc_qiurc}
};

static const int MAX_POS = (sizeof(at_proc)/sizeof(at_proc[0]));

void modem_recv_proc(unsigned short urclen)
{
	unsigned char *buf=NULL;
	unsigned char *pFind=NULL;
	unsigned short i=0,len=0;

	len = modem_getAtBufLen();
	buf = modem_getAtBuf();
	gModemRecvLen = urclen;

	modem_response(buf, len);

	memset_t((unsigned char *)gModemRecvBuf, 0, MAX_GSM_UART_RCV_LEN);
	modem_getModStr(gModemRecvBuf, gModemRecvLen);

	for(i=0; i<MAX_POS; i++)
    {
		pFind = (unsigned char *)strstr_t((char *)gModemRecvBuf, (char *)at_proc[i].p_idstr);
        if(pFind) 
		{
            if(at_proc[i].p_CallBack)
			{
				LOG("%s",pFind);
                at_proc[i].p_CallBack(pFind);
				break;
			}
        }
    }
}

void vGsmTaskQueueCreat(void)
{
	xGsmQueueId = xQueueCreate(16 , sizeof(OS_EVENT));
	if(xGsmQueueId == 0)
	{
		LOG("#gsm task queue fail\r\n");
	}
}

void vGsmTask(void *pvParameters)
{
	OS_EVENT Msg;

	OS_SendMessage(xGsmQueueId, MSG_GSM_INIT, 0, 0);

	LOG("[task] gsm start\r\n");
	
    for( ;; )
    {
		vGetStackHighWaterMark("GsmTask");

		OS_GetMessage(xGsmQueueId, &Msg);	
		LOG("gsm run free:%d\r\n",uxTaskGetStackHighWaterMark(NULL));		
		switch(Msg.sEventType)
		{
			case MSG_GSM_INIT:
			{
				modem_init();
			}
			break;

			case MSG_GSM_POW_ON:
			{
				modem_gsm_init();
				modem_SetState(GSM_INIT_STATE);
			}
			break;
			
			case MSG_GSM_CGREG:
			{
				modem_gsm_cgreg();
				modem_SetState(GSM_CREG_STATE);
			}
			break;

			case MSG_GSM_PPP:
			{
				modem_gsm_ppp();
				modem_SetState(GSM_PPP_STATE);
				handset_ota_waiting();
			}
			break;

			case MSG_GSM_IP:
			{
				modem_gsm_ip();
				modem_SetState(GSM_IP_STATE);
			}
			break;

			case MSG_GSM_LOGIN:
			{
				modem_gsm_Login();
				modem_SetState(GSM_LOGIN_STATE);
			}
			break;

			case MSG_GSM_CENTER:
			{
				modem_gsm_center();
				modem_SetState(GSM_ONLINE_STATE);
			}
			break;
			
			case MSG_GSM_IP2:
			{
				modem_gsm_ipvip();
			}
			break;

			case MSG_GSM_LOGIN2:
			{
				modem_gsm_Login2();
			}
			break;

			case MSG_GSM_CENTER2:
			{
				modem_gsm_center2();
			}
			break;
			case MSG_GSM_ACCOFF:
			{
				modem_gsm_logout();
			}
			break;

			case MSG_GSM_CLOSEIP:
			{
			#if 0
				if(Msg.param1 == 1)
				{
					modem_gsm_disconnect_ip1();
				}
			#endif
			}
			break;

			case MSG_GSM_DAILCLOSE:
			{
				modem_clear_flagCnt();
				modem_gsm_disconnect();
				modem_SetState(GSM_CLOSE_STATE);
			}
			break;

			case MSG_GSM_READ_SMS:
			{
				sms_ReadSms(Msg.param1);
			}
			break;

			case MSG_GSM_CHECK_SMS:
			{
				sms_CheckSms();
			}
			break;

			case MSG_GSM_DEL_ALL_SMS:
			{
				sms_DelAllSms();
			}
			break;

			case MSG_GSM_CALL:
			{
				switch(Msg.param1)
				{
					case MSG_GSM_ATA:
						modem_AnwserPhone();
						break;
					case MSG_GSM_NO_CARRIER:
					case MSG_GSM_NO_ANSWER:
					case MSG_GSM_BUSY:
						modem_HandUpClear();
						break;
					default:
						modem_ClccCheck();
						break;
				}
			}
			break;

			case MSG_GSM_READ_SOCKET:
			{
				
			}
			break;

			case MSG_GSM_RECV_SOCKET:
			{
				unsigned char *pRecvBuf = (unsigned char *)Msg.param1;
				unsigned char RecvSocket = (unsigned char)Msg.param2;
				unsigned short RecvLen = (unsigned short)(Msg.param2>>16);

				switch(RecvSocket)
				{
					case 1:
					{
						if(modem_GetState() == GSM_LOGIN_STATE)
						{
							oa_JT_check_login((char *)pRecvBuf,RecvLen);
						}
						else
						{
							oa_recv_Handle((char *)pRecvBuf,RecvLen);
						}
					}
					break;
					case 2:
						ipv_recv_Handle((char *)pRecvBuf,RecvLen);
						break;
				}
				
				vPortFree(pRecvBuf);
			}
			break;

			case MSG_GSM_UPDATE_START:
			{
				OS_Timer_Stop(GSM_TIMER_ID);
				OS_SendMessage(xGsmQueueId, MSG_GSM_UPDATE_ING, 0, 0);
				modem_SetState(GSM_FOTA_STATE);
			}
			break;

			case MSG_GSM_UPDATE_ING:
			{
				modem_SetState(GSM_FOTA_STATE);
				OS_Timer_Start(xGsmQueueId,MSG_GSM_UPDATE_ING,0,0,GSM_TIMER_ID,100,FALSE);// 1s
				update_linkTFTP(0);
			}
			break;
			
			default:
			break;
		}
    }
}

