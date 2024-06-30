#include "includes.h"

static unsigned char sms_status=0;
static unsigned char sms_oatype __noinit__;
static unsigned char sms_phone[MAX_PHONE_NUM] __noinit__;
//���ն�������
SMSTextMsg sDeliverTextInfo;
//���Ͷ�������
SMSTextMsg sSubmitTextInfo;
//�����ŷְ�����
SMSTextMsg sSubpackageTextInfo;

void sms_noinit_clear(void)
{
	sms_oatype = 0;
	memset_t(sms_phone, 0, MAX_PHONE_NUM);
}

void sms_get_phone(void)
{	
	sms_oatype = sSubmitTextInfo.oa;
	memset_t(sms_phone, 0, MAX_PHONE_NUM);
	memcpy_t(sms_phone, sSubmitTextInfo.phone_num, strlen_t((char *)sSubmitTextInfo.phone_num));
}

//ȥ�����пո�
char * del_space( char * const string )
{
   char *p = string;
   int j = 0;

   // delete spaces among characters
   while( *p != '\0' )
   {
      if( *p != ' ' )
      {
         string[j++] = *p;
      }
      p++;
   }
   string[j] = '\0';
   
   return string;
}

//����ָ����Ÿ�ʽ
//AS1234*F10#...#
int sms_cmd_fly(unsigned char *str, unsigned char *out)
{
	unsigned char *pend=NULL;
	
	pend = (unsigned char *)strchr_t((char *)str, '#');
	if(pend == NULL)
	{
		return 0;
	}
	*pend = 0;

	sSave_flag  = 0;
	sReset_flag = 0;
	sReppp_flag = 0;

	if(str[0] == 'F')
	{
		sprintf((char *)out,"VIN:%s,Reset",sys_GetCarVin());
		sms_status = SMS_FACTORY;
	}
	else
	{
		if(str[0] == 'U') sms_get_phone();
		handset_all_command(str,strlen_t((char *)str),out);
	}
	if(strlen_t((char *)out) > 0)
	{
		if(sSave_flag & SAVE_RUN_FLAG)
		{
			sys_saveRuning();
		}
		if(sSave_flag & SAVE_SYS_FLAG)
		{
			sys_saveSys();
		}
		if(sReset_flag == 1)
		{
		   	sms_status = SMS_RESTART;
		}

		if(sReppp_flag == 1)
		{
			sms_status = SMS_DISCONNECT_TCPIP;
		}
	}
	else
	{
		return 0;
	}
	return 1;
}

static SMS_T sms_cmd_proc[] =
{	
	{"*F10#",			sms_cmd_fly},
};

static const int MAX_SMS = (sizeof(sms_cmd_proc)/sizeof(sms_cmd_proc[0]));

//���Ŵ���
void sms_MsgHandle(SMSTextMsg *pDeliverTextInfo)
{
	unsigned char ret=0;
	unsigned char i=0;
	unsigned char *psms=NULL;
	
	if(pDeliverTextInfo == NULL)
	{
		return ;
	}

	memset_t((unsigned char *)&sSubmitTextInfo,0,sizeof(SMSTextMsg));

	sSubmitTextInfo.dcs = SMS_CHSET_GSM;
	sSubmitTextInfo.oa = pDeliverTextInfo->oa;
	memcpy_t((unsigned char *)sSubmitTextInfo.phone_num, (unsigned char *)pDeliverTextInfo->phone_num, strlen_t((char *)pDeliverTextInfo->phone_num));
			
	pDeliverTextInfo->data[pDeliverTextInfo->data_len] = 0;
	LOG("[sms] len:%d phone:%s text:%s\r\n",pDeliverTextInfo->data_len,pDeliverTextInfo->phone_num,pDeliverTextInfo->data);

	del_space((char *)pDeliverTextInfo->data);
	if(pDeliverTextInfo->data_len != strlen_t((char *)pDeliverTextInfo->data))
	{
		pDeliverTextInfo->data_len = strlen_t((char *)pDeliverTextInfo->data);
		LOG("[sms]!! len:%d phone:%s text:%s\r\n",pDeliverTextInfo->data_len,pDeliverTextInfo->phone_num,pDeliverTextInfo->data);
	}
	
	if(strstr_t((char *)pDeliverTextInfo->data,SMS_SUPER_PASSWORD) != NULL)
	{//����ָ���ó�������,�򲻹��Ƿ�������Ȩ����,�����������κκ��������
		;
	}
	else if(sms_CheckHaveAutor() == 0)
	{//��Ȩ����Ϊ�գ��豸�����������κκ��������
		;
	}
	else
	{//��������Ȩ����,ƥ��

	}

	//��ͷ
	psms = (unsigned char *)strstr_t((char *)pDeliverTextInfo->data, "AS");
	if(psms == NULL)
	{
		return;
	}
	//����
	psms += 2;
	if(sys_CheckTerminalPwd(psms) == 0)
	{
		strcpy_t((char *)sSubmitTextInfo.data,"key Incorrect");
		sSubmitTextInfo.data_len = strlen_t((const char *)sSubmitTextInfo.data);
		sms_MsgSend(&sSubmitTextInfo);
			
		return;
	}
	
	psms += 4;

	sms_status = SMS_NULL;

	for(i=0; i<MAX_SMS; i++)
	{
		if(strnicmp_t((char *)psms, (char *)sms_cmd_proc[i].cmd, strlen_t((char *)sms_cmd_proc[i].cmd)) == 0)
		{
			ret = sms_cmd_proc[i].cmd_handler(&psms[strlen_t((char *)sms_cmd_proc[i].cmd)],sSubmitTextInfo.data);
			if(ret == 0)
			{
				strcpy_t((char *)sSubmitTextInfo.data,"format Incorrect");
			}
			break;
		}
	}

	if(i >= MAX_SMS)
	{
		return;
	}

	sSubmitTextInfo.data_len = strlen_t((const char *)sSubmitTextInfo.data);
	if(sSubmitTextInfo.data_len == 0)
	{
		LOG("[sms] text empty err\r\n");
	}
	else
	{
		if(sSubmitTextInfo.data_len > MAX_TEXT_SMS_LENGTH)
		{
			memcpy_t(&sSubpackageTextInfo, &sSubmitTextInfo, sizeof(SMSTextMsg));
			memset_t(sSubpackageTextInfo.data, 0, sizeof(sSubpackageTextInfo.data));
			memcpy_t(sSubpackageTextInfo.data, sSubmitTextInfo.data, 150);
			sSubpackageTextInfo.data_len = 150;
			sms_LongMsgSend(&sSubpackageTextInfo, 2, 1);

			memset_t(sSubpackageTextInfo.data, 0, sizeof(sSubpackageTextInfo.data));
			memcpy_t(sSubpackageTextInfo.data, &sSubmitTextInfo.data[150], sSubmitTextInfo.data_len-150);
			sSubpackageTextInfo.data_len = sSubmitTextInfo.data_len-150;
			sms_LongMsgSend(&sSubpackageTextInfo, 2, 2);
		}
		else
		{
			sms_MsgSend(&sSubmitTextInfo);
		}
	}

	if (sms_status == SMS_DISCONNECT_TCPIP)			//����   
	{
        modem_send_disconnect(1);
	}
	if (sms_status == SMS_FACTORY)					//�ָ�����
	{
		OS_TaskDelayMs(3000);
		sys_ResetFacitory();
		sys_Restart();
	}
	if (sms_status == SMS_RESTART)                 	//����  
	{
		OS_TaskDelayMs(3000);
		sys_Restart();
	}
}

//ɾ�����еĶ���
void sms_DelAllSms(void)
{
	modem_send_Atcmd("AT+CMGD=1,4","OK",40);
}

//��ȡ����
void sms_ReadSms(unsigned short index)
{
	unsigned char sendbuf[16]={0};
	unsigned char *buf = modem_getAtBuf();

	sprintf((char *)sendbuf,"AT+CMGR=%d",index);
	modem_send_Atcmd((char *)sendbuf,"OK",100);

	sms_RecvSms(buf, index);
}

//�������
void sms_RecvSms(unsigned char *pSmsText, unsigned char index)
{
	unsigned char pdul=0,dcs=0;
	unsigned char *prio=NULL;
	unsigned char *next=NULL;
	unsigned char buf[160]={0};

	memset_t((unsigned char *)&sDeliverTextInfo,0,sizeof(SMSTextMsg));
	prio = pSmsText;	
	next = (unsigned char *)strstr_t((const char *)prio,"+CMGR:");
	if(prio == NULL)
	{
		return ;
	}

	prio = next;
	next = (unsigned char *)strchr_t((const char *)prio,',');
	if(next == NULL)
	{
		return;
	}
	
	prio = next+1;
	next = (unsigned char *)strchr_t((const char *)prio,',');
	if(next == NULL)
	{
		return;
	}
	
	prio = next+1;
	next = (unsigned char *)strchr_t((const char *)prio,'\r');
	if(next == NULL)
	{
		return;
	}
	prio = next;	
	prio += 2;

	// Pdu���Ŀ�ʼ
	pdul = Dchar2Int(prio);      //get sca length
	prio += 2;
	prio += pdul*2;
	pdul = Dchar2Int(prio);      //get pdu type
	prio += 2;
	pdul = Dchar2Int(prio);      //oa length		
	prio += 2;
	sDeliverTextInfo.oa = Dchar2Int(prio);	 
	prio += 2; 
	if(pdul%2)
	{
		pdul += 1;
	}
	
	strReverse(prio,sDeliverTextInfo.phone_num,pdul);//get oa str
	prio += pdul;
	pdul = Dchar2Int(prio);      //get pid
	prio += 2;

	dcs = Dchar2Int(prio);		 //get dcs
	prio += 2;
	prio += 14; 			     //skip	over the time stamp string 

	// get udl 
	pdul = Dchar2Int(prio);		 //sms len
	prio += 2;
	Pdu2Str(prio,pdul*2,sDeliverTextInfo.data);

	if((dcs&0x0C) == 0)// 7BIT
	{
		sDeliverTextInfo.dcs = SMS_CHSET_GSM;
		pdul = gsmDecode7bit((unsigned char*)sDeliverTextInfo.data,(char*)buf,(pdul/8)*7+(pdul%8));
		sDeliverTextInfo.data_len = GSM7bit2ASCII(buf,pdul,sDeliverTextInfo.data);    //�����ַ��Ĵ��� 
	}
	else
	{
		pdul = usc2_ascii(sDeliverTextInfo.data, buf, pdul);
		if(pdul)
		{
			sDeliverTextInfo.dcs = SMS_CHSET_GSM;
			memset_t(sDeliverTextInfo.data, 0, sizeof(sDeliverTextInfo.data));
			memcpy_t(sDeliverTextInfo.data, buf, pdul);
			sDeliverTextInfo.data_len = pdul;
		}
		else
		{
			sDeliverTextInfo.dcs = SMS_CHSET_UCS2;
		}
	}
	
	//ɾ������
	sms_Delete(index);
	
	switch(sDeliverTextInfo.dcs)
	{
		case SMS_CHSET_GSM:
			sms_MsgHandle(&sDeliverTextInfo);
			break;
		case SMS_CHSET_8BIT:
		case SMS_CHSET_UCS2:
			break;
		default:
			break;
	}
}

//��ʱ��ѯ����
void sms_TimerCheck(void)
{
	static unsigned int tick=0;

	if(modem_GetState() < GSM_CREG_STATE)
	{
		return;
	}
	if(modem_GetState() == GSM_FOTA_STATE)
	{
		return;
	}

	tick ++;
	if((tick%60) == 0)
	{
		OS_SendMessage(xGsmQueueId, MSG_GSM_CHECK_SMS, 0, 0);
	}
}

void sms_CheckSms(void)
{
	char *p=NULL;
	char *rbuf=(char*)modem_getAtBuf();
	unsigned char pdul=0,dcs=0;
	unsigned char index=0;
	unsigned char *prio=NULL;
	unsigned char buf[160]={0};

	if(modem_send_Atcmd("AT+CMGL=4","OK",50) != RET_AT_SUCCESS)
	{
		return;
	}

	for(;;)
	{
		p = strstr_t(rbuf,"+CMGL:");
		if(p == NULL)
		{
			return ;
		}
		p += 7;
		index = myatoi(p);
		p = strstr_t(p,"\r\n");
		if(p == NULL)
		{
			return ;
		}
		p += 2;
		memset_t((unsigned char *)&sDeliverTextInfo,0,sizeof(SMSTextMsg));
		rbuf = p;
		prio = (unsigned char *)p;

		// Pdu���Ŀ�ʼ
		pdul = Dchar2Int(prio);      //get sca length
		prio += 2;
		prio += pdul*2;
		pdul = Dchar2Int(prio);      //get pdu type
		prio += 2;
		pdul = Dchar2Int(prio);      //oa length		
		prio += 2;
		sDeliverTextInfo.oa = Dchar2Int(prio);	 
		prio += 2; 
		if(pdul%2)
		{
			pdul += 1;
		}
		
		strReverse(prio,sDeliverTextInfo.phone_num,pdul);//get oa str
		prio += pdul;
		pdul = Dchar2Int(prio);      //get pid
		prio += 2;

		dcs = Dchar2Int(prio);		 //get dcs
		prio += 2;
		prio += 14; 			     //skip	over the time stamp string 

		// get udl 
		pdul = Dchar2Int(prio);		 //sms len
		prio += 2;
		Pdu2Str(prio,pdul*2,sDeliverTextInfo.data);

		if((dcs&0x0C) == 0)// 7BIT
		{
			sDeliverTextInfo.dcs = SMS_CHSET_GSM;
			pdul = gsmDecode7bit((unsigned char*)sDeliverTextInfo.data,(char*)buf,(pdul/8)*7+(pdul%8));
			sDeliverTextInfo.data_len = GSM7bit2ASCII(buf,pdul,sDeliverTextInfo.data);    //�����ַ��Ĵ��� 
		}
		else
		{
			pdul = usc2_ascii(sDeliverTextInfo.data, buf, pdul);
			if(pdul)
			{
				sDeliverTextInfo.dcs = SMS_CHSET_GSM;
				memset_t(sDeliverTextInfo.data, 0, sizeof(sDeliverTextInfo.data));
				memcpy_t(sDeliverTextInfo.data, buf, pdul);
				sDeliverTextInfo.data_len = pdul;
			}
			else
			{
				sDeliverTextInfo.dcs = SMS_CHSET_UCS2;
			}
		}
		//ɾ������
		sms_Delete(index);
		
		switch(sDeliverTextInfo.dcs)
		{
			case SMS_CHSET_GSM:
				sms_MsgHandle(&sDeliverTextInfo);
				break;
			case SMS_CHSET_8BIT:
			case SMS_CHSET_UCS2:
				break;
			default:
				break;
		}
	}
}

//���Ͷ��Ÿ�ʽ   unicode 0 7bit����   1 unicode����
unsigned char sms_MsgSend(SMSTextMsg *pSubmitTextInfo)
{
	unsigned char buf[MAX_PDU_SMS_LENGTH]={0};
	unsigned char pdu[MAX_PDU_SMS_LENGTH*2]={0};
	unsigned int cnt=0,len=0;

	if(pSubmitTextInfo == NULL)
	{
		return 0;
	}
	
	LOG("\r\n[sms] send msg:%s  len:%d  phone:%s  type:%d\r\n", pSubmitTextInfo->data, pSubmitTextInfo->data_len, pSubmitTextInfo->phone_num, pSubmitTextInfo->dcs);
	
	buf[cnt++] = 0x00; //sca 
	buf[cnt++] = 0x11; //pdu type without udhi
	buf[cnt++] = 0x00; //mr
	buf[cnt++] = strlen_t((const char *)pSubmitTextInfo->phone_num);
	if(pSubmitTextInfo->oa)//��ͨ�ƶ����벻һ��
	{
		buf[cnt++] = pSubmitTextInfo->oa;
	}
	else
	{
		buf[cnt++] = 0x91; //'+'���ʱ���
	}
	strReverseF(pSubmitTextInfo->phone_num,buf+cnt,strlen_t((const char *)pSubmitTextInfo->phone_num));     // �绰����
	cnt += strlen_t((const char *)pSubmitTextInfo->phone_num)/2+strlen_t((const char *)pSubmitTextInfo->phone_num)%2;

	buf[cnt++] = 0x00; //pid
    if(pSubmitTextInfo->dcs == SMS_CHSET_UCS2)
	{
	   buf[cnt++] = 0x08; //dcs unicode
	}
	else
	{
	   buf[cnt++] = 0x00; //dcs 7bit
	}
	buf[cnt++] = 0xff;	 //vp

	if(pSubmitTextInfo->dcs == SMS_CHSET_UCS2)
	{
		buf[cnt++] = pSubmitTextInfo->data_len; //udl
		 
	    memcpy_t(&buf[cnt],pSubmitTextInfo->data,pSubmitTextInfo->data_len); //ud
		buf[cnt+pSubmitTextInfo->data_len] = 0;
		Str2pdu(buf, cnt+pSubmitTextInfo->data_len,pdu);
		len = pSubmitTextInfo->data_len;
	}
    else                    // 7bit
	{
	    len = ASCII2GSM7bit(pSubmitTextInfo->data,pSubmitTextInfo->data_len,pdu);

		buf[cnt++] = len; //udl
		 
	    len = gsmEncode7bit((char*)pdu,&buf[cnt],len);                     
	    buf[cnt+len] = 0;
		
	    memset_t((char *)pdu,0,MAX_PDU_SMS_LENGTH*2);	
	    Str2pdu(buf, cnt+len, pdu); // תΪPDU
	}

	len += (cnt-1);

	if (sms_SendAtCmd(len,pdu) == RET_AT_SUCCESS)
	{
		return 1;
	}

	return 0;
}

//�����ŷ���
//totalSMS:�ܵĶ�����
//numSMS:����ڼ���
unsigned char sms_LongMsgSend(SMSTextMsg *pSubmitTextInfo, unsigned char totalSMS, unsigned char numSMS)
{
	unsigned char buf[MAX_PDU_SMS_LENGTH]={0};
	unsigned char pdu[MAX_PDU_SMS_LENGTH*2]={0};
	unsigned int cnt=0,len=0;

	if(pSubmitTextInfo == NULL)
	{
		return 0;
	}
	
	LOG("\r\nSend long msg:%s  Len:%d  No:%s  Type:%d\r\n", pSubmitTextInfo->data, pSubmitTextInfo->data_len, pSubmitTextInfo->phone_num, pSubmitTextInfo->dcs);
	
	buf[cnt++] = 0x00; //sca 
	buf[cnt++] = 0x51; //pdu type with udhi
	buf[cnt++] = 0x00; //mr
	buf[cnt++] = strlen_t((const char *)pSubmitTextInfo->phone_num);
	if(pSubmitTextInfo->oa)//��ͨ�ƶ����벻һ��
	{
		buf[cnt++] = pSubmitTextInfo->oa;
	}
	else
	{
		buf[cnt++] = 0x91; //'+'���ʱ���
	}
	strReverseF(pSubmitTextInfo->phone_num,buf+cnt,strlen_t((const char *)pSubmitTextInfo->phone_num));     // �绰����
	cnt += strlen_t((const char *)pSubmitTextInfo->phone_num)/2+strlen_t((const char *)pSubmitTextInfo->phone_num)%2;

	buf[cnt++] = 0x00; //pid
    if(pSubmitTextInfo->dcs == SMS_CHSET_UCS2)
	{
		pSubmitTextInfo->data_len /= 2;
	   buf[cnt++] = 0x08; //dcs unicode
	}
	else
	{
	   buf[cnt++] = 0x00; //dcs 7bit
	}
	buf[cnt++] = 0xff;	 //vp

	if(pSubmitTextInfo->dcs == SMS_CHSET_UCS2)
	{
		buf[cnt++] = pSubmitTextInfo->data_len + 6; //udl
		
		//�����ű�ʾͷ
		buf[cnt++] = 0x05;
		buf[cnt++] = 0x00;
		buf[cnt++] = 0x03;
		buf[cnt++] = 0x39;
		buf[cnt++] = totalSMS;
		buf[cnt++] = numSMS;
		
		Str2pdu(buf, cnt, pdu);
		strcpy_t((char *)&pdu[strlen_t((char *)pdu)], (char *)pSubmitTextInfo->data);
		len = pSubmitTextInfo->data_len;
	}
    else                    // 7bit
	{
		//ת�������ַ�
	    len = ASCII2GSM7bit(pSubmitTextInfo->data,pSubmitTextInfo->data_len,pdu);
		
		buf[cnt++] = len + 7; //udl

		//�����ű�ʾͷ
		buf[cnt++] = 0x05;
		buf[cnt++] = 0x00;
		buf[cnt++] = 0x03;
		buf[cnt++] = 0x23;
		buf[cnt++] = totalSMS;
		buf[cnt++] = numSMS;
		
		buf[cnt++] = pdu[0]<<1;
	    len = gsmEncode7bit((char*)&pdu[1],&buf[cnt],len-1);                     
	    buf[cnt+len] = 0;
		
	    memset_t((char *)pdu,0,MAX_PDU_SMS_LENGTH*2);	
	    Str2pdu(buf, cnt+len, pdu); // תΪPDU
	}

	len += (cnt-1);

	if (sms_SendAtCmd(len,pdu) == RET_AT_SUCCESS)
	{
		return 1;
	}

	return 0;
}

//���Ͷ��� (AT����  ����PDU����)
unsigned char sms_SendAtCmd(unsigned char len,unsigned char *data)
{
	unsigned char buf[16]={0};

    sprintf((char *)buf,"AT+CMGS=%d",(int)len);
    if(modem_send_Atcmd((char*)buf,">",60) != RET_AT_SUCCESS)
    {
		LOG("[sms] send err\r\n");
	    return 0;
    }

    modem_send_AtStr((char*)data);
    modem_sendChar(0x1A);

    return modem_seek_AtAck("OK",750);
}

//ɾ������
unsigned char sms_Delete(unsigned short index)
{  
	unsigned char buf[16]={0};
	
	sprintf((char *)buf,"AT+CMGD=%d",(int)index);
	
	if(modem_send_Atcmd((char*)buf,"OK",50) == RET_AT_SUCCESS)
	{
		return 1;
	}
	
	return 0;
}

//����Ƿ���������Ȩ����
unsigned char sms_CheckHaveAutor(void)
{
	if((g_runtime.g_phone.authorPhone[0][0]  == 0) && \
	    (g_runtime.g_phone.authorPhone[1][0] == 0) && \
	    (g_runtime.g_phone.authorPhone[2][0] == 0))
	{
		return 0;
	}

	return 1;
}

//���ͱ������ŵ�ָ������
void sms_SendAlarmAutor(SMSTextMsg *pSubmitTextInfo)
{
	unsigned char i=0;
	
	for(i=0; i<PHONE_NUM; i++)
	{	
		if(strlen_t((char*)sys_GetAuthorPhone(i))>0)
		{
			pSubmitTextInfo->dcs = SMS_CHSET_GSM;
			pSubmitTextInfo->oa  = 0x91;
			strcpy_t((char *)pSubmitTextInfo->phone_num, (char *)sys_GetAuthorPhone(i));
		 	sms_MsgSend(pSubmitTextInfo);
		}
	}
}

void app_otaResultReport(unsigned char flag)
{
	if(sys_GetUpdateFlag() == 1)
	{
		sys_SetUpdateFlag(0);

		if(flag == 1)
		{
			if(strlen_t((char *)sms_phone))
			{
				memset_t((unsigned char *)&sSubmitTextInfo, 0, sizeof(SMSTextMsg));
				
				sSubmitTextInfo.dcs = SMS_CHSET_GSM;
				sSubmitTextInfo.oa = sms_oatype;
				memcpy_t(sSubmitTextInfo.phone_num, sms_phone, strlen_t((char *)sms_phone));
				
				sprintf((char *)sSubmitTextInfo.data,"OTA OK,APP:%s VIN:%s",APP_VER,sys_GetCarVin());
				sSubmitTextInfo.data_len = strlen_t((const char *)sSubmitTextInfo.data);

				sms_MsgSend(&sSubmitTextInfo);
			}
			
		}
		
		sms_noinit_clear();
	}
	else if(sys_GetUpdateFlag() == 2)
	{
		sys_SetUpdateFlag(0);
		
	}
	else
	{
		sys_SetUpdateFlag(0);
	}
}

int gsmEncode7bit(const char* pSrc, unsigned char* pDst, int nSrcLength)
{
	int nSrc; // Դ�ַ����ļ���ֵ
	int nDst; // Ŀ����봮�ļ���ֵ
	int nChar; // ��ǰ���ڴ���������ַ��ֽڵ���ţ���Χ��0-7
	unsigned char nLeft = 0; // ��һ�ֽڲ�������� // ����ֵ��ʼ��
	nSrc = 0;
	nDst = 0; // ��Դ��ÿ8���ֽڷ�Ϊһ�飬ѹ����7���ֽ�
	// ѭ���ô�����̣�ֱ��Դ����������
	// ������鲻��8�ֽڣ�Ҳ����ȷ����
	while (nSrc < nSrcLength)
	{
		// ȡԴ�ַ����ļ���ֵ�����3λ
		nChar = nSrc & 7; // ����Դ����ÿ���ֽ�
		if(nChar == 0)
		{
		  // ���ڵ�һ���ֽڣ�ֻ�Ǳ�����������������һ���ֽ�ʱʹ��
		  nLeft = *pSrc;
		}
		else
		{
		  // ���������ֽڣ������ұ߲��������������ӣ��õ�һ��Ŀ������ֽ�
		  *pDst = (*pSrc << (8-nChar)) | nLeft;   // �����ֽ�ʣ�µ���߲��֣���Ϊ�������ݱ�������
		  nLeft = *pSrc >> nChar;   // �޸�Ŀ�괮��ָ��ͼ���ֵ
		  pDst++;
		  nDst++;
		} // �޸�Դ����ָ��ͼ���ֵ
		pSrc++;
		nSrc++;
	} // ����Ŀ�괮����
    if(nSrcLength%8){
        *pDst = nLeft;
        pDst++;
        nDst++;
    }
	return nDst;
}
/***********************************************************************
// 7bit����
// ����: pSrc - Դ���봮ָ��
//     nSrcLength - Դ���봮����
// ���: pDst - Ŀ���ַ���ָ��
// ����: Ŀ���ַ�������
***********************************************************************/
int gsmDecode7bit(const unsigned char* pSrc, char* pDst, int nSrcLength)
{
	int nSrc; // Դ�ַ����ļ���ֵ
	int nDst; // Ŀ����봮�ļ���ֵ
	int nByte; // ��ǰ���ڴ���������ֽڵ���ţ���Χ��0-6
	unsigned char nLeft; // ��һ�ֽڲ�������� // ����ֵ��ʼ��
	nSrc = 0;
	nDst = 0;
	
	// �����ֽ���źͲ������ݳ�ʼ��
	nByte = 0;
	nLeft = 0; // ��Դ����ÿ7���ֽڷ�Ϊһ�飬��ѹ����8���ֽ�
	// ѭ���ô�����̣�ֱ��Դ���ݱ�������
	// ������鲻��7�ֽڣ�Ҳ����ȷ����
	while(nSrc<nSrcLength)
	{
		// ��Դ�ֽ��ұ߲��������������ӣ�ȥ�����λ���õ�һ��Ŀ������ֽ�
		*pDst = ((*pSrc << nByte) | nLeft) & 0x7f; // �����ֽ�ʣ�µ���߲��֣���Ϊ�������ݱ�������
		nLeft = *pSrc >> (7-nByte); // �޸�Ŀ�괮��ָ��ͼ���ֵ
		pDst++;
		nDst++; // �޸��ֽڼ���ֵ
		nByte++; // ����һ������һ���ֽ�	  
		if(nByte == 7)
		{
		  // ����õ�һ��Ŀ������ֽ�	 
          *pDst = nLeft;   // �޸�Ŀ�괮��ָ��ͼ���ֵ      
		  pDst++;
		  nDst++;   // �����ֽ���źͲ������ݳ�ʼ��
		  nByte = 0;
		  nLeft = 0;
		} // �޸�Դ����ָ��ͼ���ֵ	   
		pSrc++;
		nSrc++;
	} // ����ַ����Ӹ�������
	*pDst = '\0'; // ����Ŀ�괮����	   
	return nDst;
}

unsigned char ValueToChar(unsigned char ch) 
{
	if(ch >= 0x0A) 
		return(((ch&0x0F) - 0x0A) + 'A');
	else 
		return(ch + '0');
}

unsigned char ValueOfChar(unsigned char ch) 
{
	if(ch >= 'a') 
		return((ch - 'a') + 0x0A);
	else if(ch >= 'A') 
		return((ch - 'A') + 0x0A);
	else 
		return(ch - '0');
}

unsigned int Dchar2Int(unsigned char* s)
{
	unsigned int ret=0;

	ret = ValueOfChar((unsigned char)s[0]);
	ret = ret*16;
	ret += ValueOfChar((unsigned char)s[1]);
	return ret;
}
void strReverse(unsigned char *src,unsigned char *dst,unsigned int srclen)
{
	unsigned int  i= 0;

	if((src==NULL) || (dst==NULL) || (srclen == 0))
	{
	    return;
	}
	while(i < srclen)
	{
	    dst[i] = src[i+1];
	    dst[i+1] = src[i];
	    i+=2;
	}
	if(dst[srclen-1] == 'F' || dst[srclen-1] == 'f')
		dst[srclen-1] = '\0';
	else
		dst[srclen] = '\0';
	return ;
}
void strReverseF(unsigned char *src, unsigned char *dst,unsigned int srclen)
{
	unsigned int i =0;

	if((src==NULL) ||(dst==NULL) || (srclen==0))
	{
	    return;
	}
	while(i <srclen)
	{
	    dst[i/2] = (src[i+1] - '0');
	    dst[i/2] = (dst[i/2] << 4)|(src[i] - '0');
	     i+=2;
	}
	if(srclen %2)
	{
		i -= 2;
		dst[srclen/2] = (src[i] - '0')|0xf0;
	}
	
	return;
}

//�ַ���תPDU
void Str2pdu(unsigned char *str,unsigned int strlen,unsigned char *pdu)
{
	unsigned int  i = 0;
	unsigned char *curr= NULL;

	if((str==NULL) || (strlen==0))
	{
		return;
	}

	curr = pdu;
	for(i = 0;i<strlen;i++)
	{
		*curr++ = ValueToChar((unsigned char)str[i] >> 4);
		*curr++ = ValueToChar((unsigned char)str[i] & 0x0f);
	}
	*curr = '\0';
	
	return ;
}

//PDUת�ַ���
void Pdu2Str(unsigned char *pdu,unsigned char pdulen,unsigned char *str)
{
	unsigned int  i= 0;
	unsigned int ret=0;

	if((pdu==NULL) || (pdulen==0) || (pdulen %2))
 	{
 	   return;
	}

	for(i =0; i< pdulen;i+= 2)
	{
		ret = ValueOfChar((unsigned char)pdu[i]);
		ret = ret*16;
		ret += ValueOfChar((unsigned char)pdu[i+1]);
		str[i/2] = (char)ret;
	}
	return ;
}


//ASCCII��ת7bit��;�����ַ��Ĵ���
unsigned int ASCII2GSM7bit(const unsigned char *pinSms,unsigned int srclen,unsigned char *poutSms)
{
	unsigned int i = 0;
	unsigned int k = 0;
	
	if((poutSms==NULL) || (srclen==0))
	{
		return 0;
	}
	while(srclen --)
	{
		switch(pinSms[k])
		{
			case 0x24://��ASCII��'$' ת��Ϊ7-bit�����'$' 
				poutSms[i ++] = 0x02;
				break;
			case 0x40://��ASCII��'@' ת��Ϊ7-bit�����'@' 
				poutSms[i ++] = 0x00;
				break;
			case 95://��ASCII��'_' ת��Ϊ7-bit�����'_' 
				poutSms[i ++] = 0x11;
				break;
			case 12://from feed
				poutSms[i ++] = 0x1b;
				poutSms[i ++] = 0x0a;
				break;
			case 94://��ASCII��'^' ת��Ϊ7-bit�����'^' 
				poutSms[i ++] = 0x1b;
				poutSms[i ++] = 0x14;
				break;
			case 123://��ASCII��'{' ת��Ϊ7-bit�����'{' 
				poutSms[i ++] = 0x1b;
				poutSms[i ++] = 0x28;
				break;
			case 125://��ASCII��'}' ת��Ϊ7-bit�����'}' 
				poutSms[i ++] = 0x1b;
				poutSms[i ++] = 0x29;
				break;
			case 92://��ASCII��'\' ת��Ϊ7-bit�����'\' 
				poutSms[i ++] = 0x1b;
				poutSms[i ++] = 0x2f;
				break;
			case 91://��ASCII��'[' ת��Ϊ7-bit�����'[' 
				poutSms[i ++] = 0x1b;
				poutSms[i ++] = 0x3c;
				break;
			case 126://��ASCII��'~' ת��Ϊ7-bit�����'~' 
				poutSms[i ++] = 0x1b;
				poutSms[i ++] = 0x3d;
				break;
			case 93://��ASCII��']' ת��Ϊ7-bit�����']' 
				poutSms[i ++] = 0x1b;
				poutSms[i ++] = 0x3e;
				break;
			case 124://��ASCII��'|' ת��Ϊ7-bit�����'|' 
				poutSms[i ++] = 0x1b;
				poutSms[i ++] = 0x40;
				break;
			default:
				poutSms[i ++] = pinSms[k];//����ESC
				break;
		}
		k ++;
	}
	return i;//����ת����ĳ���
}

//7 bit תascii����;�����ַ��Ĵ���
unsigned int GSM7bit2ASCII(const unsigned char *pinSms, unsigned int srclen,unsigned char *poutSms)
{
	unsigned int i = 0;
	unsigned int k = 0;

	if ((pinSms==NULL) || (srclen==0))
	{
	    return 0;
	}
	while(srclen --)
	{
		switch(pinSms[k])
		{
			case 0x02://��7-bit�����'$' ת��ΪASCII��'$' 
			{
				poutSms[i ++] = 0x24;
				break;
			}
			case 0x00://��7-bit�����'@' ת��ΪASCII��'@' 
			{
				poutSms[i ++] = 0x40;
				break;
			}
			case 0x11://��7-bit�����'_' ת��ΪASCII��'_' 
			{
				poutSms[i ++] = 95;
				break;
			}
			case 0x1b:
			{
				switch(pinSms[++k])
				{
					case 0x0a:
					{
						poutSms[i ++] = 12; //from feed
						srclen --;
					}
					case 0x14://��7-bit�����'^' ת��ΪASCII��'^' 
					{
						poutSms[i ++] = 94;
						srclen --;
						break;
					}
					case 0x28://��7-bit�����'{' ת��ΪASCII��'{' 
					{
						poutSms[i ++] = 123;
						srclen --;
						break;
					}
					case 0x29://��7-bit�����'}' ת��ΪASCII��'}' 
					{
						poutSms[i ++] = 125;
						srclen --;
						break;
					}
					case 0x2f://��7-bit�����'\' ת��ΪASCII��'\' 
					{
						poutSms[i ++] = 92;
						srclen --;
						break;
					}
					case 0x3c://��7-bit�����'[' ת��ΪASCII��'[' 
					{
						poutSms[i ++] = 91;
						srclen --;
						break;
					}
					case 0x3d://��7-bit�����'~' ת��ΪASCII��'~' 
					{
						poutSms[i ++] = 126;
						srclen --;
						break;
					}
					case 0x3e://��7-bit�����']' ת��ΪASCII��']' 
						poutSms[i ++] = 93;
						srclen --;
						break;
					case 0x40://��7-bit�����'|' ת��ΪASCII��'|' 
						poutSms[i ++] = 124;
						srclen --;
						break;
					default :
						k --;
						poutSms[i ++] = pinSms[k]; // ESC
						break;
				}
				break;
			}
			default:
				poutSms[i ++] = pinSms[k];
				break;
		}
		k ++;
	}
	poutSms[i] = '\0';
	return i;//����ת����ĳ���
}

