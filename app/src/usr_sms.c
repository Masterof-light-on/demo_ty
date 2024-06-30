#include "includes.h"

static unsigned char sms_status=0;
static unsigned char sms_oatype __noinit__;
static unsigned char sms_phone[MAX_PHONE_NUM] __noinit__;
//接收短信内容
SMSTextMsg sDeliverTextInfo;
//发送短信内容
SMSTextMsg sSubmitTextInfo;
//长短信分包内容
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

//去掉所有空格
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

//串口指令短信格式
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

//短信处理
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
	{//短信指令用超级密码,则不管是否设置授权号码,都接收来自任何号码的命令
		;
	}
	else if(sms_CheckHaveAutor() == 0)
	{//授权号码为空，设备将接受来自任何号码的命令
		;
	}
	else
	{//设置了授权号码,匹配

	}

	//桢头
	psms = (unsigned char *)strstr_t((char *)pDeliverTextInfo->data, "AS");
	if(psms == NULL)
	{
		return;
	}
	//密码
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

	if (sms_status == SMS_DISCONNECT_TCPIP)			//断网   
	{
        modem_send_disconnect(1);
	}
	if (sms_status == SMS_FACTORY)					//恢复出厂
	{
		OS_TaskDelayMs(3000);
		sys_ResetFacitory();
		sys_Restart();
	}
	if (sms_status == SMS_RESTART)                 	//重启  
	{
		OS_TaskDelayMs(3000);
		sys_Restart();
	}
}

//删除所有的短信
void sms_DelAllSms(void)
{
	modem_send_Atcmd("AT+CMGD=1,4","OK",40);
}

//读取短信
void sms_ReadSms(unsigned short index)
{
	unsigned char sendbuf[16]={0};
	unsigned char *buf = modem_getAtBuf();

	sprintf((char *)sendbuf,"AT+CMGR=%d",index);
	modem_send_Atcmd((char *)sendbuf,"OK",100);

	sms_RecvSms(buf, index);
}

//处理短信
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

	// Pdu报文开始
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
		sDeliverTextInfo.data_len = GSM7bit2ASCII(buf,pdul,sDeliverTextInfo.data);    //特殊字符的处理 
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
	
	//删除短信
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

//定时查询短信
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

		// Pdu报文开始
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
			sDeliverTextInfo.data_len = GSM7bit2ASCII(buf,pdul,sDeliverTextInfo.data);    //特殊字符的处理 
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
		//删除短信
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

//发送短信格式   unicode 0 7bit发送   1 unicode发送
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
	if(pSubmitTextInfo->oa)//联通移动编码不一样
	{
		buf[cnt++] = pSubmitTextInfo->oa;
	}
	else
	{
		buf[cnt++] = 0x91; //'+'国际编码
	}
	strReverseF(pSubmitTextInfo->phone_num,buf+cnt,strlen_t((const char *)pSubmitTextInfo->phone_num));     // 电话号码
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
	    Str2pdu(buf, cnt+len, pdu); // 转为PDU
	}

	len += (cnt-1);

	if (sms_SendAtCmd(len,pdu) == RET_AT_SUCCESS)
	{
		return 1;
	}

	return 0;
}

//长短信发送
//totalSMS:总的短信数
//numSMS:具体第几条
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
	if(pSubmitTextInfo->oa)//联通移动编码不一样
	{
		buf[cnt++] = pSubmitTextInfo->oa;
	}
	else
	{
		buf[cnt++] = 0x91; //'+'国际编码
	}
	strReverseF(pSubmitTextInfo->phone_num,buf+cnt,strlen_t((const char *)pSubmitTextInfo->phone_num));     // 电话号码
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
		
		//长短信表示头
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
		//转义特殊字符
	    len = ASCII2GSM7bit(pSubmitTextInfo->data,pSubmitTextInfo->data_len,pdu);
		
		buf[cnt++] = len + 7; //udl

		//长短信表示头
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
	    Str2pdu(buf, cnt+len, pdu); // 转为PDU
	}

	len += (cnt-1);

	if (sms_SendAtCmd(len,pdu) == RET_AT_SUCCESS)
	{
		return 1;
	}

	return 0;
}

//发送短信 (AT命令  发送PDU编码)
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

//删除短信
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

//检测是否有设置授权号码
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

//发送报警短信到指定号码
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
	int nSrc; // 源字符串的计数值
	int nDst; // 目标编码串的计数值
	int nChar; // 当前正在处理的组内字符字节的序号，范围是0-7
	unsigned char nLeft = 0; // 上一字节残余的数据 // 计数值初始化
	nSrc = 0;
	nDst = 0; // 将源串每8个字节分为一组，压缩成7个字节
	// 循环该处理过程，直至源串被处理完
	// 如果分组不到8字节，也能正确处理
	while (nSrc < nSrcLength)
	{
		// 取源字符串的计数值的最低3位
		nChar = nSrc & 7; // 处理源串的每个字节
		if(nChar == 0)
		{
		  // 组内第一个字节，只是保存起来，待处理下一个字节时使用
		  nLeft = *pSrc;
		}
		else
		{
		  // 组内其它字节，将其右边部分与残余数据相加，得到一个目标编码字节
		  *pDst = (*pSrc << (8-nChar)) | nLeft;   // 将该字节剩下的左边部分，作为残余数据保存起来
		  nLeft = *pSrc >> nChar;   // 修改目标串的指针和计数值
		  pDst++;
		  nDst++;
		} // 修改源串的指针和计数值
		pSrc++;
		nSrc++;
	} // 返回目标串长度
    if(nSrcLength%8){
        *pDst = nLeft;
        pDst++;
        nDst++;
    }
	return nDst;
}
/***********************************************************************
// 7bit解码
// 输入: pSrc - 源编码串指针
//     nSrcLength - 源编码串长度
// 输出: pDst - 目标字符串指针
// 返回: 目标字符串长度
***********************************************************************/
int gsmDecode7bit(const unsigned char* pSrc, char* pDst, int nSrcLength)
{
	int nSrc; // 源字符串的计数值
	int nDst; // 目标解码串的计数值
	int nByte; // 当前正在处理的组内字节的序号，范围是0-6
	unsigned char nLeft; // 上一字节残余的数据 // 计数值初始化
	nSrc = 0;
	nDst = 0;
	
	// 组内字节序号和残余数据初始化
	nByte = 0;
	nLeft = 0; // 将源数据每7个字节分为一组，解压缩成8个字节
	// 循环该处理过程，直至源数据被处理完
	// 如果分组不到7字节，也能正确处理
	while(nSrc<nSrcLength)
	{
		// 将源字节右边部分与残余数据相加，去掉最高位，得到一个目标解码字节
		*pDst = ((*pSrc << nByte) | nLeft) & 0x7f; // 将该字节剩下的左边部分，作为残余数据保存起来
		nLeft = *pSrc >> (7-nByte); // 修改目标串的指针和计数值
		pDst++;
		nDst++; // 修改字节计数值
		nByte++; // 到了一组的最后一个字节	  
		if(nByte == 7)
		{
		  // 额外得到一个目标解码字节	 
          *pDst = nLeft;   // 修改目标串的指针和计数值      
		  pDst++;
		  nDst++;   // 组内字节序号和残余数据初始化
		  nByte = 0;
		  nLeft = 0;
		} // 修改源串的指针和计数值	   
		pSrc++;
		nSrc++;
	} // 输出字符串加个结束符
	*pDst = '\0'; // 返回目标串长度	   
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

//字符串转PDU
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

//PDU转字符串
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


//ASCCII码转7bit中途特殊字符的处理
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
			case 0x24://将ASCII的'$' 转换为7-bit编码的'$' 
				poutSms[i ++] = 0x02;
				break;
			case 0x40://将ASCII的'@' 转换为7-bit编码的'@' 
				poutSms[i ++] = 0x00;
				break;
			case 95://将ASCII的'_' 转换为7-bit编码的'_' 
				poutSms[i ++] = 0x11;
				break;
			case 12://from feed
				poutSms[i ++] = 0x1b;
				poutSms[i ++] = 0x0a;
				break;
			case 94://将ASCII的'^' 转换为7-bit编码的'^' 
				poutSms[i ++] = 0x1b;
				poutSms[i ++] = 0x14;
				break;
			case 123://将ASCII的'{' 转换为7-bit编码的'{' 
				poutSms[i ++] = 0x1b;
				poutSms[i ++] = 0x28;
				break;
			case 125://将ASCII的'}' 转换为7-bit编码的'}' 
				poutSms[i ++] = 0x1b;
				poutSms[i ++] = 0x29;
				break;
			case 92://将ASCII的'\' 转换为7-bit编码的'\' 
				poutSms[i ++] = 0x1b;
				poutSms[i ++] = 0x2f;
				break;
			case 91://将ASCII的'[' 转换为7-bit编码的'[' 
				poutSms[i ++] = 0x1b;
				poutSms[i ++] = 0x3c;
				break;
			case 126://将ASCII的'~' 转换为7-bit编码的'~' 
				poutSms[i ++] = 0x1b;
				poutSms[i ++] = 0x3d;
				break;
			case 93://将ASCII的']' 转换为7-bit编码的']' 
				poutSms[i ++] = 0x1b;
				poutSms[i ++] = 0x3e;
				break;
			case 124://将ASCII的'|' 转换为7-bit编码的'|' 
				poutSms[i ++] = 0x1b;
				poutSms[i ++] = 0x40;
				break;
			default:
				poutSms[i ++] = pinSms[k];//包括ESC
				break;
		}
		k ++;
	}
	return i;//返回转换后的长度
}

//7 bit 转ascii码中途特殊字符的处理
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
			case 0x02://将7-bit编码的'$' 转换为ASCII的'$' 
			{
				poutSms[i ++] = 0x24;
				break;
			}
			case 0x00://将7-bit编码的'@' 转换为ASCII的'@' 
			{
				poutSms[i ++] = 0x40;
				break;
			}
			case 0x11://将7-bit编码的'_' 转换为ASCII的'_' 
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
					case 0x14://将7-bit编码的'^' 转换为ASCII的'^' 
					{
						poutSms[i ++] = 94;
						srclen --;
						break;
					}
					case 0x28://将7-bit编码的'{' 转换为ASCII的'{' 
					{
						poutSms[i ++] = 123;
						srclen --;
						break;
					}
					case 0x29://将7-bit编码的'}' 转换为ASCII的'}' 
					{
						poutSms[i ++] = 125;
						srclen --;
						break;
					}
					case 0x2f://将7-bit编码的'\' 转换为ASCII的'\' 
					{
						poutSms[i ++] = 92;
						srclen --;
						break;
					}
					case 0x3c://将7-bit编码的'[' 转换为ASCII的'[' 
					{
						poutSms[i ++] = 91;
						srclen --;
						break;
					}
					case 0x3d://将7-bit编码的'~' 转换为ASCII的'~' 
					{
						poutSms[i ++] = 126;
						srclen --;
						break;
					}
					case 0x3e://将7-bit编码的']' 转换为ASCII的']' 
						poutSms[i ++] = 93;
						srclen --;
						break;
					case 0x40://将7-bit编码的'|' 转换为ASCII的'|' 
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
	return i;//返回转换后的长度
}

