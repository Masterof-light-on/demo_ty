#include "includes.h"

g_sys_info  g_sys;
g_sn_info  g_sn;
g_runtime_info g_runtime;
g_mile_info g_mile;
g_flag_info g_flag;
TIME g_rtc;

unsigned char g_log_flag  = LOG_RUNINFO;
unsigned int g_AlarmST  = 0;
unsigned int	g_CarState = 0;
unsigned int	g_SysState = 0;
unsigned int	g_lastGpsA = 0;
unsigned char g_carSta900 = 0;
void sys_Init(void)
{
	sys_check_rst();		//检测重启类型
	
	sys_clear_runFlag();	//清运行标志

	sys_ReadUpdate_Flag();	//远程升级标志

	sys_readSys();			//读系统参数

	sys_readSN();			//读SN参数

	sys_readRuning();		//读运行参数

	sys_clear_run();		//清空全局变量

	gps_mileage_read();		//读里程值

	gps_valid_read();		//读gps有效值

	MsgGsm_Init();			//队列初始化
}

//检测mcu重启状态类型
void sys_check_rst(void)
{	
	if(rcu_flag_get(RCU_FLAG_PORRST) != RESET)
	{
		sys_noinit_clear();
		ipv_serialnum_noinit();
		sms_noinit_clear();
		sys_SetUpdateFlag(0);
	}
	else
	{
		LOG("[sys] no init\r\n");
	}

	rcu_all_reset_flag_clear();
}

//crc16
unsigned short sys_param_crc16(unsigned char d[], unsigned int len)
{
	unsigned char b = 0;
	unsigned short crc = 0xffff;
	unsigned int i, j;

	for(i=0; i<len; i++)
	{
		for(j=0; j<8; j++)
		{
			b = ((d[i]<<j)&0x80) ^ ((crc&0x8000)>>8);
			crc<<=1;
			if(b!=0)
			{
				crc^=0x1021;
			}
		}
	}

	return crc;
}

unsigned char sys_fw_check(void)
{
	if(memcmp_t(g_sys.FwVersion, MACHINE_STRING, strlen_t(MACHINE_STRING)) || (strlen_t((char *)g_sys.FwVersion) != strlen_t(MACHINE_STRING)))
	{
		memcpy_t(g_sys.FwVersion, MACHINE_STRING, strlen_t(MACHINE_STRING));

		return 0;
	}

	return 1;
}

//保存通讯参数
void sys_saveSys(void)
{
	LOG("[sys] save sys param\r\n");
	g_sys.g_chksum = sys_param_crc16((unsigned char*)&g_sys,(sizeof(g_sys)-4));

	fmc_write_data(ADDR_FMC_SECTOR_1,sizeof(g_sys),(unsigned char*)&g_sys);//内部flash保存
	flash_Write(PAGE_SYSCONFIG,sizeof(g_sys),(unsigned char*)&g_sys);//外部flash保存备份
}
//读通讯参数
void sys_readSys(void)
{
	unsigned int crc=0;

	LOG("[sys] read sys param\r\n");
	//读片内Flash
	memset_t((char *)&g_sys,0,sizeof(g_sys));
	fmc_read_data(ADDR_FMC_SECTOR_1,sizeof(g_sys),(unsigned char*)&g_sys);
		
	crc = sys_param_crc16((unsigned char*)&g_sys,(sizeof(g_sys)-4));
	if((sys_fw_check() == 0) /*|| (crc != g_sys.g_chksum)*/)
	{
		LOG("[sys] read internal sys error\r\n");
		LOG("[sys] crc = %x,%x, FW = %s->%s\r\n",crc,g_sys.g_chksum,g_sys.FwVersion,MACHINE_STRING);
		//读外部Flash
		memset_t((char *)&g_sys,0,sizeof(g_sys));
		flash_Read(PAGE_SYSCONFIG,sizeof(g_sys),(unsigned char*)&g_sys);
		
		crc = sys_param_crc16((unsigned char*)&g_sys,(sizeof(g_sys)-4));
		if((sys_fw_check() == 0) /*|| (crc != g_sys.g_chksum)*/)
		{
			LOG("[sys] read external sys error\r\n");
			LOG("[sys] crc = %x,%x, FW = %s->%s\r\n",crc,g_sys.g_chksum,g_sys.FwVersion,MACHINE_STRING);
			sys_ClearFlash(TRUE);	
		}
		else
		{
			LOG("[sys] external sys param ok\r\n");
			fmc_write_data(ADDR_FMC_SECTOR_1,sizeof(g_sys),(unsigned char*)&g_sys);
		}	
	}
}
//保存运行参数
void sys_saveRuning(void)
{
	LOG("[sys] save run param\r\n");
	g_runtime.g_chksum = sys_param_crc16((unsigned char*)&g_runtime,(sizeof(g_runtime)-4));
	flash_Write(PAGE_RUNTIME,sizeof(g_runtime),(unsigned char*)&g_runtime);//外部flash保存	
}
//读运行参数
void sys_readRuning(void)
{	
	LOG("[sys] read run param\r\n");
	memset_t((char *)&g_runtime,0,sizeof(g_runtime));
	flash_Read(PAGE_RUNTIME,sizeof(g_runtime),(unsigned char*)&g_runtime);
}
//保存SN
void sys_saveSN(void)
{
	LOG("[sys] save sn param\r\n");
	flash_Write(PAGE_SNINFO,sizeof(g_sn),(unsigned char*)&g_sn);	
}
//读SN参数
void sys_readSN(void)
{	
	LOG("[sys] read sn param\r\n");
	memset_t((char *)&g_sn,0,sizeof(g_sn));
	flash_Read(PAGE_SNINFO,sizeof(g_sn),(unsigned char*)&g_sn);
}

//清零
void sys_clear_run(void)
{
	g_AlarmST  = 0;
	g_CarState = 0;	
	
	g_SysState = 0;
	g_lastGpsA = 0;

	sys_noinit_state();
}

//清报警
void sys_clear_alarm(void)
{
	g_AlarmST = 0;
}

//凌晨0点重启
void sys_reset_day(void)
{
	TIME sysTime;
	static unsigned int runTime=0;

	//终端至少运行2小时以上
	if(runTime++ <= 2*3600)
	{
		return;
	}
	memcpy_t(&sysTime,sys_GetSysRTC(),sizeof(TIME));
	if(sysTime.hour == 0)
	{		

		sys_SoftReset();
	}	
}

void sys_reset_offline(void)
{
//	static unsigned short runTime=0;

//	if(strstr_t((char *)sys_GetCarVin(), "00000000000000000"))
//	{
//		return;
//	}
//	if((modem_GetCgreg() == 1)||(modem_GetCgreg() == 5))
//	{
//		if((g_obd.RPM != 0xFFFF)&&(g_obd.RPM > 600))
//		{
//			if(modem_GetState() != GSM_ONLINE_STATE)
//			{
//				if(++runTime > 3600)
//				{
//					LOG("[gsm] 1h reset\r\n");
//					sys_SoftReset();
//				}
//			}
//			else
//			{
//				runTime = 0;
//			}
//		}
//		else
//		{
//			runTime = 0;
//		}
//	}
//	else
//	{
//		runTime = 0;
//	}
}

//校时rtc时间
void sys_SetSysRTC(TIME *rtc)
{
	if(g_flag.bit.setCaltimeFlag == 0)
	{
		if(RTC_Set_Time(rtc) == 1)
		{
			if(sys_state_get_gpsloc())
			{
				g_flag.bit.setCaltimeFlag = 1;
			}
		}
	}	
}
//获取rtc时间
TIME *sys_GetSysRTC(void)
{
	RTC_Get_Time(&g_rtc,RTC_GetForCounter());

	return &g_rtc;
}
//获取IP
unsigned char *sys_GetIp(void)
{ 
	return g_sys.g_comm.sCenterIp;
}
//设置IP
void sys_SetIp(unsigned char* buf)
{
	if((buf==NULL) || ((strlen_t((const char *)buf)>sizeof(g_sys.g_comm.sCenterIp))))
	{
		return;
	}
	
	memset_t((char *)g_sys.g_comm.sCenterIp,0,sizeof(g_sys.g_comm.sCenterIp));
	memcpy_t(g_sys.g_comm.sCenterIp,buf,strlen_t((const char *)buf));
}
//清空IP
void sys_ClrIp(void)
{
	memset_t((char *)g_sys.g_comm.sCenterIp,0,sizeof(g_sys.g_comm.sCenterIp));
}
//获取备份IP
unsigned char *sys_GetIpBak(void)
{ 
	return g_sys.g_comm.sCenterIpBak;
}
//设置备份IP
void sys_SetIpBak(unsigned char* buf)
{
	if((buf==NULL) || ((strlen_t((const char *)buf)>sizeof(g_sys.g_comm.sCenterIpBak))))
	{
		return;
	}
	
	memset_t((char *)g_sys.g_comm.sCenterIpBak,0,sizeof(g_sys.g_comm.sCenterIpBak));
	memcpy_t(g_sys.g_comm.sCenterIpBak,buf,strlen_t((const char *)buf));
}
//清空IP
void sys_ClrIpBak(void)
{
	memset_t((char *)g_sys.g_comm.sCenterIpBak,0,sizeof(g_sys.g_comm.sCenterIpBak));
}
//获取IP端口
unsigned short sys_GetPort(void)
{
   	return g_sys.g_comm.nCenterPort;
}
//设置IP端口
void sys_SetPort(unsigned short port)
{
    g_sys.g_comm.nCenterPort = port;
}
//获取备份端口
unsigned short sys_GetPortBak(void)
{
   	return g_sys.g_comm.nCenterPortBak;
}
//设置端口
void sys_SetPortBak(unsigned short port)
{
    g_sys.g_comm.nCenterPortBak = port;
}
//获取连接方式
unsigned char sys_GetLinkType(void)
{
	return g_sys.g_comm.nNetType;
}
//设置连接方式
void sys_SetLinkType(unsigned char type)
{
    g_sys.g_comm.nNetType = type;
}
//获取备份连接方式
unsigned char sys_GetLinkTypeBak(void)
{
	return g_sys.g_comm.nNetTypeBak;
}
//设置备份连接方式
void sys_SetLinkTypeBak(unsigned char type)
{
    g_sys.g_comm.nNetTypeBak = type;
}
//获取域名
unsigned char *sys_GetUrl(void)
{
	return g_sys.g_comm.sURL;
}
//获取备份域名
unsigned char *sys_GetUrlBak(void)
{
	return g_sys.g_comm.sURLBak;
}
//设置域名
void sys_SetUrl(unsigned char *buf)
{
	if ((buf==NULL) || ((strlen_t((const char *)buf) >sizeof(g_sys.g_comm.sURL))))
	{
		return;
	}
	memset_t((char *)g_sys.g_comm.sURL,0,sizeof(g_sys.g_comm.sURL));
	memcpy_t(g_sys.g_comm.sURL,buf,strlen_t((const char *)buf));
}
//设置备份域名
void sys_SetUrlBak(unsigned char *buf)
{
	if ((buf==NULL) || ((strlen_t((const char *)buf) >sizeof(g_sys.g_comm.sURLBak))))
	{
		return;
	}
	memset_t((char *)g_sys.g_comm.sURLBak,0,sizeof(g_sys.g_comm.sURLBak));
	memcpy_t(g_sys.g_comm.sURLBak,buf,strlen_t((const char *)buf));
}
//清除域名
void sys_ClearUrl(void)
{
	memset_t((char *)g_sys.g_comm.sURL,0,sizeof(g_sys.g_comm.sURL));
}
//清除备份域名
void sys_ClearUrlBak(void)
{
	memset_t((char *)g_sys.g_comm.sURLBak,0,sizeof(g_sys.g_comm.sURLBak));
}

//获取APN
unsigned char *sys_GetApn(void)
{
   	return  g_sys.g_comm.sApn;
}
//设置APN
void sys_SetApn(unsigned char *buf)
{
    if ((buf==NULL) || ((strlen_t((const char *)buf)>sizeof(g_sys.g_comm.sApn))))
    {		
         return ;
    }
    
    memset_t((char *)g_sys.g_comm.sApn,0,sizeof(g_sys.g_comm.sApn));
    memcpy_t(g_sys.g_comm.sApn,buf,strlen_t((const char *)buf));
}
//获取APN用户名
unsigned char *sys_GetApnUser(void)
{
   	return  g_sys.g_comm.sUser;
}
//设置APN用户名
void sys_SetApnUser(unsigned char *buf)
{
    if ((buf==NULL) || ((strlen_t((const char *)buf)>sizeof(g_sys.g_comm.sUser))))
    {		
		return ;
    }
    
    memset_t((char *)g_sys.g_comm.sUser,0,sizeof(g_sys.g_comm.sUser));
    memcpy_t(g_sys.g_comm.sUser,buf,strlen_t((const char *)buf));
}
//获取APN密码
unsigned char *sys_GetApnPasswd(void)
{
   	return  g_sys.g_comm.sPasswd;
}
//设置APN密码
void sys_SetApnPasswd(unsigned char *buf)
{
    if ((buf==NULL) || ((strlen_t((const char *)buf)>sizeof(g_sys.g_comm.sPasswd))))
    {		
         return ;
    }
    
    memset_t((char *)g_sys.g_comm.sPasswd,0,sizeof(g_sys.g_comm.sPasswd));
    memcpy_t(g_sys.g_comm.sPasswd,buf,strlen_t((const char *)buf));
}


/****************************808*********************************************************/
//设置终端ID
void sys_SetId(unsigned char* buf)
{
	if (buf==NULL)
	{
		return;
	}
	
	g_sys.g_login.sFakeIp[0] = 0x0F & (buf[0]-'0');
	g_sys.g_login.sFakeIp[1] = (buf[1]-'0')<<4 | (buf[2]-'0');
	g_sys.g_login.sFakeIp[2] = (buf[3]-'0')<<4 | (buf[4]-'0');
	g_sys.g_login.sFakeIp[3] = (buf[5]-'0')<<4 | (buf[6]-'0');
	g_sys.g_login.sFakeIp[4] = (buf[7]-'0')<<4 | (buf[8]-'0');
	g_sys.g_login.sFakeIp[5] = (buf[9]-'0')<<4 | (buf[10]-'0');
}
//获取终端ID 
unsigned char *sys_GetId1(void)
{
    return g_sys.g_login.sFakeIp;
}
void sys_GetId2(unsigned char *buf)
{
	sprintf((char *)buf,"%02x%02x%02x%02x%02x%02x",\
			(int)g_sys.g_login.sFakeIp[0],(int)g_sys.g_login.sFakeIp[1],(int)g_sys.g_login.sFakeIp[2],(int)g_sys.g_login.sFakeIp[3],(int)g_sys.g_login.sFakeIp[4],(int)g_sys.g_login.sFakeIp[5]);	 
}

//设置车辆VIN
void sys_SetCarVin(unsigned char* buf)
{
	if ((buf==NULL) || ((strlen_t((const char *)buf)>sizeof(g_sys.g_login.sCarVin))))
    {		
         return ;
    }
	
	memset_t((char *)g_sys.g_login.sCarVin,0,sizeof(g_sys.g_login.sCarVin));
	memcpy_t(g_sys.g_login.sCarVin,buf,strlen_t((const char *)buf));
}
//获取车辆VIN
unsigned char* sys_GetCarVin(void)
{
	return g_sys.g_login.sCarVin;
}
//设置终端ID
void sys_SetTermId(unsigned char* buf)
{
	if ((buf==NULL) || ((strlen_t((const char *)buf)>sizeof(g_sys.g_login.sTerID))))
    {		
         return ;
    }
	memset_t((char *)g_sys.g_login.sTerID,0,sizeof(g_sys.g_login.sTerID));
	memcpy_t(g_sys.g_login.sTerID,buf,strlen_t((const char *)buf));
}
//获取终端ID
unsigned char *sys_GetTermId(void)
{
	if(strlen_t((char *)g_sys.g_login.sTerID) == 0)
	{
		strcpy_t((char *)g_sys.g_login.sTerID, "0000000");
	}
	
	return g_sys.g_login.sTerID;
}
//设置省域ID
void sys_SetProvinceId(unsigned short data)
{
	g_sys.g_login.nProvince_ID = data;
}
//获取省域ID
unsigned short sys_GetProvinceId(void)
{
	return g_sys.g_login.nProvince_ID;
}
//设置市域ID
void sys_SetCityId(unsigned short data)
{
	g_sys.g_login.nCity_ID = data;
}
//获取市域ID
unsigned short sys_GetCityId(void)
{
	return g_sys.g_login.nCity_ID;
}
//设置制造商ID
void sys_SetManuId(unsigned char* buf)
{
	if ((buf==NULL) || ((strlen_t((const char *)buf)>sizeof(g_sys.g_login.sManuCode))))
    {		
         return ;
    }
	memset_t((char *)g_sys.g_login.sManuCode,0,sizeof(g_sys.g_login.sManuCode));
	memcpy_t(g_sys.g_login.sManuCode,buf,strlen_t((const char *)buf));
}
//获取制造商ID
unsigned char* sys_GetManuId(void)
{
	if(strlen_t((char *)g_sys.g_login.sManuCode) == 0)
	{
		strcpy_t((char *)g_sys.g_login.sManuCode, "88888");
	}
	
	return g_sys.g_login.sManuCode;
}
//设置制造商ID
void sys_SetTermType(unsigned char* buf)
{
	if ((buf==NULL) || ((strlen_t((const char *)buf)>sizeof(g_sys.g_login.sTerType))))
    {		
         return ;
    }
	memset_t((char *)g_sys.g_login.sTerType,0,sizeof(g_sys.g_login.sTerType));
	memcpy_t(g_sys.g_login.sTerType,buf,strlen_t((const char *)buf));
}
//获取制造商ID
unsigned char* sys_GetTermType(void)
{
	if(strlen_t((char *)g_sys.g_login.sTerType) == 0)
	{
		memcpy_t(g_sys.g_login.sTerType,"F300A-JT808",strlen_t("F300A-JT808"));
	}
	return g_sys.g_login.sTerType;
}
//设置车牌颜色
void sys_SetCarColor(unsigned char data)
{
	g_sys.g_login.nCarColor= data;
}
//获取车牌颜色
unsigned char sys_GetCarColor(void)
{
	return g_sys.g_login.nCarColor;
}
//设置车牌号
void sys_SetCarLicences(unsigned char* buf)
{
	if ((buf==NULL) || ((strlen_t((const char *)buf)>sizeof(g_sys.g_login.sCarLicences))))
    {		
         return ;  
    }
	memset_t((char *)g_sys.g_login.sCarLicences,0,sizeof(g_sys.g_login.sCarLicences));
	memcpy_t(g_sys.g_login.sCarLicences,buf,strlen_t((const char *)buf));
}
//获取车牌号
unsigned char* sys_GetCarLicences(void)
{
	return g_sys.g_login.sCarLicences;
}

//设置设备ID 
void sys_SetImeiDevID(unsigned char* buf)
{
	g_devid_info xDevID;
	xDevID.value = 0;
	sscanf((char*)buf,"%lld",&xDevID.value);	
	g_sys.g_login.sImeiDevID[0] = xDevID.buf[7];
	g_sys.g_login.sImeiDevID[1] = xDevID.buf[6];
	g_sys.g_login.sImeiDevID[2] = xDevID.buf[5];
	g_sys.g_login.sImeiDevID[3] = xDevID.buf[4];
	g_sys.g_login.sImeiDevID[4] = xDevID.buf[3];
	g_sys.g_login.sImeiDevID[5] = xDevID.buf[2];
	g_sys.g_login.sImeiDevID[6] = xDevID.buf[1];
	g_sys.g_login.sImeiDevID[7] = xDevID.buf[0];
}

/****************************808*********************************************************/
//获取SN
unsigned char *sys_GetTermSN(void)
{
	if(g_sn.nTermSN[0] == 0xFF)
	{
		memset_t((char *)g_sn.nTermSN,0,sizeof(g_sn.nTermSN));
		memcpy_t(g_sn.nTermSN,"00000000000000000",17);
	}
	return g_sn.nTermSN;
}
//设置SN
void sys_SetTermSN(unsigned char* buf)
{
	memset_t((char *)g_sn.nTermSN,0,sizeof(g_sn.nTermSN));
	memcpy_t(g_sn.nTermSN,buf,strlen_t((const char *)buf));
}

//设置终端密码
void sys_SetTerminalPwd(unsigned char *buf)
{
    if ((buf==NULL) || ((strlen_t((const char *)buf))>sizeof(g_runtime.g_phone.sTerminalPwd)))
    {
       	return;
    }
	
    memset_t((char *)g_runtime.g_phone.sTerminalPwd,0,sizeof(g_runtime.g_phone.sTerminalPwd));
    memcpy_t(g_runtime.g_phone.sTerminalPwd,buf,strlen_t((const char *)buf));	
}
//获取终端密码
unsigned char *sys_GetTerminalPwd(void)
{	
	if(strlen_t((char*)g_runtime.g_phone.sTerminalPwd) == 0)
	{
		memcpy_t((char*)g_runtime.g_phone.sTerminalPwd, SMS_INITIAL_PASSWORD, strlen_t(SMS_INITIAL_PASSWORD));
	}
	
	return g_runtime.g_phone.sTerminalPwd;
}
//密码匹配
unsigned char sys_CheckTerminalPwd(unsigned char *buf)
{
	if(memcmp_t(buf, sys_GetTerminalPwd(), strlen_t((char*)sys_GetTerminalPwd())) == 0)
	{
		return 1;
	}
	//超级密码
	else if(memcmp_t(buf, SMS_SUPER_PASSWORD, strlen_t(SMS_SUPER_PASSWORD)) == 0)
	{
		return 1;
	}
	else if(memcmp_t(buf, "1234", strlen_t("1234")) == 0)
	{
		return 1;
	}

	return 0;
}
//设置授权号码
void sys_SetAuthorPhone(unsigned char cn, unsigned char *buf)
{
	if ((cn>=PHONE_NUM)  || (buf==NULL) || ((strlen_t((const char *)buf) >=PHONE_LEN)))
	{
		return;
	} 
	
	memset_t((char *)g_runtime.g_phone.authorPhone[cn],0,PHONE_LEN);	 
	memcpy_t(g_runtime.g_phone.authorPhone[cn],buf,strlen_t((const char *)buf));
}
//获取授权号码
unsigned char *sys_GetAuthorPhone(unsigned char cn)
{
	if(g_runtime.g_phone.authorPhone[cn][0] == 0xFF)
	{
		memset_t((char *)g_runtime.g_phone.authorPhone[cn],0,PHONE_LEN);	
	}
	return g_runtime.g_phone.authorPhone[cn];
}
//清空授权号码
void sys_ClrAuthorPhone(void)
{
	unsigned char i;

	for(i=0; i<PHONE_NUM; i++)
	{
		memset_t((char *)g_runtime.g_phone.authorPhone[i],0,PHONE_LEN);
	}
}
//检查是否与授权号码匹配
unsigned char sys_CheckAuthorPhone(unsigned char *buf)
{
	unsigned char i=0;

	//授权号码为空
	if(sms_CheckHaveAutor() == 0)
	{
		return AUTHORPHONE_NULL;
	}
	
	for(i=0;i<PHONE_NUM;i++)
	{
		if(memcmp_inverted((char*)sys_GetAuthorPhone(i),(char*)buf)==0)
		{
			return AUTHORPHONE_CMP_SUCCESS;	
		}
	}
	return AUTHORPHONE_CMP_FAIL;
}
//设置是否使能请求CAN数据
void sys_SetCanRequestAble(unsigned char ctl)
{
	g_sys.g_param.canRequest = ctl;
}
//获取是否使能请求CAN数据
unsigned char sys_GetCanRequestAble(void)
{
	return g_sys.g_param.canRequest;
}
//设置时区
void sys_SetTimeZone(unsigned char zone)
{
    g_sys.g_param.timezone = zone;
}
//获取时区
unsigned char sys_GetTimeZone(void)
{
	if(g_sys.g_param.timezone == 0xFF)
	{
		g_sys.g_param.timezone = 8;
	}
	return g_sys.g_param.timezone;
}
//设置时区类型
void sys_SetTimeType(unsigned char type)
{
    g_sys.g_param.timetype = type;
}
//获取时区类型
unsigned char sys_GetTimeType(void)
{
    if(!((g_sys.g_param.timetype == '+')||(g_sys.g_param.timetype == '-')))
	{
		g_sys.g_param.timetype = '+';
	}
	
    return g_sys.g_param.timetype;
}
//设置应答
void sys_SetAckSwitch(unsigned char ack)
{
    g_sys.g_param.ackSwitch = ack;
}
//获取应答
unsigned char sys_GetAckSwitch(void)
{
	return g_sys.g_param.ackSwitch;
}
//设置里程系数 
void sys_SetageNum(unsigned char data)
{
   if ((data>200) || (data == 0))
   {
      	g_runtime.g_gpsrun.ageTypeNum = 100;
   }
   else
   {
       	g_runtime.g_gpsrun.ageTypeNum = data;
   }
}
//获取里程系数
unsigned char sys_GetageNum(void)
{
    if ((g_runtime.g_gpsrun.ageTypeNum > 200) || (g_runtime.g_gpsrun.ageTypeNum == 0))
    {
        g_runtime.g_gpsrun.ageTypeNum = 100;
    }

    return g_runtime.g_gpsrun.ageTypeNum;
}
//设置里程  (单位是米)
void sys_SetMileAge(unsigned int mileage)
{
	g_mile.nMileAge = mileage;
}
//里程的叠加
void sys_AddMileAge(unsigned int mileage)
{
	g_mile.nMileAge += mileage;
}
//获取里程
unsigned int sys_GetMileAge(void)
{
   return g_mile.nMileAge;
}
//设置LOG
void sys_set_debugSwitch(char on)
{
	g_log_flag = on;	
}
unsigned char sys_get_debugSwitch(void)
{
	return g_log_flag;
}

//GPS状态
//0未定位 1定位 2天线短路 3天线开路 4模块故障
GPSANT_STATUS sys_GetGpsState(void)
{
	GPSANT_STATUS ret;

	if(sys_state_checkAlarm(ALARMST_GPSMODEM))
	{
		ret = GPS_ANTFAULT;
	}
	else if(sys_state_checkAlarm(ALARMST_GPSANTS))
	{
		ret = GPS_ANTSHORT;	
	}
	else if(sys_state_checkAlarm(ALARMST_GPSANTO))
	{
		ret = GPS_ANTOPEN;
	}
	else if(sys_state_checkCar(ON_STS_GPSLOC))
	{
		ret = GPS_ANTA;
	}
	else
	{
		ret = GPS_ANTV;
	}

	return ret;
}
//设置自动接听的时间
void sys_SetAutoAnswerTime(unsigned char data)
{
    g_runtime.g_phone.nAutoAnswer = data;
}
//获取自动接听的时间
unsigned char sys_GetAutoAnswerTime(void)
{
   	return  g_runtime.g_phone.nAutoAnswer;
}
//设置通话权限
void sys_SetCallCtrl(unsigned char data)
{
	if((data  == CALLPHONE_FORBID_DIAL)||\
		(data == CALLPHONE_FORBID_ATA)||\
		(data == CALLPHONE_FORBID_DIAL_ATA)||\
		(data == CALLPHONE_ALLOW_DIAL_ATA))
	{
		g_runtime.g_phone.nCallCtrl = data;
	}
}
//获取通话权限
unsigned char sys_GetCallCtrl(void)
{
    if(!((g_runtime.g_phone.nCallCtrl==CALLPHONE_FORBID_DIAL)||\
		  (g_runtime.g_phone.nCallCtrl==CALLPHONE_FORBID_ATA)||\
		  (g_runtime.g_phone.nCallCtrl==CALLPHONE_FORBID_DIAL_ATA)||\
		  (g_runtime.g_phone.nCallCtrl==CALLPHONE_ALLOW_DIAL_ATA)))
    {
		g_runtime.g_phone.nCallCtrl = CALLPHONE_NULL;
    }
    
    return g_runtime.g_phone.nCallCtrl;
}

//设置ACC关定时汇报间隔
void sys_SetAccStateOffInterval(unsigned short data)
{
	if (data == 0)
	{
		g_runtime.g_watch.nWatchIntervaloff = 0;
		return;
	}
	if ((data < 3) || (data >= 65535))
	{
		g_runtime.g_watch.nWatchIntervaloff = 30;
	}
	else
	{
		g_runtime.g_watch.nWatchIntervaloff = data;
	} 
}
//获取ACC关定时汇报间隔
unsigned short sys_GetAccStateOffInterval(void)
{
    if (g_runtime.g_watch.nWatchIntervaloff >= 65535)
    {
		g_runtime.g_watch.nWatchIntervaloff = 30;
    }

    return g_runtime.g_watch.nWatchIntervaloff;
}

//设置ACC开时的汇报间隔
void sys_SetAccStateOnInterval(unsigned short data)
{
	if (data == 0)
	{
		g_runtime.g_watch.nWatchInterval = 0;
		return;	
	}
	if ((data < 3) || (data >= 65535))
	{
		g_runtime.g_watch.nWatchInterval = 30;
	}
	else
	{
		g_runtime.g_watch.nWatchInterval = data;
	}
   
}
//获取ACC开定时汇报间隔
unsigned short sys_GetAccStateOnInterval(void)
{
    if (g_runtime.g_watch.nWatchInterval >= 65535)
    {
        g_runtime.g_watch.nWatchInterval = 30;
    }

    return g_runtime.g_watch.nWatchInterval;
}
//设置定距汇报距离
void sys_SetDistanceInterval(unsigned short data)
{
	if (data == 0)
	{
		g_runtime.g_watch.nWatchDistance = 0;
		return;	
	}
	if ((data < 10) || (data >= 65535))
	{
		g_runtime.g_watch.nWatchDistance = 100;
	}
	else
	{
		g_runtime.g_watch.nWatchDistance = data;
	}
}
//获取定距汇报距离
unsigned short sys_GetDistanceInterval(void)
{
	if (g_runtime.g_watch.nWatchDistance >= 65535)
	{
		g_runtime.g_watch.nWatchDistance = 100;
	}
	return g_runtime.g_watch.nWatchDistance;
}
//设置拐点
void sys_SetRedeem(unsigned char data)
{
	if (data >= 180)
	{
		g_runtime.g_watch.nRedeem = 30;
	}
	else
	{
		g_runtime.g_watch.nRedeem = data;
	}
}
//获取拐点
unsigned char sys_GetRedeem(void)
{
    if (g_runtime.g_watch.nRedeem >= 180)
    {
        g_runtime.g_watch.nRedeem = 30;
    }
	
    return g_runtime.g_watch.nRedeem;
}
//设置监控类型 1 定时  2 定距
void sys_SetWatchType(unsigned char type)
{
	g_runtime.g_watch.nWatchType = type;
	
	if ((type!=1) && (type !=2))
	{
		g_runtime.g_watch.nWatchType = 1;
	}          
}
//获取监控类型
unsigned char sys_GetWatchType(void)
{
	if ((g_runtime.g_watch.nWatchType!=1) && (g_runtime.g_watch.nWatchType !=2))
	{
		return 1;
	}
	
	return g_runtime.g_watch.nWatchType;
}

//设置超速的阀值
void sys_SetOverSpeed(unsigned char speed)
{
	if (speed >= 255)
	{
		g_runtime.g_gpsrun.nOverSpeed = 0;
	}
	else
	{
		g_runtime.g_gpsrun.nOverSpeed = speed;
	}
}

//获取超速的阀值
unsigned char sys_GetOverSpeed(void)
{
	if(g_runtime.g_gpsrun.nOverSpeed >= 255)
	{
		g_runtime.g_gpsrun.nOverSpeed = 0;	
	}
    return g_runtime.g_gpsrun.nOverSpeed;
}


//设置超速的持续时间
void sys_SetOverSpeedTime(unsigned short time)
{
	if (time >= 65535)
	{
		g_runtime.g_gpsrun.nOverSpeedTime = 0;
	}
	else
	{
		g_runtime.g_gpsrun.nOverSpeedTime = time;
	}
}

//获取超速的持续时间
unsigned short sys_GetOverSpeedTime(void)
{
	if(g_runtime.g_gpsrun.nOverSpeedTime >= 65535)
	{
		g_runtime.g_gpsrun.nOverSpeedTime = 0;
	}
	
	
	return g_runtime.g_gpsrun.nOverSpeedTime;
}
//设置怠速的持续时间
void sys_SettLowSpeedTime(unsigned short time)
{
	if(time >= 65535)
	{
		g_runtime.g_gpsrun.nLowSpeedTime = 0;
	}
	else
	{
		g_runtime.g_gpsrun.nLowSpeedTime = time;  
	}    
}

//获取怠速的持续时间
unsigned short sys_GetLowSpeedTime(void)
{
	if(g_runtime.g_gpsrun.nLowSpeedTime >= 65535)
	{
		g_runtime.g_gpsrun.nLowSpeedTime = 0;
	}
	
	return g_runtime.g_gpsrun.nLowSpeedTime;
}

//设置疲劳驾驶时间
void sys_SetDriveTimeOut(unsigned short time)
{
	if(time >= 65535)
	{
		g_runtime.g_gpsrun.nDriveTimeOut = 0;
	}
	else
	{
		g_runtime.g_gpsrun.nDriveTimeOut = time;
	}  
}

//获取疲劳驾驶的时间
unsigned short sys_GetDriveTimeOut(void)
{
	if(g_runtime.g_gpsrun.nDriveTimeOut >= 65535)
	{
		g_runtime.g_gpsrun.nDriveTimeOut = 0;
	}
	
	return  g_runtime.g_gpsrun.nDriveTimeOut;
}

//设置休息时间
void sys_SetRestTimeOut(unsigned short time)
{
	if(time >= 65535) 
	{
		g_runtime.g_gpsrun.nRestTimeOut = 0;
	}
	else
	{
		g_runtime.g_gpsrun.nRestTimeOut = time;
	}  
}

//获取休息时间
unsigned short sys_GetRestTimeOut(void)
{
	if(g_runtime.g_gpsrun.nRestTimeOut >= 65535)
	{
		g_runtime.g_gpsrun.nRestTimeOut = 0;
	}
	
	return  g_runtime.g_gpsrun.nRestTimeOut;
}
//设置基准里程值
void sys_SetBaseMileage(unsigned int mile)
{
	g_runtime.g_gpsrun.nBaseMileage = mile;
}
//获取基准里程值
unsigned int sys_GetBaseMileage(void)
{
	return  g_runtime.g_gpsrun.nBaseMileage;
}
//设置位移报警的距离
void sys_SetMoveDistance(unsigned short distance)
{
	if(distance >= 65535)
	{
		g_runtime.g_gpsrun.nMoveDistance = 0;
	}
	else
	{
		g_runtime.g_gpsrun.nMoveDistance = distance;
	}
}

//获取位移报警的距离
unsigned short sys_GetMoveDistance(void)
{
	if(g_runtime.g_gpsrun.nMoveDistance >= 65535)
	{
		g_runtime.g_gpsrun.nMoveDistance = 0;
	}
	
	return g_runtime.g_gpsrun.nMoveDistance;
}

//设置IP2在线时间
void sys_SetIP2KeepAlive(unsigned int keepalive)
{
	g_runtime.g_gpsrun.nIP2KeepAlive = keepalive;
}

//获取IP2在线时间
unsigned int sys_GetIP2KeepAlive(void)
{
	return g_runtime.g_gpsrun.nIP2KeepAlive;
}

//设置CAN波特率
void sys_SetCan1Baud(unsigned short baud)
{
	g_sys.g_param.can1Baud = baud;
}
//获取CAN波特率
unsigned short sys_GetCan1Baud(void)
{
	return g_sys.g_param.can1Baud;
}
//设置CAN波特率
void sys_SetCan2Baud(unsigned short baud)
{
	g_sys.g_param.can2Baud = baud;
}
//获取CAN波特率
unsigned short sys_GetCan2Baud(void)
{
	return g_sys.g_param.can2Baud;
}

/**********************************************sleep***********************************************************************/
//设置休眠使能
void sys_SetSleepAble(unsigned char type)
{
	if ((type != 0) && (type != 1) && (type != 2))
	{
		type=0;
	}
	
	g_runtime.g_watch.nSleepAble = type;
}
//获取监控类型
unsigned char sys_GetSleepAble(void)
{
	if ((g_runtime.g_watch.nSleepAble !=0) && (g_runtime.g_watch.nSleepAble !=1) && (g_runtime.g_watch.nSleepAble !=2))
	{
		return 0;
	}
	
	return g_runtime.g_watch.nSleepAble;
}

//设置休眠时的唤醒间隔
void sys_SetSleepWakeUpInterval(unsigned short data)
{
	if (data >= 65535)
	{
		g_runtime.g_watch.nsleepInterval = 0;
	}
	else
	{
		g_runtime.g_watch.nsleepInterval = data;
	}
}

//获取休眠时唤醒间隔
unsigned short sys_GetSleepWakeUpInterval(void)
{
    if (g_runtime.g_watch.nsleepInterval >= 65535)
    {
        g_runtime.g_watch.nsleepInterval = 0;
    }

    return g_runtime.g_watch.nsleepInterval;
}

void sys_SetSleepTimeWakeUp(unsigned int data)
{
	if ((data < 300) || (data > 86400))
	{
		g_runtime.g_watch.sleepTime = 300;
	}
	else
	{
		g_runtime.g_watch.sleepTime = data;
	}
}


/**********************************************sleep***********************************************************************/
/***********************************************************************************/
//设置油耗的总容量
void sys_SetOilVolume(unsigned char id, unsigned short data)
{
	if((id == 0) || (id > MAX_OIL_CH))
	{
		return;
	}
	g_runtime.g_dev.oilinfo.nOilVolume[id-1] = data;
}

//获取油耗的总容量
unsigned short sys_GetOilVolume(unsigned char id)
{
	return g_runtime.g_dev.oilinfo.nOilVolume[id];
}
//设置偷油量百分比
void sys_SetOilToSteal(unsigned char data)
{
	g_runtime.g_dev.oilinfo.nStealOil = data;
}
//获取偷油量百分比
unsigned char sys_GetOilToSteal(void)
{
	return g_runtime.g_dev.oilinfo.nStealOil;
}
//设置低油量百分比
void sys_SetOilToLow(unsigned char data)
{
	g_runtime.g_dev.oilinfo.nLowOil = data;
}
//获取低油量百分比
unsigned char sys_GetOilToLow(void)
{
	return g_runtime.g_dev.oilinfo.nLowOil;
}

/***********************************************************************************/
//串口功能设置
void sys_SetUartFunc(unsigned char no,unsigned int data)
{
	g_sys.g_param.uartMask[no] = data;
}
//获取串口功能设置
unsigned int sys_GetUartFunc(unsigned char no)
{
	if((g_sys.g_param.uartMask[no] == 0)||(g_sys.g_param.uartMask[no] == 0xFFFF))
	{
		if(no == 0)
		{
			g_sys.g_param.uartMask[no] = URT_MASK_NULL;
		}
		else
		{
			g_sys.g_param.uartMask[no] = URT_MASK_NULL;
		}
	}
	
	return g_sys.g_param.uartMask[no];	
}
/***********************************************************************************/
void sys_state_newNullAlarm(unsigned int x)
{
	if(0 == (g_AlarmST&(x)))
	{
		g_AlarmST |= (x);
	}
}

void sys_state_offNullAlarm(unsigned int x)
{
	if(g_AlarmST&(x))
	{
		g_AlarmST &= (~(x));
	}
}
/***********************************************************************************/
//获取上一条的GPS数据
gpsinfo* sys_GetLastGpsData(void)
{
	return &g_gpsLastinfo;
}

//设置上一条的GPS数据
void sys_SetLastGpsData(gpsinfo *data)
{
	memset_t((char*)&g_gpsLastinfo, 0, sizeof(gpsinfo));
	memcpy_t(&g_gpsLastinfo,data,sizeof(gpsinfo));
}

//清除运行参数
void sys_ClearRuning(void)
{
   	memset_t((char *)&g_runtime, 0, sizeof(g_runtime));
}
//清空盲区游标
void sys_ClearBlindIndex(void)
{
    MsgGsm_BlindPost_clear();
    MsgGsm_BlindIndex_clear();
    MsgGsm_BlindOutdex_clear();
}
//恢复默认参数
void sys_SetDefaultSysParam(bool bfactory)
{
	if(bfactory == TRUE)
	{
		memset_t((char *)&g_sys,0,sizeof(g_sys));

		sys_SetUrl((unsigned char*)"neidnl.gps218.com");
		sys_SetPort(15553);
		sys_SetLinkType(1);
		sys_SetIpBak((unsigned char*)"58.218.151.98");
		sys_SetPortBak(8112);
		sys_SetLinkTypeBak(1);
		sys_SetId((unsigned char*)"13620222832"/*"00000000000"*/);//13620222832
		sys_SetApn((unsigned char*)"CMIOT");
		sys_SetApnUser((unsigned char*)"CMIOT");
		sys_SetApnPasswd((unsigned char*)"CMIOT");
		memcpy_t(g_sys.FwVersion, MACHINE_STRING, strlen_t(MACHINE_STRING));

		sys_SetTimeType('+'); //'+'
		sys_SetTimeZone(0);   //8时区
		sys_SetUartFunc(0,URT_MASK_XD_OIL);
		sys_SetUartFunc(1,URT_MASK_NULL);
		sys_SetUartFunc(2,URT_MASK_NULL);
		sys_SetCan1Baud(250);
		sys_SetCan2Baud(250);
		
		sys_saveSys();
	}
}
//设置默认参数
void sys_SetDefaultParam(void)
{
	sys_SetAccStateOnInterval(30);
	sys_SetAccStateOffInterval(3600);
	sys_SetDistanceInterval(0);
	sys_SetRedeem(0);
	sys_SetWatchType(1);
	sys_SetCallCtrl(CALLPHONE_ALLOW_DIAL_ATA);

	sys_SetTerminalPwd((unsigned char*)SMS_INITIAL_PASSWORD);
	sys_SetSleepAble(1);
	
	g_runtime.g_watch.sleepTime = 86400;//单位秒
	
	g_runtime.g_watch.nsleepInterval = 600;//单位秒

	sys_saveRuning();
}
//软复位
void sys_SoftReset(void)
{
#if 1
	__set_FAULTMASK(1);//关闭所有中断
	NVIC_SystemReset();//复位
#else
	gc_watchdog = 1;
#endif
}

//重启
void sys_Restart(void)
{
	sys_saveRuning();
	LOG("[sys] restart...\r\n");

	sys_SoftReset();
}
//清空flash
void sys_ClearFlash(bool bfactory)
{
	sys_ClearRuning();

	flash_gpsvalid_erase();

	flash_mileage_erase();

    sys_ClearBlindIndex();

	sys_SetDefaultParam();

	sys_SetDefaultSysParam(bfactory);
}
//恢复出厂
void sys_ResetFacitory(void)
{
	//挂起所有的任务
	vTaskSuspendAll();
	
	sys_ClearFlash(FALSE);
}
//清运行标志
void sys_clear_runFlag(void)
{
	g_flag.value = 0;
}

//升级标志位
// 1:成功  2:失败
static unsigned char g_updateFlag  __noinit__;
void sys_SetUpdateFlag(unsigned char flag)
{
	g_updateFlag = flag;	
}

unsigned char sys_GetUpdateFlag(void)
{
	return g_updateFlag;
}

void sys_ReadUpdate_Flag(void)
{
	unsigned char buf[5]={0};

	flash_Read(PAGE_UPDATE_END,5,buf);

	if((buf[0]==0)&&(buf[1]==1))//升级成功
	{
		/**************************/
		sys_noinit_clear();
		ipv_serialnum_noinit();
		/**************************/
		
		sys_SetUpdateFlag(1);
		memset_t((char *)buf,0,sizeof(buf));

		flash_Write(PAGE_UPDATE_END,5,buf);
	}
	else if((buf[0]==1)&&(buf[1]==0))//升级失败
	{
		sys_SetUpdateFlag(2);
		memset_t((char *)buf,0,sizeof(buf));

		flash_Write(PAGE_UPDATE_END,5,buf);
	}
	else
	{
		;
	}
}

//备份程序到外部flash
void sys_backupApp_to_ExtFlash(void)
{
	unsigned short i=0,j=0;
	unsigned char *buf=NULL;
	unsigned char *p=NULL; 

	buf = (unsigned char *)pvPortMalloc(512);
	if(buf == NULL)
	{
		return;
	}
	
	flash_fota_backup_erase();
	
	p = (unsigned char*)ADDR_FMC_SECTOR_2;//起始地址 
	for(i=0;i<400;i++)
	{
		memset_t(buf, 0, 512);
		
		for(j=0;j<512;j++)
		{
			buf[j] = *p;
			p++;
		}

		flash_fota_write(PAGE_APPBACKUP_START+(i*512),512,buf);
	}

	vPortFree(buf);
}
//检验外部备份程序
unsigned char sys_CheckbackupApp(void)
{
	unsigned short i=0,j=0;
	unsigned char *buf=NULL;
	unsigned char *p=NULL; //起始地址

	buf = (unsigned char *)pvPortMalloc(512);
	if(buf == NULL)
	{
		return 0;
	}
	
	for(i=0;i<400;i++)
	{		
		p = (unsigned char*)(ADDR_FMC_SECTOR_2+(i*512));

		memset_t(buf, 0, 512);
		flash_Read(PAGE_APPBACKUP_START+(i*512),512,buf);

		for(j=0;j<512;j++)
		{
			if(buf[j] != p[j])
			{
				vPortFree(buf);
				
				return 0;
			}
		}
	}

	vPortFree(buf);
	
	return 1;
}
//自动备份
void sys_auto_backupApp(void)
{
#define BKP_APP_KEY1	0x5F
#define BKP_APP_KEY2	0xF5

	unsigned char buf[2];
	static unsigned char success_flag = 0;

	if(success_flag == 1)
	{
		return;
	}
	
	flash_Read(PAGE_APPBACKUP_END, 2, buf);
	if((buf[0] != BKP_APP_KEY1)&&(buf[1] != BKP_APP_KEY2))
	{
		sys_backupApp_to_ExtFlash();
		if(sys_CheckbackupApp() == 1)
		{
			buf[0] = BKP_APP_KEY1;
			buf[1] = BKP_APP_KEY2;
			flash_Write(PAGE_APPBACKUP_END, 2, buf);
			success_flag = 1;
			LOG("[backup] success\r\n");
		}
		else
		{
			LOG("[backup] failed\r\n");
		}
	}
	else
	{
		success_flag = 1;
		LOG("[backup] ok\r\n");
	}
}

//系统状态
void sys_state_gpsloc(unsigned int on)
{
	static unsigned char loc_num = 0;
	
	if(on)
	{
		sys_state_newCar(ON_STS_GPSLOC);
		sys_state_newCar(ON_STS_GPS_FIX);
		if(loc_num == 0)
		{
			loc_num = 1;
			LOG("[gps] A\r\n");
		}
	}
	else
	{
		sys_state_offCar(ON_STS_GPSLOC);
		sys_state_offCar(ON_STS_GPS_FIX);
		if(loc_num == 1)
		{
			loc_num = 0;
			LOG("[gps] V\r\n");
		}
	}
}

static unsigned char obd_num __noinit__;
static unsigned char sos_num __noinit__;
static unsigned char acc_num __noinit__;
static unsigned short g_OTANo __noinit__;

//清零
void sys_noinit_clear(void)
{
	obd_num = 0;
	sos_num = 0;
	acc_num = 0;
	g_OTANo = 0;
}

void sys_SetOTASerialNo(unsigned short no)
{
	g_OTANo = no;
}

unsigned short sys_GetOTASerialNo(void)
{
	return g_OTANo;
}

void sys_noinit_state(void)
{
	if(acc_num)
	{
		sys_state_newCar9(GPIO_ACC_ST);
		sys_state_newCar(ON_STS_ACC);
	}
}

void sys_state_obd(unsigned int on)
{
	if(on)
	{
		if(obd_num == 0)
		{
			obd_num = 1;
			sys_state_newCar(ON_STS_ACC);
			LOG("[gpio] obd on\r\n");
		}
	}
	else
	{
		if(obd_num == 1)
		{
			obd_num = 0;
			sys_state_offCar(ON_STS_ACC);
			LOG("[gpio] obd off\r\n");
		}
	}
}

void sys_state_newCar(unsigned int x)
{
	if(0 ==(g_CarState&(x)))
	{
		g_CarState |= (x);
	}
}
void sys_state_newAlarm(unsigned int x)
{
	if(0 == (g_AlarmST&(x)))
	{
		g_AlarmST |= (x);
	}
}
void sys_state_newSysState(unsigned int x)
{
	g_SysState |= (x);
}
void sys_state_offCar(unsigned int x)
{
	g_CarState &= (~(x));
}
void sys_state_offAlarm(unsigned int x)
{
	if(g_AlarmST&(x))
	{
		g_AlarmST &= (~(x));
	}
}
void sys_state_offSysState(unsigned int x)
{
	g_SysState &= (~(x));
}
unsigned int sys_state_checkCar(unsigned int x)
{
	return g_CarState & (x);
}
unsigned int sys_state_checkAlarm(unsigned int x)
{
	return g_AlarmST & (x);
}
unsigned int sys_state_checkSysState(unsigned int x)
{
	return g_SysState & (x);
}
unsigned int sys_state_get_acc(void)
{
	return g_CarState&(ON_STS_ACC);
}
unsigned int sys_state_get_gpsloc(void)
{
	return g_CarState&(ON_STS_GPSLOC);
}
unsigned int sys_state_get_gpslastloc(void)
{
	return g_lastGpsA&(ON_STS_GPSLOC);
}
unsigned int sys_state_get_power(void)
{
	return g_AlarmST&(ALARMST_POWER_DOWN);
}
unsigned int sys_state_get_car(void)
{
	return g_CarState;
}
unsigned int sys_state_get_alarm(void)
{
	return g_AlarmST;
}
unsigned int sys_state_get_sys(void)
{
	return g_SysState;
}
void sys_state_lastGpsA(void)
{
	g_lastGpsA = g_CarState;	
}
void sys_clear_lastGpsA(void)
{
	g_lastGpsA = 0;
}

void sys_state_newCar9(unsigned char x)
{
	if(0 ==(g_carSta900&(x)))
	{
		g_carSta900 |= (x);
	}
}

void sys_state_offCar9(unsigned char x)
{
	g_carSta900 &= (~(x));
}

unsigned char sys_state_checkCar9(unsigned char x)
{
	return g_carSta900 & (x);
}

unsigned char sys_state_getCar9(void)
{
	return g_carSta900;
}

void sys_state_acc9(unsigned int on)
{	
	if(on)
	{
		sys_state_newCar9(GPIO_ACC_ST);
		sys_state_newCar(ON_STS_ACC);
		if(acc_num == 0)
		{
			acc_num = 1;
			LOG("[gpio] acc on\r\n");
		}
	}
	else
	{
		sys_state_offCar9(GPIO_ACC_ST);
		sys_state_offCar(ON_STS_ACC);
		if(acc_num == 1)
		{
			acc_num = 0;
			LOG("[gpio] acc off\r\n");
		}
	}
}

void sys_state_sos9(unsigned int on)
{
	if(on)
	{
		sys_state_newCar9(GPIO_SOS_ST);
		if(sos_num == 0)
		{
			sos_num = 1;
			LOG("[gpio] sos on\r\n");
			sys_state_newAlarm(ALARMST_HIJACK);
		}
	}
	else
	{
		sys_state_offCar9(GPIO_SOS_ST);
		if(sos_num == 1)
		{
			sos_num = 0;
			LOG("[gpio] sos off\r\n");
			g_AlarmST &= (~(ALARMST_HIJACK));
		}
	}
}

void sys_state_di19(unsigned int on)
{	
	if(on)
	{
		sys_state_newCar9(GPIO_DI1_ST);
	}
	else
	{
		sys_state_offCar9(GPIO_DI1_ST);

	}
}

void sys_state_di29(unsigned int on)
{	
	if(on)
	{
		sys_state_newCar9(GPIO_DI2_ST);
		sys_state_newAlarm(ALARMST_DEFINE1);
	}
	else
	{
		sys_state_offCar9(GPIO_DI2_ST);
		g_AlarmST &= (~(ALARMST_DEFINE1));
	}
	
	
}

void sys_state_di39(unsigned int on)
{	
	if(on)
	{
		sys_state_newCar9(GPIO_DI3_ST);
	}
	else
	{
		sys_state_offCar9(GPIO_DI3_ST);

	}
}

void sys_state_do19(unsigned int on)
{	
	if(on)
	{
		sys_state_newCar9(GPIO_DO1_ST);
	}
	else
	{
		sys_state_offCar9(GPIO_DO1_ST);

	}
}

void sys_state_do29(unsigned int on)
{	
	if(on)
	{
		sys_state_newCar9(GPIO_DO2_ST);
	}
	else
	{
		sys_state_offCar9(GPIO_DO2_ST);

	}
}

void sys_state_do39(unsigned int on)
{	
	if(on)
	{
		sys_state_newCar9(GPIO_DO3_ST);
	}
	else
	{
		sys_state_offCar9(GPIO_DO3_ST);
	}
}

