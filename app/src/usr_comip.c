#include "includes.h"
static unsigned short g_ipvserail_num __noinit__;			//流水号
/**********************************************************************************************/
//IP
void modem_gsm_ipvip(void)
{
	char buf[128]={0};

	modem_send_Atcmd("ATE0","OK",10);

	IF_CONDITION_EC2x
	{
		sprintf((char*)buf,"AT+QIOPEN=1,2,\"TCP\",\"%s\",%d,0,1","IP.007sky.com",3535);	

		g_modem_runtime.nSocketState = RET_AT_WAITING;
		if(modem_send_Atcmd(buf,"OK",30) == RET_AT_SUCCESS)
		{
			SocketWait_SemTake(xSocketSem, (30*1000));
			if(g_modem_runtime.nSocketState == RET_AT_SUCCESS)
			{
				if(modem_send_Atcmd("AT+QISTATE=1,2","+QISTATE:",30) == RET_AT_SUCCESS)
				{
					g_modem_runtime.nIp2State = GSM_IP_STATE;
					g_modem_runtime.nIp2Waittime = 0;
					g_modem_runtime.nIp2ErrCount = 0;
					g_modem_runtime.tick_value_ip2 = 0;
					g_flag.bit.loginIP2Flag = 0;
					OS_SendMessage(xGsmQueueId, MSG_GSM_LOGIN2, 0, 0);

					return;
				}
			}
			else
			{
				g_modem_runtime.nSocketState = RET_AT_TIMEOUT;
				LOG("ip2 -> error/timeout\r\n");
			}
		}
		else
		{
			g_modem_runtime.nSocketState = RET_AT_ERROR;
		}
	}
	
	modem_gsm_disconnect_ip2();
	modem_gsm_ip2_errcheck();
	return;
}


unsigned char modem_send_gprsData2(char* data,char* Erc,unsigned short Time,unsigned short len)
{
	unsigned char buf[128]={0};
	unsigned char ack[16]={0};

	LOG("[ip2] send:");
	LOGX((unsigned char *)data,len);
	
	IF_CONDITION_EC2x
	{	
		sprintf((char *)buf,"AT+QISEND=2,%d",(unsigned short)len);
		strcpy_t((char *)ack,">");
		modem_send_Atcmd((char*)buf,(char*)ack,30);
		
		modem_sendStr((unsigned char *)data, len);
		modem_send_AtStr("\r\n");
		
		modem_clearAtBuf();
		
		if(modem_seek_AtAck(Erc,Time) != RET_AT_SUCCESS)//超时或error
		{
			LOG("[gsm] send return error\r\n");
        	modem_gsm_disconnect_ip2();
			
	        return 0;
	    }
	}
	return 1;
}

//IP2网络
void modem_reconnect_ip2(void)
{
	static unsigned int fixtime = 0;

	if(modem_GetState() == GSM_FOTA_STATE)
	{
		return;
	}

	fixtime ++;
	if(((fixtime%3600) == 0)||(g_flag.bit.openIP2Flag == 1))
	{
		if(g_flag.bit.socketPPPFlag == 1)
		{
			if(g_flag.bit.connIP2Flag == 0)
			{
				g_modem_runtime.nIp1Waittime = 0;
				g_modem_runtime.nIp1ErrCount = 0;	
				
				g_flag.bit.connIP2Flag = 1;
				OS_SendMessage(xGsmQueueId, MSG_GSM_IP2, 0, 0);
			}
			g_flag.bit.openIP2Flag = 0;
		}
	}
}
//IP2发送数据
void modem_send_ip2(void)
{
	//IP2在发送数据,如果IP1一直连不上,清0以防断开整个网络
	g_modem_runtime.nIp1ErrCount = 0;
	if((g_modem_runtime.tick_value_ip2%30) == 0)
	{
		ipv_send_location_msg2();		
	}
}

//无设置VIN,IP2一直在线
//IP2时间到断开网络
void modem_disconnect_ip2(void)
{
	if(sys_GetIP2KeepAlive() && (g_modem_runtime.tick_value_ip2 > (sys_GetIP2KeepAlive()*60)))
	{
		modem_gsm_close_ip2();
	}
}

void modem_gsm_close_ip2(void)
{
	g_flag.bit.connIP2Flag = 0;
	OS_Timer_Stop(IP2_TIMER_ID);

	IF_CONDITION_EC2x
	{
		modem_send_Atcmd("AT+QICLOSE=2","OK",100);
	}
}

void modem_gsm_disconnect_ip2(void)
{
	OS_Timer_Stop(IP2_TIMER_ID);

	IF_CONDITION_EC2x
	{
		modem_send_Atcmd("AT+QICLOSE=2","OK",100);
	}

	g_modem_runtime.nIp2State = GSM_CLOSE_STATE;
	g_modem_runtime.nIp2Waittime = 0;
}

void modem_gsm_reconnect_ip2(void)
{
	if(modem_GetState() == GSM_FOTA_STATE)
	{
		return;
	}
	if(g_modem_runtime.nIp2State == GSM_CLOSE_STATE)
	{
		g_modem_runtime.nIp2Waittime ++;
		if(g_modem_runtime.nIp2Waittime >= 10)
		{
			g_modem_runtime.nIp2State = GSM_IP_STATE;
			g_modem_runtime.nIp2Waittime = 0;
			OS_SendMessage(xGsmQueueId, MSG_GSM_IP2, 0, 0);
		}
	}
}

void modem_gsm_ip2_errcheck(void)
{
	g_modem_runtime.nIp2ErrCount++;
	if(g_modem_runtime.nIp2ErrCount >= 30)
	{
		g_modem_runtime.nIp2State = GSM_INIT_STATE;
		g_modem_runtime.nIp2Waittime = 0;
		g_modem_runtime.nIp2ErrCount = 0;
		modem_send_disconnect(0);
	}
}

void modem_gsm_Login2(void)
{
	if(g_modem_runtime.nLoginCount2 >= 6)//超时
	{
		g_modem_runtime.nLoginCount2 = 0;
		LOG("[ip2] login timeout\r\n");
		modem_gsm_disconnect_ip2();
	}
	else
	{
		ipv_login_msg();
		g_modem_runtime.nLoginCount2++;
		OS_Timer_Start(xGsmQueueId,MSG_GSM_LOGIN2,0,0,IP2_TIMER_ID,1000,FALSE);
	}
	if(g_modem_runtime.nIp2State == GSM_CLOSE_STATE)
	{
		g_modem_runtime.nLoginCount2 = 0;
		OS_Timer_Stop(IP2_TIMER_ID);
	}
}
void modem_gsm_center2(void)
{
	g_modem_runtime.tick_value_ip2++;
	
	modem_send_ip2();
	
	OS_Timer_Start(xGsmQueueId,MSG_GSM_CENTER2,0,0,IP2_TIMER_ID,100,FALSE);// 1s

	if(g_modem_runtime.nIp2State == GSM_CLOSE_STATE)
	{
		OS_Timer_Stop(IP2_TIMER_ID);
	}

	modem_disconnect_ip2();
}
/**********************************************************************************************/
void ipv_splice_seg(ipv_tcp_protocol_data_segment seg, unsigned char **cur, unsigned short *cur_len,unsigned char* data, unsigned short data_len)
{ 
	switch(seg)
	{
		case IPV_TCP_DATA_SEG_HEADER:
			*(*cur) = 0x23;
			*(*cur+1) = 0x23;
			*cur += 2;
			*cur_len += 2;
			break;

		case IPV_TCP_DATA_SEG_CMD:
			*(*cur) = *data;
			*cur += 1;
			*cur_len += 1;
			break;

		case IPV_TCP_DATA_SEG_IMEI:
			strcat_t((char *)*cur, "00");
			strcat_t((char *)*cur, (char *)modem_GetImei());
			*cur += 17;
			*cur_len += 17;
			break;

		case IPV_TCP_DATA_SEG_VER:
			*(*cur) = 0x01;
			*cur += 1;
			*cur_len += 1;
			break;

		case IPV_TCP_DATA_SEG_CRYP:
			*(*cur) = *data;
			*cur += 1;
			*cur_len += 1;
			break;

		case IPV_TCP_DATA_SEG_LEN:
			*(*cur) = (unsigned char)(data_len>>8);
			*(*cur+1) = (unsigned char)data_len;
			*cur += 2;
			*cur_len += 2;
			break;

		case IPV_TCP_DATA_SEG_RAWDATA:
			if(data && data_len)
			{
				memcpy_t(*cur, data, data_len);
				*cur += data_len;
				*cur_len += data_len;
			}
			break;

		case IPV_TCP_DATA_SEG_XOR:
			*(*cur) = xor_check(data+2, data_len-2);
			*cur += 1;
			*cur_len += 1;
			break;

		default:
			break;
	}
}

void ipv_serialize_general(unsigned char cmd, unsigned char cryp, unsigned char* out_data, unsigned short* out_len, unsigned char* in_data, unsigned short in_len)
{
	unsigned char *pp = out_data;
	unsigned short stringLen = 0;//should be the whole string length
	/*header*/
	ipv_splice_seg(IPV_TCP_DATA_SEG_HEADER, &pp, &stringLen ,NULL, 0);
	ipv_splice_seg(IPV_TCP_DATA_SEG_CMD, &pp, &stringLen ,&cmd, 0);
	ipv_splice_seg(IPV_TCP_DATA_SEG_IMEI, &pp, &stringLen ,NULL, 0);
	ipv_splice_seg(IPV_TCP_DATA_SEG_VER, &pp, &stringLen ,NULL, 0);
	ipv_splice_seg(IPV_TCP_DATA_SEG_CRYP, &pp, &stringLen ,&cryp, 0);
	ipv_splice_seg(IPV_TCP_DATA_SEG_LEN, &pp, &stringLen ,NULL, in_len);
	ipv_splice_seg(IPV_TCP_DATA_SEG_RAWDATA, &pp, &stringLen ,in_data, in_len);
	ipv_splice_seg(IPV_TCP_DATA_SEG_XOR, &pp, &stringLen ,out_data, stringLen);

	*out_len = stringLen;
}

//流水号,已天为单位
void ipv_serialnum_noinit(void)
{
	g_ipvserail_num = 0;
}

//GB17691
unsigned int ipv_latlon_to_double(char *str)
{
    char degree[5] = {0};
    char cent[64] = {0};
    char *p = str;
	double db;
	unsigned int ret;
    
    degree[0] = *p;
    p++;
    degree[1] = *p;
    p++;
    degree[2] = *p;
    p++;

    db = myatof(degree);

    strcpy_t(cent, p);

    ret = ((db) + (myatof(cent) /60))*100000;

	return ret;
}


void ipv_login_msg(void)
{
	unsigned char sendbuf[64]={0};
	unsigned char databuf[32]={0};
	unsigned short p = 0,len = 0;
	TIME rtc;

	memcpy_t(&rtc,sys_GetSysRTC(),sizeof(TIME));
	databuf[p++] = (rtc.year>2000)?(rtc.year-2000):0;
	databuf[p++] = rtc.month;
	databuf[p++] = rtc.day;
	databuf[p++] = rtc.hour;
	databuf[p++] = rtc.minute;
	databuf[p++] = rtc.second;

	g_ipvserail_num ++;
	p += uint16_to_uint8(g_ipvserail_num, &databuf[p]);

	memcpy_t(&databuf[p], modem_GetCcid(), strlen_t((char *)modem_GetCcid()));
	p += strlen_t((char *)modem_GetCcid());
	
	ipv_serialize_general(0x01,0x01,sendbuf,&len,databuf,p);
	modem_send_gprsData2((char*)sendbuf,"OK",50,len);
}

void ipv_request_time_msg(void)
{
	unsigned char sendbuf[64]={0};
	unsigned char databuf[32]={0};
	unsigned short p = 0,len = 0;

	g_ipvserail_num ++;
	p += uint16_to_uint8(g_ipvserail_num, &databuf[p]);
	
	ipv_serialize_general(0xC0,0x01,sendbuf,&len,databuf,p);
	modem_send_gprsData2((char*)sendbuf,"OK",50,len);
}

//主动上报参数信息
void ipv_send_param_msg(void)
{
	unsigned char buf[256]={0};
	unsigned char sendbuf[256]={0};
	unsigned short blen=0,slen=0;

	memcpy_t(&buf[blen], modem_GetImei(), 15);
	blen += 15;

	memcpy_t(&buf[blen], modem_GetCcid(), 20);
	blen += 20;

	buf[blen++] = strlen_t(APP_VER);
	memcpy_t(&buf[blen], APP_VER, strlen_t(APP_VER));
	blen += strlen_t(APP_VER);

	buf[blen++] = 0;

	memcpy_t(&buf[blen], "0000000000000000", 16);
	blen += 16;

	memcpy_t(&buf[blen], sys_GetCarVin(), 17);
	blen += 17;
	//备案状态
	buf[blen++] = 0;

	buf[blen++] = sys_GetGpsState();

	memcpy_t(&buf[blen], sys_GetTermSN(), 17);
	blen += 17;

	blen += sprintf((char *)&buf[blen], "%s%s,%d,", sys_GetIp(),sys_GetUrl(),sys_GetPort());
	
	ipv_serialize_general(0x83,0x01,sendbuf,&slen,buf,blen);
	modem_send_gprsData2((char*)sendbuf,"OK",30,slen);
}

int ipv_serialize_pid(unsigned char *pout)
{
	unsigned short p=0;
	gpsinfo pfull;
	gpsinfo *lastgpsdata=sys_GetLastGpsData();
	
	if(sys_state_get_gpsloc())
	{
		memcpy_t((char *)&pfull,(char *)gps_GetGpsInfo(),sizeof(gpsinfo));
	}
	else
	{
		memcpy_t((char *)&pfull,(char *)lastgpsdata,sizeof(gpsinfo));
		strcpy_t(pfull.speed_str,"00.00");
	}

	//车速
	pout[p++] = 0xFF;
	pout[p++] = 0xFF;
	//大气压力
	pout[p++] = 0xFF;
	//发动机净输出扭矩
	pout[p++] = 0xFF;
	//摩擦扭矩
	pout[p++] = 0xFF;
	//发动机转速
	p += uint16_to_uint8(sys_state_get_acc()?999:0, &pout[p]);	
	//发动机燃料流量
	pout[p++] = 0xFF;
	pout[p++] = 0xFF;
	//SCR上游NOx传感器输出值
	pout[p++] = 0xFF;
	pout[p++] = 0xFF;
	//SCR下游NOx传感器输出值
	pout[p++] = 0xFF;
	pout[p++] = 0xFF;
	//反应剂余量
	pout[p++] = 0xFF;
	//空气质量流量传感器读取的进气量
	pout[p++] = 0xFF;
	pout[p++] = 0xFF;
	//SCR入口温度
	pout[p++] = 0xFF;
	pout[p++] = 0xFF;
	//SCR出口温度
	pout[p++] = 0xFF;
	pout[p++] = 0xFF;
	//DPF压差
	pout[p++] = 0xFF;
	pout[p++] = 0xFF;
	//发动机冷却液温度
	pout[p++] = 0xFF;
	//油箱液位
	pout[p++] = 0xFF;
	//定位状态
	if(sys_state_get_gpsloc())
	{
		pout[p++] = 0;
	}
	else
	{
		pout[p++] = 1;
	}
	//经度
	p += uint32_to_uint8(ipv_latlon_to_double(pfull.longitude_str), &pout[p]);
	//纬度
	p += uint32_to_uint8(ipv_latlon_to_double(pfull.latitude_str), &pout[p]);
	//累计里程
	pout[p++] = 0xFF;
	pout[p++] = 0xFF;
	pout[p++] = 0xFF;
	pout[p++] = 0xFF;

	return p;
}

//组位置信息包
void ipv_serialize_location(unsigned char cmd, unsigned char* out_data, unsigned short* out_len)
{
	unsigned char databuf[256]={0};
	unsigned short p=0;
	TIME rtc;

	if(sys_state_get_gpsloc())
	{
		memcpy_t(&rtc,sys_GetSysRTC(),sizeof(TIME));
		//上传时间为gps时间
		LOG("[msg] gps time:%02d-%02d-%02d	%02d:%02d:%02d\r\n",
					rtc.year,rtc.month,rtc.day,rtc.hour,rtc.minute,rtc.second);
	}
	else
	{
		memcpy_t(&rtc,sys_GetSysRTC(),sizeof(TIME));
		//上传时间为rtc时间
		LOG("[msg] rtc time:%02d-%02d-%02d	%02d:%02d:%02d\r\n",
					rtc.year,rtc.month,rtc.day,rtc.hour,rtc.minute,rtc.second);
	}
	databuf[p++] = (rtc.year>2000)?(rtc.year-2000):0;
	databuf[p++] = rtc.month;
	databuf[p++] = rtc.day;
	databuf[p++] = rtc.hour;
	databuf[p++] = rtc.minute;
	databuf[p++] = rtc.second;

	databuf[p++] = 0x02;

	g_ipvserail_num ++;
	p += uint16_to_uint8(g_ipvserail_num, &databuf[p]);
	//PID
	p += ipv_serialize_pid(&databuf[p]);
	
	ipv_serialize_general(cmd,0x01,out_data,out_len,databuf,p);
}

void ipv_send_location_msg2(void)
{
	unsigned char buf[128]={0};
	unsigned short len=0;
	if(modem_GetState() == GSM_FOTA_STATE) return;
	ipv_serialize_location(0x02,buf,&len);
	modem_send_gprsData2((char*)buf,"OK",30,len);
}

//AGPS请求星历数据(0xA5)
void ipv_agps_req(void)
{
	unsigned char sendbuf[64]={0};
	unsigned char databuf[32]={0};
	unsigned short p = 0,len = 0;
	
	if(sys_state_get_gpsloc())
	{
		return;
	}
	
	databuf[p++] = 0;
	databuf[p++] = 2;

	//经度
	p += uint16_to_uint8(1130, &databuf[p]);
	//纬度
	p += uint16_to_uint8(220, &databuf[p]);

	ipv_serialize_general(0xA5,0x01,sendbuf,&len,databuf,p);
	modem_send_gprsData2((char*)sendbuf,"OK",50,len);
}

//应答包
void ipv_ack(void)
{
	if(g_flag.bit.loginIP2Flag == 0)
	{
		LOG("[ip2] 终端登陆成功\r\n");
		g_flag.bit.loginIP2Flag = 1;
		g_modem_runtime.nLoginCount2 = 0;
		OS_Timer_Stop(IP2_TIMER_ID);
		OS_SendMessage(xGsmQueueId, MSG_GSM_CENTER2, 0, 0);
		ipv_send_param_msg();
		ipv_request_time_msg();
//		ipv_agps_req();
	}
	if(gc_debug == 1)
	{
		modem_SetState(GSM_ONLINE_STATE);
	}
}

//平台校时应答
void ipv_cal_timing_ack(unsigned char *data, unsigned short len)
{
	TIME rtc;

	rtc.year   = data[2] + 2000;
	rtc.month  = data[3];
	rtc.day    = data[4];
	rtc.hour   = data[5];
	rtc.minute = data[6];
	rtc.second = data[7];
	
	sys_SetSysRTC(&rtc);
	LOG("[ip2] 终端校时成功\r\n");
}

//文本信息应答
void ipv_send_txt_ack(unsigned short serialno, unsigned char result, unsigned char *ack, unsigned short alen)
{
	unsigned char buf[384]={0};
	unsigned char sendbuf[384]={0};
	unsigned short blen=0,slen=0;

	buf[blen++] = (serialno>>8);
	buf[blen++] = (serialno&0xFF);
	buf[blen++] = result;
	memcpy_t(&buf[blen], ack, alen);
	blen += alen;

	ipv_serialize_general(0x81,0x01,sendbuf,&slen,buf,blen);
	modem_send_gprsData2((char*)sendbuf,"OK",30,slen);
}

/*
num : 星历数据总包数
rang : 星历数据当前包号
buf : agps数据
buflen : 长度
*/
void ipvagps_Recv_Handle(unsigned char num,unsigned char rang,unsigned char* buf,unsigned short buflen) 
{

	LOG("[agps] %d-%d-%d\r\n", num,rang,buflen);
	agps_sendCmd(buf, buflen);
}

//接收处理
void ipv_recv_Handle(char* buf,unsigned short buflen)
{
	unsigned char cmd,ret;
	unsigned char *msg = NULL;
	unsigned short len,serialno;
	unsigned char rbuf[384]={0};
	
	if ((buf==NULL) || (buflen==0))
	{
	   	return;
	}

	LOG("[ip2] rcv:%d\r\n",buflen);
//	LOGX((unsigned char *)buf,buflen);
	
	if((buf[0] != 0x23)&&(buf[1] != 0x23))
	{
		return;
	}
	cmd = buf[2];
	len = uint8_to_uint16((unsigned char *)&buf[22]);
	msg = (unsigned char *)&buf[24];
	
	switch(cmd)
	{
		case 0xA0://应答包
			ipv_ack();
			break;
		case 0xA6://AGPS应答星历数据(0xA6)
			ipvagps_Recv_Handle(msg[0],msg[1],&msg[2],len-2) ;
			break;
		case 0xC1://校时
			ipv_cal_timing_ack(msg, len);
			break;
		case 0x80://扩展文本信息
			len = uint8_to_uint16((unsigned char *)&buf[22]) - 2;
			serialno = uint8_to_uint16((unsigned char *)&buf[24]);
			buf[buflen - 1] = 0;
			if(strstr_t((char *)&buf[26], "*F10#F"))
			{
				sprintf((char *)rbuf,"Reset");
				ret = PC_FACTORY;
			}
			else
			{
				ret = handset_ConfigSet((unsigned char *)&buf[26],len,rbuf);
			}
			if(strlen_t((char *)rbuf) > 0)
			{
				ipv_send_txt_ack(serialno, 0x01, rbuf, strlen_t((char *)rbuf));
				if(ret == PC_DISCONNECT_TCPIP)
				{
					//重新计时
					g_modem_runtime.tick_value_ip2 = 0;
					ipv_send_param_msg();
//					Msg_Clear();
			        modem_gsm_disconnect_ip1();
				}
				else if(ret == PC_RESTART)
				{
					OS_TaskDelayMs(3000);
					sys_Restart();
				}
				else if(ret == PC_FACTORY)
				{
					OS_TaskDelayMs(3000);
					sys_ResetFacitory();
					sys_Restart();
				}
				else{;}
			}
			else
			{
				ipv_send_txt_ack(serialno, 0x03, NULL, 0);
			}
			break;
		default:
			break;
	}
}

