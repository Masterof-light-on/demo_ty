#include "includes.h"

const unsigned char gCarLicense_table[][4] = {
	"京","津","沪","渝","冀","云","辽","黑","湘","皖",
	"鲁","新","苏","浙","赣","鄂","桂","甘","晋","蒙",
	"陕","吉","闽","贵","粤","青","藏","川","宁","琼",
	"警","豫",
};

unsigned char gc_debug = 0;
unsigned char gc_watchdog = 0;

void handset_Init(void)
{
	
}

//handset数据处理函数
void handset_handle(unsigned char com_type, unsigned char *data, unsigned short dataLen)
{
	handset_SetSchedul(com_type,data, dataLen);
}

unsigned char sSave_flag = 0;
unsigned char sReppp_flag = 0;
unsigned char sReset_flag = 0;

//apn
int handset_cmd_apn(unsigned char *str, unsigned char *out)
{	
	if(handset_SetApn(&str[2]) == 1)
	{
		sSave_flag |= SAVE_SYS_FLAG;
		sReppp_flag = 1;
		handset_CheckParam(out);
	}
	else
	{
		strcpy_t((char *)out,"SET APN FORMAT ERR");
		
		return 0;
	}
	return 1;
}

//重启
int handset_cmd_reboot(unsigned char *str, unsigned char *out)
{	
	sprintf((char *)out,"VIN:%s,Restart",sys_GetCarVin());
	sReset_flag = 1;

	return 1;
}

//查询ip参数
int handset_cmd_showparam(unsigned char *str, unsigned char *out)
{
	//*F10#CC
	if((str[0] == 'C')&&(str[1] == 'C'))
	{
		handset_CheckParamAll(out);
	}
	else
	{
		handset_CheckParam(out);
	}

	return 1;
}

//工程模式
int handset_cmd_debugmode(unsigned char *str, unsigned char *out)
{
	gc_debug = 1;
	
	return 1;
}

//上传间隔
int handset_cmd_interval(unsigned char *str, unsigned char *out)
{
	if(strstr_t((char *)str, "GPS"))
	{
		strcpy_t((char *)out, (char *)gps_GetGprmc());
	}
	else
	{
		if(handset_SetAccOnOffInterval(&str[2]) == 1)
		{
			sSave_flag |= SAVE_RUN_FLAG;
		   	handset_CheckRun(out);
		}
		else
		{
			strcpy_t((char *)out,"SET INTERVAL FORMAT ERR");
			
			return 0;
		}
	}

	return 1;
}

//IP2在线时间
//*F10#IP2,x  x:min
int handset_cmd_ip2keepalive(unsigned char *str, unsigned char *out)
{
	if(strstr_t((char *)str, "IP2,"))
	{
		sys_SetIP2KeepAlive(myatoi((char *)&str[4]));
		sSave_flag |= SAVE_RUN_FLAG;
	   	handset_CheckRun(out);
	}
	else
	{
		strcpy_t((char *)out,"SET IP2 FORMAT ERR");
		
		return 0;
	}

	return 1;
}

//拐点
int handset_cmd_redeem(unsigned char *str, unsigned char *out)
{	
	if(handset_SetRedeem(&str[2]) == 1)
	{
		sSave_flag |= SAVE_RUN_FLAG;
	   	handset_CheckRun(out);
	}
	else
	{
		strcpy_t((char *)out,"SET REDEEM FORMAT ERR");
		
		return 0;
	}

	return 1;
}

//恢复出厂
int handset_cmd_reset(unsigned char *str, unsigned char *out)
{
	sys_ResetFacitory();
	sprintf((char *)out,"VIN:%s,Reset",sys_GetCarVin());
	sReset_flag = 1;

	return 1;
}

//查询运行参数
int handset_cmd_showrun(unsigned char *str, unsigned char *out)
{	
	handset_CheckRun(out);
	
	return 1;
}

//自动接听设置
int handset_cmd_autoata(unsigned char *str, unsigned char *out)
{	
	if(handset_SetATAParam(&str[2]) == 1)
	{
		sSave_flag |= SAVE_RUN_FLAG;
		handset_CheckRun(out);
	}
	else
	{
		strcpy_t((char *)out,"SET ATA FORMAT ERR");
		
		return 0;
	}

	return 1;
}


//里程系数
int handset_cmd_mileage(unsigned char *str, unsigned char *out)
{	
	if(handset_SetMileage(&str[2]) == 1)
	{
		sSave_flag |= SAVE_RUN_FLAG;
		sprintf((char *)out,"rate:%d",sys_GetageNum());
	}
	else
	{
		strcpy_t((char *)out,"SET MILEAGE FORMAT ERR");
		
		return 0;
	}

	return 1;
}

//位置信息应答使能开关
int handset_cmd_ack(unsigned char *str, unsigned char *out)
{
	unsigned char ack=0;

	str += 2;
	ack = str[0] - '0';
	sys_SetAckSwitch(ack);
	sSave_flag |= SAVE_SYS_FLAG;
	handset_CheckRun(out);
	
	return 1;
}

//VIN
int handset_cmd_vin(unsigned char *str, unsigned char *out)
{	
	if(handset_SetVin(&str[2]) == 1)
	{
		MsgGsm_Clear();
		sSave_flag |= SAVE_SYS_FLAG;
		sReppp_flag = 1;
		handset_CheckParam(out);
	}
	else
	{
		strcpy_t((char *)out,"SET VIN FORMAT ERR");
		
		return 0;
	}

	return 1;
}

//ID
int handset_cmd_id(unsigned char *str, unsigned char *out)
{	
	if(handset_SetId(&str[2]) == 1)
	{
		sSave_flag |= SAVE_SYS_FLAG;
		sReppp_flag = 1;
		handset_CheckParam(out);
	}
	else
	{
		strcpy_t((char *)out,"SET ID FORMAT ERR");
		
		return 0;
	}

	return 1;
}

//SN
int handset_cmd_sn(unsigned char *str, unsigned char *out)
{	
	if(handset_SetSn(&str[2]) == 1)
	{
		sys_saveSN();
		sSave_flag |= SAVE_SYS_FLAG;
		sReppp_flag = 1;
		handset_CheckParamAll(out);
	}
	else
	{
		strcpy_t((char *)out,"SET ID FORMAT ERR");
		
		return 0;
	}

	return 1;
}

//密码修改
int handset_cmd_password(unsigned char *str, unsigned char *out)
{
	str += 2;
	str[4] = 0;
	sys_SetTerminalPwd(str);
	sSave_flag |= SAVE_RUN_FLAG;

	sprintf((char *)out,"new password:%s",sys_GetTerminalPwd());

	return 1;
}

//*F10#U:122.114.126.6,8898,F300A.bin,
int handset_cmd_upgrade(unsigned char *str, unsigned char *out)
{
	if(modem_GetState() == GSM_FOTA_STATE)
	{
		strcpy_t((char *)out,"OTAing");
		
		return 1;
	}
	if(handset_SetUpgrade(&str[2]) == 1)
	{

		if(g_flag.bit.socketPPPFlag == 1)
		{
			g_flag.bit.MutexFlag = 0;
			g_flag.bit.otaWaitFlag = 0;
			OS_Timer_Stop(GSM_TIMER_ID);
			xQueueReset(xGsmQueueId);
			update_SetUpdateFlag(1);
			update_SetTFTPFlag(0);
			modem_SetState(GSM_FOTA_STATE);
			OS_SendMessage(xGsmQueueId, MSG_GSM_UPDATE_START, 0, 0);
		}
		else
		{
			g_flag.bit.otaWaitFlag = 1;
		}

		sprintf((char *)out,"OTA Start:%s,%d,%s",update_GetIp(),(unsigned short)update_GetPort(),update_GetFilename());
	}
	else
	{
		strcpy_t((char *)out,"SET OTA FORMAT ERR");
	}

	return 1;
}

int handset_ota_waiting(void)
{
	if(g_flag.bit.otaWaitFlag == 1)
	{
		g_flag.bit.otaWaitFlag = 0;
		g_flag.bit.MutexFlag = 0;
		OS_Timer_Stop(GSM_TIMER_ID);
		xQueueReset(xGsmQueueId);
		update_SetUpdateFlag(1);
		update_SetTFTPFlag(0);
		modem_SetState(GSM_FOTA_STATE);
		OS_SendMessage(xGsmQueueId, MSG_GSM_UPDATE_START, 0, 0);

		return 1;
	}

	return 0;
}

//授权号码
int handset_cmd_phone(unsigned char *str, unsigned char *out)
{
	if(handset_SetAuthorPhone(&str[2]) == 1)
	{
		sSave_flag |= SAVE_RUN_FLAG;
	   	sprintf((char *)out,"phone:%s,%s,%s,%s",sys_GetAuthorPhone(0), sys_GetAuthorPhone(1), 
			sys_GetAuthorPhone(2),sys_GetAuthorPhone(3));
	}
	else
	{
		strcpy_t((char *)out,"SET PHONE FORMAT ERR");
		
		return 0;
	}

	return 1;
}

//时区
int handset_cmd_timezone(unsigned char *str, unsigned char *out)
{	
	if(handset_SetTimeZone(&str[2]) == 1)
	{
		sSave_flag |= SAVE_SYS_FLAG;
		sprintf((char *)out,"timezone:%c%d",sys_GetTimeType(),sys_GetTimeZone());
	}
	else
	{
		strcpy_t((char *)out,"SET TIMEZONE FORMAT ERR");
		
		return 0;
	}

	return 1;
}

//IP/URL
int handset_cmd_ip1(unsigned char *str, unsigned char *out)
{	
	if(handset_SetIP1(&str[2]) == 1)
	{
		sSave_flag |= SAVE_SYS_FLAG;
		sReppp_flag = 1;
		handset_CheckParam(out);
	}
	else
	{
		strcpy_t((char *)out,"SET IP/URL FORMAT ERR");
		
		return 0;
	}

	return 1;
}

int handset_cmd_ip2(unsigned char *str, unsigned char *out)
{	
	if(handset_SetIP2(&str[2]) == 1)
	{
		sSave_flag |= SAVE_SYS_FLAG;
		sReppp_flag = 1;
		handset_CheckParam(out);
	}
	else
	{
		strcpy_t((char *)out,"SET IP/URL FORMAT ERR");
		
		return 0;
	}

	return 1;
}

//查询版本号
int handset_cmd_version(unsigned char *str, unsigned char *out)
{
	app_GetVersion(out);

	return 1;
}

//设置CAN波特率
//*F10#X,can1,can2,
int handset_cmd_canbaud(unsigned char *str, unsigned char *out)
{
	if(handset_SetCanbaud(&str[2]) == 1)
	{
		sSave_flag |= SAVE_SYS_FLAG;
		sReset_flag = 1;
		handset_CheckRun(out);
	}
	else
	{
		strcpy_t((char *)out,"SET CAN FORMAT ERR");
		
		return 0;
	}

	return 1;
}

//带*的指令
int handset_cmd_other(unsigned char *str, unsigned char *out)
{
	str ++;
	switch(str[0])
	{
		case '6'://*F10#*6,x,y,z   x=rs232,y=rs485
		{
			if(handset_SetDevice(&str[2]) == 1)
			{
				sSave_flag |= SAVE_SYS_FLAG;
				sReset_flag = 1;
				handset_CheckRun(out);
			}
			else
			{
				strcpy_t((char *)out,"SET DEVICE FORMAT ERR");
			}
		}
		break;
		
		case 'D':
		{
			unsigned char log=0;

			log = str[1] - '0';
			sys_set_debugSwitch(log);
			strcpy_t((char *)out, "LOG OK");
		}
		break;

		case 'T':
		{
			sprintf((char *)out, "IMEI:%s", modem_GetImei());
		}
		break;
		
		default:
		break;
	}
	
	return 1;
}

int handset_cmd_testmode(unsigned char *str, unsigned char *out)
{
	str ++;
	if(str[0] == '1')
	{
		gc_watchdog = 1;
	}
	else if(str[0] == '2')
	{
		unsigned char i=0;
		unsigned char phone[PHONE_LEN]={0};
		
		str += 2;

		while((*str != ';')&&(i < PHONE_LEN))
		{
			phone[i++] = *str++;
		}
		if(modem_CallPhone(phone, 1))
		{
			OS_SendMessage(xGsmQueueId, MSG_GSM_CALL, MSG_GSM_RING, 0);
		}
	}
	
	return 1;
}

//*F10#[AT+....]
int handset_cmd_at(unsigned char *str, unsigned char *out)
{
	unsigned char *p=NULL;

	str ++;
	p = (unsigned char*)strchr_t((const char *)str, ']');
	if(p == NULL)
	{
		return 0;
	}
	*p = 0;
	
	modem_send_Atcmd((char *)str,"OK",10);
	
	return 1;
}

typedef struct
{
	unsigned char  *cmd;
	int (*cmd_handler)(unsigned char *str, unsigned char *out);
}CMD_T;


static CMD_T uart_cmd_proc[] =
{
	{"0", handset_cmd_ip2},
	{"1", handset_cmd_ip1},
	{"2", handset_cmd_id},
	{"3", handset_cmd_apn},
	{"4", handset_cmd_password},
	{"5", handset_cmd_phone},
	{"6", handset_cmd_vin},
	{"A", handset_cmd_autoata},
	{"B", handset_cmd_reboot},
	{"C", handset_cmd_showparam},
	{"D", handset_cmd_debugmode},
	{"F", handset_cmd_reset},
	{"G", handset_cmd_interval},
	{"I", handset_cmd_ip2keepalive},
	{"L", handset_cmd_mileage},
	{"M", handset_cmd_ack},
	{"R", handset_cmd_showrun},
	{"S", handset_cmd_sn},
	{"U", handset_cmd_upgrade},
	{"V", handset_cmd_version},
	{"X", handset_cmd_canbaud},
	{"*", handset_cmd_other},
	{"@", handset_cmd_testmode},
	{"[", handset_cmd_at},
};

static const int MAX_CMD = (sizeof(uart_cmd_proc)/sizeof(uart_cmd_proc[0]));

//*F10#...
int handset_all_command(unsigned char *str, unsigned short len, unsigned char *out)
{
	unsigned char *pCmd=NULL;
	unsigned char i=0;

	pCmd = str;

	for(i=0; i<MAX_CMD; i++)
	{
		if(pCmd[0] == uart_cmd_proc[i].cmd[0])
		{
			if(uart_cmd_proc[i].cmd_handler(pCmd, out+strlen_t((char *)out)) == 1)
			{
				break;
			}
			else
			{
				return 0;
			}
		}
	}

	return 1;
}

//*#1234#
//恢复到出厂程序
int handset_restore_command(unsigned char *str, unsigned short len, unsigned char *out)
{
	unsigned char buf[5];
	
	buf[0] = 0;
	buf[1] = 0;
	buf[2] = 0; 
	buf[3] = 0;
	buf[4] = 0x55;

	flash_Write(PAGE_UPDATE_END,5,buf);
	strcpy_t((char *)out,"restore app");
	sReset_flag = 1;
			
	return 1;
}

//*HAEP#
int handset_heap_command(unsigned char *str, unsigned short len, unsigned char *out)
{
	sprintf((char *)out, "FreeStackSize:%d  FreeHeapSize:%d\r\n", uxTaskGetStackHighWaterMark(NULL), xPortGetFreeHeapSize());
	
	return 1;
}

typedef struct
{
	unsigned char  *cmd;
	int (*cmd_handler)(unsigned char *str, unsigned short len, unsigned char *out);
}ALL_T;

static ALL_T uart_all_cmd_proc[] =
{
	{"*F10#",		handset_all_command},
	{"AT@",			handset_all_pccmd},
	{"*#1234#",		handset_restore_command},
	{"*HEAP#",		handset_heap_command},
};

static const int MAX_ALL = (sizeof(uart_all_cmd_proc)/sizeof(uart_all_cmd_proc[0]));


//设置参数
unsigned char handset_ConfigSet(unsigned char* pdata, unsigned short len, unsigned char *pOut)
{
	unsigned char i=0;
	
	sSave_flag  = 0;
	sReset_flag = 0;
	sReppp_flag = 0;

	for(i=0; i<MAX_ALL; i++)
	{
		if(strnicmp_t((char *)pdata, (char *)uart_all_cmd_proc[i].cmd, strlen_t((char *)uart_all_cmd_proc[i].cmd)) == 0)
		{
			uart_all_cmd_proc[i].cmd_handler(pdata+strlen_t((char *)uart_all_cmd_proc[i].cmd),len,pOut);
			break;
		}
	}

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
	   	return PC_RESTART;
	}

	if(sReppp_flag == 1)
	{
		return PC_DISCONNECT_TCPIP; 
	}

	return PC_NULL; 
}

//串口信息
void handset_SetSchedul(unsigned char com_type,unsigned char *buf, unsigned short dataLen)
{
	unsigned char ret=0;
	unsigned char rbuf[320]={0};

	ret = handset_ConfigSet(buf,dataLen,rbuf);
	if(strlen_t((char *)rbuf) > 0)
	{
		switch(com_type)
		{
			case COM0:
			{
				
			}
			break;
			case COM3:
			{
				
			}
			break;
			case COM4:
			{
				
			}
			break;
			case USBCOM:
			{
				Log_Lock(xLogMutex);
				PRINTF("[ack] %s\r\n\r\n", rbuf);//在串口上打印
				Log_UnLock(xLogMutex);
			}
			break;
		}
		
	}

	if(ret == PC_DISCONNECT_TCPIP)
	{
        modem_send_disconnect(1);
	}
	else if(ret == PC_RESTART)
	{
		sys_Restart();
	}
	else{;}	
}

//设置ip/url--ip/url,port,linktype
unsigned char handset_SetIP1(unsigned char* pdata)
{
	unsigned int port=0;
	unsigned char ipbuf[URL_LEN]={0};
	unsigned char linktype=0;
	
	unsigned char flag=0;
	unsigned char *ps=NULL;

	if(pdata == NULL)
	{
		return 0;
	}

	ps = (unsigned char *)strchr_t((const char *)pdata,',');
	if(ps == NULL)
	{
	   	return 0;
	}
	
	if(is_ip_address((char *)pdata,(ps-pdata)) == 0)
	{
		flag |= 0x01;
		sys_ClearUrl();		
	}
	else
	{
		flag |= 0x02;
		sys_ClrIp();
	}
	if(ps-pdata > URL_LEN)
	{
		return 0;
	}
	ps[0] = 0;
	strcpy_t((char *)ipbuf,(char *)pdata);
	
	pdata = ps+1;
	ps = (unsigned char *)strchr_t((const char *)pdata,',');
	if(ps == NULL)
	{
	    return 0;
	}
	ps[0] = 0;
	port = myatoi((char *)pdata);//端口
	if(port > 65535)
	{
	    return 0;
	}
	
	ps++;
	linktype = ps[0]-'0';
	if (!((linktype==0) || (linktype==1)))//连接方式  0 是UDP  1 TCP
	{
		return 0;
	}

	if((flag&0x01) == 0x01)
	{
		sys_SetIp(ipbuf);
	}
	else
	{
		sys_SetUrl(ipbuf);
	}
	sys_SetPort((unsigned short)port);
	sys_SetLinkType(linktype);
	
	return 1;
}

unsigned char handset_SetIP2(unsigned char* pdata)
{
	unsigned int port=0;
	unsigned char ipbuf[URL_LEN]={0};
	unsigned char linktype=0;
	
	unsigned char flag=0;
	unsigned char *ps=NULL;

	if(pdata == NULL)
	{
		return 0;
	}

	ps = (unsigned char *)strchr_t((const char *)pdata,',');
	if(ps == NULL)
	{
	   	return 0;
	}
	
	if(is_ip_address((char *)pdata,(ps-pdata)) == 0)
	{
		flag |= 0x01;
		sys_ClearUrlBak();		
	}
	else
	{
		flag |= 0x02;
		sys_ClrIpBak();
	}
	if(ps-pdata > URL_LEN)
	{
		return 0;
	}
	ps[0] = 0;
	strcpy_t((char *)ipbuf,(char *)pdata);
	
	pdata = ps+1;
	ps = (unsigned char *)strchr_t((const char *)pdata,',');
	if(ps == NULL)
	{
	    return 0;
	}
	ps[0] = 0;
	port = myatoi((char *)pdata);//端口
	if(port > 65535)
	{
	    return 0;
	}
	
	ps++;
	linktype = ps[0]-'0';
	if (!((linktype==0) || (linktype==1)))//连接方式  0 是UDP  1 TCP
	{
		return 0;
	}

	if((flag&0x01) == 0x01)
	{
		sys_SetIpBak(ipbuf);
	}
	else
	{
		sys_SetUrlBak(ipbuf);
	}
	sys_SetPortBak((unsigned short)port);
	sys_SetLinkTypeBak(linktype);
	
	return 1;
}

//设置vin
//*F10#6,车牌号,17位VIN[,静态ip,网关,掩码,]
unsigned char handset_SetVin(unsigned char *data)
{
	unsigned char *pFind=NULL;
	
	if(data == NULL)
	{ 
		return 0;
	}

	pFind = (unsigned char *)strchr_t((char *)data, ',');
	if(pFind)
	{
		pFind[0] = 0;	
		
		data = ++pFind;
		if(data[17] == ',')
		{
			data[17] = 0;
			
			sys_SetCarVin(data);

		}
		else
		{
			data[17] = 0;
			
			sys_SetCarVin(data);
		}
	}
	else
	{
		return 0;
	}
		
	return 1; 
}

//设置id
//*F10#2,17位
unsigned char handset_SetId(unsigned char *data)
{	
	if(data == NULL)
	{ 
		return 0;
	}

	if(strlen_t((char *)data) != 17)
	{
		return 0;
	}
	data[17] = 0;
	
	sys_SetCarVin(data);
		
	return 1; 
}

//设置sn
//*F10#SN,17位
unsigned char handset_SetSn(unsigned char *data)
{	
	if(data == NULL)
	{ 
		return 0;
	}

	if(strlen_t((char *)data) != 17)
	{
		return 0;
	}
	data[17] = 0;
	
	sys_SetTermSN(data);
	
	return 1; 
}

//设置apn
unsigned char handset_SetApn(unsigned char* pdata)
{
	unsigned char i=0,j=0,len=0;
	unsigned char *ps=NULL,*pt=NULL;
	unsigned char apn[APN_LEN]={0},apnuser[APN_LEN]={0},apnpasswd[APN_LEN]={0};

    if (pdata == NULL)
    {
        return 0;
    }
	
    memset_t((char *)apn,0,sizeof(apn));
    memset_t((char *)apnuser,0,sizeof(apnuser));
    memset_t((char *)apnpasswd,0,sizeof(apnpasswd));
    for(i=0; i<3; i++)
    {
		ps = (unsigned char *)strchr_t((const char *)pdata,',');
		if(ps == NULL)
		{
			return 0;
		}
		ps[0] = 0;
		len = ps-pdata;			
		if(len > APN_LEN)//最大的长度
		{
			return 0;
		}
	  
		pt = pdata;	 
		for(j=0; j<len; j++)        
		{
			if(pt[j] == '&')
			{
				pt[j] = '@';
			}
			pt++;
		}

		if (i==0)
		{
			memcpy_t(apn,pdata,len);
		}
		if (i==1)
		{
			memcpy_t(apnuser,pdata,len);
		}
		if (i==2)
		{
			memcpy_t(apnpasswd,pdata,len);
		}
		pdata = ps+1;
	}
	
	sys_SetApn(apn);
	sys_SetApnUser(apnuser);
	sys_SetApnPasswd(apnpasswd);	
	
	return 1;
}

//查询参数
void handset_CheckParam(unsigned char *pOut)
{
	unsigned char id[20] = {0};
	sys_GetId2(id);
	sprintf((char *)pOut,"IP1:%s%s,%d,%d,IP2:%s%s,%d,%d,%s,%s,%s,ID:%s,M:%d,GPS:%s,%d,CSQ:%d,CGR:%d,ACC:%d,%d,%d,POW:%d,%s",
		sys_GetUrl(),sys_GetIp(),sys_GetPort(),sys_GetLinkType(),\
		sys_GetUrlBak(),sys_GetIpBak(),sys_GetPortBak(),sys_GetLinkTypeBak(),\
		sys_GetApn(),sys_GetApnUser(),sys_GetApnPasswd(),id,modem_GetState(),\
		sys_state_get_gpsloc()?"A":"V",gps_GetGpsStarNum(),modem_GetCsq(),\
		modem_GetCgreg(),sys_state_get_acc()?1:0,sys_GetAccStateOnInterval(),sys_GetAccStateOffInterval(),sys_state_get_power()?0:1,FIRMWARE_STRING);
}

void handset_CheckParamAll(unsigned char *pOut)
{
	unsigned char id[20] = {0};
	sys_GetId2(id);
	sprintf((char *)pOut,"IP1:%s%s,%d,%d,IP2:%s%s,%d,%d,%s,%s,%s,ID:%s,IMEI:%s,SN:%s,M:%d,GPS:%s,%d,CSQ:%d,CGR:%d,ACC:%d,%d,%d,POW:%d,%s",
		sys_GetUrl(),sys_GetIp(),sys_GetPort(),sys_GetLinkType(),\
		sys_GetUrlBak(),sys_GetIpBak(),sys_GetPortBak(),sys_GetLinkTypeBak(),\
		sys_GetApn(),sys_GetApnUser(),sys_GetApnPasswd(),id,modem_GetImei(),sys_GetTermSN(),\
		modem_GetState(),\
		sys_state_get_gpsloc()?"A":"V",gps_GetGpsStarNum(),modem_GetCsq(),\
		modem_GetCgreg(),sys_state_get_acc()?1:0,sys_GetAccStateOnInterval(),sys_GetAccStateOffInterval(),\
		sys_state_get_power()?0:1,FIRMWARE_STRING);
}

//查询参数
void handset_CheckRun(unsigned char *pOut)
{		
	unsigned char device[20]={0};
	
	handset_CheckDevice(device);
	sprintf((char*)pOut,"accOn:%d accOff:%d rd:%d z:%c%d ack:%d can:%d,%d gsm:%d,%d %s",
		sys_GetAccStateOnInterval(),sys_GetAccStateOffInterval(),sys_GetRedeem(),\
		sys_GetTimeType(),sys_GetTimeZone(),sys_GetAckSwitch(),sys_GetCan1Baud(),sys_GetCan2Baud(),\
		MsgGsm_GetFlashIn(),MsgGsm_GetFlashOut(),device);
}

//设置授权号码
unsigned char handset_SetAuthorPhone(unsigned char *data)
{
	unsigned char i=0,j=0,len=0;
	unsigned char no[PHONE_NUM][PHONE_LEN]={0};
	unsigned char *p=NULL;

	if (data == NULL)
	{
		return 0;
	}
	
	sys_ClrAuthorPhone();

	for (i=0;i<PHONE_NUM;i++)
	{
		p = (unsigned char *)strchr_t((const char *)data,',');
		if (p != NULL)
		{
			p[0] = 0;
			len = p-data; 
			if ((len>=15)||((len==1)&&(data[0] !='0')))
			{
				return 0;
			} 
			memset_t((char *)no[i],0,PHONE_LEN);
			if(len > 1)
			{
				memcpy_t(no[i],data,len);
			}
		}
		else
		{
			if (i == 0)
			{
				memcpy_t(no[i],data,11);
				sys_SetAuthorPhone(i,no[i]);
			}
			break; 
		}
		data = p+1;
	}
	
	for (j=0;j<i;j++)
	{ 
		sys_SetAuthorPhone(j,no[j]);  
	}
	
	return 1;    
}

//设置ACC 开关时时间间隔
unsigned char handset_SetAccOnOffInterval(unsigned char *data)
{
	unsigned char *p=NULL;
	unsigned short accon=0,accoff=0;

	if(data == NULL)
	{
		return 0;
	}

	p = (unsigned char*)strchr_t((const char *)data, ',');
	if(p == NULL)
	{
		return 0;
	}

	accon = myatoi((char *)data);

	data = p+1;
	p = (unsigned char*)strchr_t((const char *)data, ',');
	if(p == NULL)
	{
		return 0;
	}
	accoff = myatoi((char *)data);

	if((accon > 65535)||(accoff > 65535))
	{
		return 0;
	}
	sys_SetWatchType(1);
	sys_SetAccStateOnInterval(accon);
	sys_SetAccStateOffInterval(accoff);

	return 1;
}

//设置拐点
unsigned char handset_SetRedeem(unsigned char *data)
{
	unsigned char *p=NULL;
	unsigned char redeem=0;

	if(data == NULL)
	{
		return 0;
	}

	p = (unsigned char*)strchr_t((const char *)data, ',');
	if(p == NULL)
	{
		return 0;
	}

	redeem = myatoi((char *)data);

	sys_SetRedeem(redeem);

	return 1;
}

//自动接听设置
unsigned char handset_SetATAParam(unsigned char *data)
{
	unsigned char time=0;

	if(data == NULL)
	{
		return 0;
	}
	
	time = myatoi((char *)data);
	sys_SetAutoAnswerTime(time);

	return 1;
}

//设置里程初始化
unsigned char handset_SetMileage(unsigned char *data)
{
	unsigned char coefficient=0;

	if(data == NULL)
	{
		return 0;
	}

	if(data[0] == '0')
	{
		;
	}
	else if(data[0] == '1')
	{
		data += 2;
		
		coefficient = myatoi((char*)data);
		sys_SetageNum(coefficient);
	}
	else
	{
		return 0;
	}

	return 1;
}

//设置CAN波特率
unsigned char handset_SetCanbaud(unsigned char *data)
{
	unsigned char *p=NULL;
	unsigned short baud1,baud2;

	if(data == NULL)
	{
		return 0;
	}

	p = (unsigned char*)strchr_t((const char *)data, ',');
	if(p == NULL)
	{
		return 0;
	}
	*p = 0;
	baud1 = myatoi((char *)data);
	if((baud1 != 125)&&(baud1 != 250)&&(baud1 != 500))
	{
		return 0;
	}

	data = p+1;
	p = (unsigned char*)strchr_t((const char *)data, ',');
	if(p == NULL)
	{
		return 0;
	}
	*p = 0;
	baud2 = myatoi((char *)data);
	if((baud2 != 125)&&(baud2 != 250)&&(baud2 != 500))
	{
		return 0;
	}

	sys_SetCan1Baud(baud1);
	sys_SetCan2Baud(baud2);

	return 1;
}


//设置远程升级
unsigned char handset_SetUpgrade(unsigned char *data)
{
	unsigned char *p=NULL;
	unsigned char *pEnd=NULL;
	unsigned short port=0;

	if (data == NULL)
	{
		return 0;
	}

	p = (unsigned char *)strchr_t((const char *)data,',');
	if (p == NULL)
	{
		return 0;
	}
	p[0] = 0;
	p++;
	port = myatoi((char*)p);

	p = (unsigned char *)strchr_t((const char *)p,',');
	if (p == NULL)
	{
		return 0;
	}
	p++;
	
	pEnd = (unsigned char *)strchr_t((const char *)p,',');
	if (pEnd == NULL)
	{
		return 0;
	}
	pEnd[0] = 0;
	
	if (!update_SetIp(data, strlen_t((char *)data)))
	{
	 	return 0;
	}
	if (!update_SetPort(port))
	{
	 	return 0;
	}
	if (!update_SetFilename(p))
	{
		return 0;
	}

	return 1;	
}

//设置时区
unsigned char handset_SetTimeZone(unsigned char *data)
{
	char ch=0;
	
	if (data == NULL)
	{
		return 0;
	}
	
	if(data[0] == 'W')
	{
		sys_SetTimeType('-');
	}
	else
	{
		sys_SetTimeType('+');
	}

	data++;
	ch = (unsigned char)myatoi((char*)data);
	if(ch > 13)
	{
		return 0;
	}
	
	sys_SetTimeZone(ch);
	
	return 1;   
}

//查询外设功能
void handset_CheckDevice(unsigned char *pOut)
{
	unsigned char com1,com2,com3;

	if((sys_GetUartFunc(0)&URT_MASK_JT_OIL) == URT_MASK_JT_OIL)
	{
		com1 = 1;	
	}
	else if((sys_GetUartFunc(0)&URT_MASK_CR_OIL) == URT_MASK_CR_OIL)
	{
		com1 = 2;
	}
	else if((sys_GetUartFunc(0)&URT_MASK_XD_OIL) == URT_MASK_XD_OIL)
	{
		com1 = 3;	
	}
	else if((sys_GetUartFunc(0)&URT_MASK_BT) == URT_MASK_BT)
	{
		com1 = 4;	
	}
	else
	{
		com1 = 0;
	}

	if((sys_GetUartFunc(1)&URT_MASK_JT_OIL) == URT_MASK_JT_OIL)
	{
		com2 = 1;	
	}
	else if((sys_GetUartFunc(1)&URT_MASK_CR_OIL) == URT_MASK_CR_OIL)
	{
		com2 = 2;
	}
	else if((sys_GetUartFunc(1)&URT_MASK_XD_OIL) == URT_MASK_XD_OIL)
	{
		com2 = 3;	
	}
	else if((sys_GetUartFunc(1)&URT_MASK_BT) == URT_MASK_BT)
	{
		com2 = 4;	
	}
	else
	{
		com2 = 0;
	}
    

	if((sys_GetUartFunc(2)&URT_MASK_JT_OIL) == URT_MASK_JT_OIL)
	{
		com3 = 1;	
	}
	else if((sys_GetUartFunc(2)&URT_MASK_CR_OIL) == URT_MASK_CR_OIL)
	{
		com3 = 2;
	}
	else if((sys_GetUartFunc(2)&URT_MASK_XD_OIL) == URT_MASK_XD_OIL)
	{
		com3 = 3;
	}
	else if((sys_GetUartFunc(2)&URT_MASK_BT) == URT_MASK_BT)
	{
		com3 = 4;
	}
	else
	{
		com3 = 0;
	}

	sprintf((char *)pOut, "232:%d,%d 485:%d",com1,com2,com3);
}

//设置外设功能指令
unsigned char handset_SetDevice(unsigned char *data)
{
	unsigned char *p=NULL;
	unsigned int func[3]={0};

	if (data == NULL)
	{
		return 0;
	}

	p = (unsigned char *)strchr_t((const char *)data,',');
	if (p == NULL)
	{
		return 0;
	}
	func[0] = myatoi((char *)data);
	data = ++p;

	p = (unsigned char *)strchr_t((const char *)data,',');
	if (p == NULL)
	{
		return 0;
	}
	func[1] = myatoi((char *)data);
	data = ++p;

	func[2] = myatoi((char *)data);

	if(func[0] == 0)
	{
		sys_SetUartFunc(0, URT_MASK_NULL);
	}
	else
	{
		func[0] -= 1;
		sys_SetUartFunc(0, (1<<func[0]));
	}
	if(func[1] == 0)
	{
		sys_SetUartFunc(1, URT_MASK_NULL);
	}
	else
	{
		func[1] -= 1;
		sys_SetUartFunc(1, (1<<func[1]));
	}
	if(func[2] == 0)
	{
		sys_SetUartFunc(2, URT_MASK_NULL);
	}
	else
	{
		func[2] -= 1;
		sys_SetUartFunc(2, (1<<func[2]));
	}

	//对应外设重新清零
	if(serialoil_enable_urtfun(ALL) == ENABLE)
	{
		serialoil_noinit_clear();
	}

	return 1; 
}

void handset_gc_mode(void)
{	
	if(gc_debug == 0)
	{
		unsigned char buf[320]={0};

		if(sDev232_2time)
		{
			sDev232_2time--;
		}
		
//"<SIM:%d;CSQ:%d;CGREG:%d;CEN:%d;GPS:%d;SAT:%d;IMEI:%s;ICID:%s;APN:%s,%s,%s;POW:%d;AD:%d-%d-%d-%d-%d;IO:%d;>\r\n",
		sprintf((char *)buf, \
			"<9,%d,%d,%d,%d,%d,%d,%s,%s,%s,%s,%s,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%02x%02x%02x%02x%02x%02x%02x%02x,>", \
			modem_GetCpinStatus(),
			modem_GetCsq(),
			((modem_GetCgreg()==1)||(modem_GetCgreg()==5))?1:0,
			modem_GetState(),
			sys_GetGpsState(),
			gps_GetGpsStarNum(),
			modem_GetImei(),
			modem_GetCcid(),
			sys_GetApn(),
			sys_GetApnUser(),
			sys_GetApnPasswd(),
			sys_state_checkAlarm(ALARMST_POWER_DOWN)?0:1,
			sDevPowerVolt[0],
			sDevPowerVolt[1],
			sDevPowerVolt[2],
			sDevPowerVolt[3],
			sDevPowerVolt[4],
			sys_state_getCar9(),
			sDev232_2time,
			xdevinfo.tempbt,
			xdevinfo.sKeyfor,
			g_flag.bit.RfidLink,
			Byte_buf[0],Byte_buf[1],Byte_buf[2],Byte_buf[3],
			Byte_buf[4],Byte_buf[5],Byte_buf[6],Byte_buf[7]
			);
		print0(buf,strlen_t((char *)buf));
//		print4("AT\r\n",4);
	}
}

