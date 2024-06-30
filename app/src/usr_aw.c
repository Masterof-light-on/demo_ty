#include "includes.h"

static unsigned short g_up_seq_num=0;					//上行流水号
static unsigned short g_register_seq=0;				//下行流水号
static unsigned char  sLoginsel[SET_LOGIN_SEL_LEN+1];	//终端鉴权码
static unsigned char g_register_sign=0;				//注册标志

static unsigned char g_sendpack[DATALEN+1];				//
static unsigned char g_sendinfo[512];				//
static unsigned char g_escapepack[DATALEN+1];				//
//组消息包
int oa_JT_general_msg(Gbroad_msg msg,unsigned char* pdst)
{
	unsigned short p,j,len;

	memset_t(g_escapepack,0,DATALEN+1);
	
	len = (msg.msgattr&MSG_LENINVALID);
	msg.msgattr &= ~MSG_ENCODE;//不加密

	p = 0;
	//消息ID
	p += uint16_to_uint8(msg.cmdid,&g_escapepack[p]);
	//消息属性
	p += uint16_to_uint8(msg.msgattr,&g_escapepack[p]);
	//终端手机号
	memcpy_t((char *)&g_escapepack[p],sys_GetId1(),FAKE_IP_LE);
	p += FAKE_IP_LE;
	//流水号
	p += uint16_to_uint8(msg.msgsel,&g_escapepack[p]);
	if(msg.msgattr&MSG_MULTY)
	{
		//消息包封装项
		p += uint16_to_uint8(msg.msgpack.total,&g_escapepack[p]);
		p += uint16_to_uint8(msg.msgpack.cur_bag,&g_escapepack[p]);
	}
	//消息体
	if(msg.msg && len)
	{
		memcpy_t(&g_escapepack[p],msg.msg,len);
		p += len;
	}

	//校验
	g_escapepack[p] = xor_check(&g_escapepack[0],p);
	p ++;

	//报文转义
	len = 1;
	for(j=0;j<p;j++)
	{
		switch(g_escapepack[j])
		{
			case 0x7D:
				pdst[len++] = 0x7D;
				pdst[len++] = 0x01;
				break;
			case 0x7E:
				pdst[len++] = 0x7D;
				pdst[len++] = 0x02;
				break;
			default:
				pdst[len++] = g_escapepack[j];
				break;
		}
	}

	pdst[0] = GBROAD_MSG_SIGN;		//帧头
	pdst[len++] = GBROAD_MSG_SIGN;	//帧尾
	p = len;
	
	return p;
}


//流水号管理
unsigned short oa_change_UpSel(unsigned short sel)
{
	unsigned short sel_cur;

	sel_cur = g_up_seq_num;
	g_up_seq_num += sel;

	return sel_cur;
}

//获取TLV
int oa_get_TLV(unsigned char *msg,TLV *tlv)
{
	tlv->type = uint8_to_uint32(&msg[0]);
	tlv->len = msg[4];
	tlv->msg = (unsigned char*)&msg[5];

	return (5+tlv->len);
}

//获取ext TLV
void oa_get_ext_TLV(unsigned char *msg,TLV *tlv)
{
	tlv->type = uint8_to_uint16(&msg[0]);
	tlv->len = uint8_to_uint16(&msg[2]);
	tlv->msg = (unsigned char*)&msg[4];
}

//type:分高低16位使用,低16位表示cmd id
int oa_JT_locate(unsigned int type,Gbroad_msg msg,unsigned char* pdst)
{
	msg.cmdid = type&0xFFFF;
	if(type&MSG_UP_ACK)
	{
	}
	else
	{
		msg.msgsel = oa_change_UpSel(1);
	}
	return oa_JT_general_msg(msg,pdst);
}


//注册
int oa_JT_create_register_msg(unsigned char *pout)
{
	Gbroad_msg msg = {0};
	unsigned char buf[128]={0};

	int p;

	p = 0;
	p += uint16_to_uint8(sys_GetProvinceId(),&buf[p]);	//省ID
	p += uint16_to_uint8(sys_GetCityId(),&buf[p]);		//市ID
	memcpy_t(&buf[p],sys_GetManuId(),5);				//制造商ID
	p += 5;

	memcpy_t(&buf[p],sys_GetTermType(),20);				//终端型号20bytes
	p += 20;

	memcpy_t(&buf[p],sys_GetTermId(),7);				//终端ID
	p += 7;
	buf[p++] = 1;
	
	memcpy_t(&buf[p],g_sys.g_login.sImeiDevID,8);			//车辆 VIN
	p += 8;

	msg.msgsel = oa_change_UpSel(1);
	g_register_seq = msg.msgsel;
	msg.msgattr = p;
	msg.msg = buf;
	p = oa_JT_locate(MSG_GPRS_TERREG|MSG_UP_ACK,msg,pout);

	return p;
}

//登录
int oa_JT_create_login_msg(unsigned char *sLoginsel,unsigned char *pout)
{
	Gbroad_msg msg = {0};
	int p;

	p = strlen_t((char *)sLoginsel);
	if(p > SET_LOGIN_SEL_LEN)
	{
		p = SET_LOGIN_SEL_LEN;
	}
	msg.msg = sLoginsel;
	msg.msg[p] = 0;

	msg.msgsel = oa_change_UpSel(1);
	g_register_seq = msg.msgsel;
	msg.msgattr = strlen_t((char*)msg.msg);
	p = oa_JT_locate(MSG_GPRS_TERLOGIN|MSG_UP_ACK,msg,pout);

	return p;
}

//注销
int oa_JT_create_logout_msg(unsigned char *pout)
{
	Gbroad_msg msg = {0};
	unsigned short ret;	

	ret = oa_JT_locate(MSG_GPRS_TERLOGOUT,msg,pout);

	return ret;
}


//创建附加信息
int oa_JT_create_append_Ext(unsigned char type,unsigned char len,unsigned int val,unsigned char *pout)
{
	int p = 0; 

	pout[p++] = type;
	pout[p++] = len;

	switch(len)
	{
		case 1:
			pout[p++] = val;
			break;
		case 2:
			p += uint16_to_uint8(val,&pout[p]);
			break;
		case 4:
			p += uint32_to_uint8(val,&pout[p]);
			break;
		default:
			p = 0;
			break;
	}

	return p;
}

//透传协议命令字
unsigned short oa_JT_create_TLV_GeneralExt(unsigned short type,unsigned char *pin,unsigned short len,unsigned char *pout)
{
	unsigned short p;

	p = uint16_to_uint8(type,&pout[0]);
	p += uint16_to_uint8(len,&pout[p]);
	if(pin && len)
	{
		memcpy_t((char*)&pout[p],pin,len);
		p += len;
	}

	return p;
}

//gps数据组包
int oa_JT_serialize_gps(unsigned char *pout,unsigned char sign)
{
	gpsinfo pfull;
	gpsinfo *lastgpsdata=sys_GetLastGpsData(); 
	double fTmp = 0.0;
	unsigned int tmp=0;
	int p=0;
	
	if(sys_state_get_gpsloc())
	{
		//实时更新位置信息
		if(gps_update_location_condition() == TRUE)
		{
			memcpy_t((char *)&pfull,(char *)gps_GetGpsInfo(),sizeof(gpsinfo));
		}
		//最后一点的位置
		else
		{
			memcpy_t((char *)&pfull,(char *)lastgpsdata,sizeof(gpsinfo));
			strcpy_t(pfull.speed_str,"00.00");
			gps_GetGpsTime(&pfull.date);
		}
		//上传时间为gps时间
		LOG("[msg] gps time:%02d-%02d-%02d  %02d:%02d:%02d\r\n",pfull.date.year,pfull.date.month,pfull.date.day,pfull.date.hour,pfull.date.minute,pfull.date.second);
	}
	else
	{
		memcpy_t((char *)&pfull,(char *)lastgpsdata,sizeof(gpsinfo));
		strcpy_t(pfull.speed_str,"00.00");
		
		//上传时间为rtc时间
		memcpy_t(&pfull.date,sys_GetSysRTC(),sizeof(TIME));
		LOG("[msg] rtc time:%02d-%02d-%02d  %02d:%02d:%02d\r\n",pfull.date.year,pfull.date.month,pfull.date.day,pfull.date.hour,pfull.date.minute,pfull.date.second);
	}

	//报警标志,状态标志
	uint32_to_uint8(sys_state_get_alarm(),(unsigned char*)&pout[0]);
	uint32_to_uint8(sys_state_get_car(),(unsigned char*)&pout[4]);
	p = 8;

	//纬度(4)
	parse_lat_or_lon_to_double(pfull.latitude_str,&fTmp);
	tmp = (unsigned int)(fTmp*1000000);
	p += uint32_to_uint8(tmp,&pout[p]);

	//经度(4)
	parse_lat_or_lon_to_double(pfull.longitude_str,&fTmp);
	tmp = (unsigned int)(fTmp*1000000);
	p += uint32_to_uint8(tmp,&pout[p]);
	
	//高度(2)
	tmp = gps_GetGpsAltitude();
	p += uint16_to_uint8(tmp,&pout[p]);

	//速度(2)
	tmp = (unsigned int)(myatof(pfull.speed_str)*1.852f*10);//km/h
	p += uint16_to_uint8(tmp,&pout[p]);

	//方向(2)
	tmp = (unsigned int)(myatoi(pfull.head_str));
	p += uint16_to_uint8(tmp,&pout[p]);
	
	//年月日时分秒
	pout[p++] = HexToCompactBCD(pfull.date.year-2000);
	pout[p++] = HexToCompactBCD(pfull.date.month);
	pout[p++] = HexToCompactBCD(pfull.date.day);
	pout[p++] = HexToCompactBCD(pfull.date.hour);
	pout[p++] = HexToCompactBCD(pfull.date.minute);
	pout[p++] = HexToCompactBCD(pfull.date.second);

	if(sign)
	{
		//附加信息
		//里程,100m
		p += oa_JT_create_append_Ext(GPRS_APPEND_Millage,4,sys_GetMileAge()/100,&pout[p]);	

		//CSQ
		p += oa_JT_create_append_Ext(GPRS_APPEND_CSQ,1,modem_GetCsq(),&pout[p]);
		
		//星数
		p += oa_JT_create_append_Ext(GPRS_APPEND_GPS_STA,1,gps_GetGpsStarNum(),&pout[p]);

		//模拟量
//		if(serialoil_enable_urtfun(ALL) == ENABLE)
//		{
//			p += oa_JT_create_append_Ext(GPRS_APPEND_AD,4,serialoil_get_percent(1),&pout[p]);
		
			pout[p++] = GPRS_APPEND_AD;
			pout[p++] = 4;
			p += uint16_to_uint8(serialoil_get_percent(1),&pout[p]);
			p += uint16_to_uint8(serialoil_get_percent(2),&pout[p]);
	
//		}
			
		pout[p++] = 0xef;//蓝牙
		pout[p++] = 0x04;
		pout[p++] = 0x06;//id
		pout[p++] = 0x02;//id
		p += uint16_to_uint8(xdevinfo.tempbt+4000,&pout[p]);
		
		pout[p++] = 0xf0;//ain1 电压
		pout[p++] = 0x02;
		p += uint16_to_uint8(sDevPowerVolt[4]*10,&pout[p]);
		
		pout[p++] = 0xf1;//ain2 电压
		pout[p++] = 0x02;
		p += uint16_to_uint8(sDevPowerVolt[3]*10,&pout[p]);
		
		pout[p++] = 0xf2;//ain3 电压
		pout[p++] = 0x02;
		p += uint16_to_uint8(sDevPowerVolt[0]*10,&pout[p]);
	}

	return p;
}


//对接收到平台的进行数据转义,校验,解密,解压
int oa_JT_deal_msg(char *msg,int len,Gbroad_msg *pout)
{
	//暂按消息体为分包后单独加密压缩处理
	int msglen;
	unsigned short ret;
	unsigned int plen = 0;

	if(msg[0] != GBROAD_MSG_SIGN)
	{
		return 0;
	}
	//取其中一个数据包
	for(plen=12;plen<len;plen++)
	{
		if(msg[plen] == GBROAD_MSG_SIGN)
		{
			break;
		}
	}
	if(plen >= len || plen < 14)
	{
		return 0;
	}
	plen += 1;

	//报文转义
	msglen = 0;
	for(ret=0;ret<plen;ret++,msglen++)
	{
		if(msg[ret] == 0x7D)	//0x01 -> 0x7D    0x02->0x7E
		{
			ret ++;
			if(msg[ret] == 0x01)
			{
				msg[msglen] = 0x7D;
			}
			else
			{
				msg[msglen] = 0x7E;
			}
		}
		else
		{
			msg[msglen] = msg[ret];
		}
	}

	//校验
	ret = xor_check((unsigned char*)&msg[1],msglen-3)&0xFF;
	if(ret != msg[msglen-2])
	{
		goto end;
	}
	//比较终端ID
	if(memcmp_t(sys_GetId1(),&msg[5],FAKE_IP_LE))
	{
		goto end;
	}

	ret = 1;
	//消息ID
	pout->cmdid = uint8_to_uint16((unsigned char*)&msg[ret]);
	ret += 2;
	//消息属性
	pout->msgattr = uint8_to_uint16((unsigned char*)&msg[ret]);
	ret += 2;
	if((pout->msgattr&MSG_LENINVALID)+19 != msglen && (pout->msgattr&MSG_LENINVALID)+15 != msglen)
	{
		goto end;
	}
	//终端手机号msg[5]~msg[10]
	ret += 6;
	//流水号
	pout->msgsel = uint8_to_uint16((unsigned char*)&msg[ret]);
	ret += 2;
	if(pout->msgattr&MSG_MULTY)
	{
		//消息包封装项
		pout->msgpack.total = uint8_to_uint16((unsigned char*)&msg[ret]);
		ret += 2;
		pout->msgpack.cur_bag = uint8_to_uint16((unsigned char*)&msg[ret]);
		ret += 2;
	}

	memcpy_t((char*)pout->msg,&msg[ret],(pout->msgattr&MSG_LENINVALID));
	return msglen;
end:

	return (msglen|0x80000);
}

//通用应答
int oa_JT_create_general_ack(Gbroad_msg msg,unsigned char status,unsigned char *pout)
{
	unsigned char buf[5]={0};

	uint16_to_uint8(msg.msgsel,&buf[0]);
	uint16_to_uint8(msg.cmdid,&buf[2]);
	buf[4] = status;

	msg.msg = buf;
	msg.msgattr = 5;	//不分包,不加密

	return oa_JT_locate(MSG_GPRS_TERGENACK,msg,(unsigned char *)pout);
}


//登陆检查
int oa_JT_check_login(char *msg,int len)
{
	int status = 0;
	unsigned char ret = 0,msglen = 0;
	unsigned char buf[128]={0};
	unsigned char sendbuf[64]={0};
	Gbroad_msg msg_rcv = {0};
	unsigned short tmp = 0;

	LOG("[ip1] rcv:");
	LOGX((unsigned char *)msg,len);
	
	msg_rcv.msg = buf;
	while(len >= 17)
	{
		status = oa_JT_deal_msg(msg,len,&msg_rcv);
		if(status == 0)
		{
			break;
		}
		if(status & 0x80000)	//终端ID不符
		{
			status &= 0xFFFF;
			msg += status;
			len -= status;
			continue;
		}

		tmp = uint8_to_uint16((unsigned char*)&msg_rcv.msg[0]);
		if(tmp != g_register_seq)
		{
			msg += status;
			len -= status;
			continue;
		}

		msg += status;
		len -= status;
		ret = 0;
		switch(msg_rcv.cmdid)
		{
			case MSG_GPRS_CENGENACK:		//0x8001	//平台通用应答
				tmp = uint8_to_uint16((unsigned char*)&msg_rcv.msg[2]);
				if(tmp == MSG_GPRS_TERLOGIN)
				{
					if(msg_rcv.msg[4] == 0x00)
					{
						ret = 1;
						g_register_sign = 0;
						g_modem_runtime.npppCount    = 0;
						g_modem_runtime.nLoginCount  = 0;
						g_modem_runtime.nCenterCount = 0;
						
						if(modem_GetState() == GSM_LOGIN_STATE)
						{
							LOG("[ip1] 终端鉴权成功\r\n");	
						}
						OS_Timer_Stop(GSM_TIMER_ID);
						OS_SendMessage(xGsmQueueId, MSG_GSM_CENTER, 0, 0);
					}
					else
					{
						g_register_sign = 0;		//无效
						modem_send_disconnect(0);
					}
				}
				break;
			case MSG_GPRS_CENREGACK:		//0x8100	//终端注册
				if((msg_rcv.msgattr&MSG_LENINVALID) > SET_LOGIN_SEL_LEN+3)
				{
					msglen = oa_JT_create_general_ack(msg_rcv,MSG_IS_NOT_SUPORT&0x0F,sendbuf);		//通用回复
					modem_send_gprsData((char*)sendbuf,"OK",30,msglen);
					break;
				}
				if(msg_rcv.msg[2] == 2 || msg_rcv.msg[2] == 4)
				{
					//平台未登记
					LOG("[ip1] 无该车辆/终端信息\r\n");
					break;
				}
				if(msg_rcv.msg[2] == 1 || msg_rcv.msg[2] == 3)
				{
					LOG("[ip1] 车辆/终端已被注册\r\n");
					g_register_sign = 0;		//无效
					ret = 0;
					msglen = oa_JT_create_logout_msg(sendbuf);
					modem_send_gprsData((char*)sendbuf,"OK",30,msglen);
					break;
				}
				LOG("[ip1] 车辆/终端注册成功\r\n");
				memset_t((char*)sLoginsel,0,SET_LOGIN_SEL_LEN+1);
//				memcpy_t((char*)sLoginsel,(char*)&msg_rcv.msg[3],(msg_rcv.msgattr&MSG_LENINVALID)-3);
				memcpy_t((char*)sLoginsel,(char*)&msg_rcv.msg[4],(msg_rcv.msgattr&MSG_LENINVALID)-4);
				g_register_sign = 1;			//有效
				g_modem_runtime.npppCount   = 0;
				g_modem_runtime.nLoginCount = 0;

				OS_Timer_Stop(GSM_TIMER_ID);
				OS_SendMessage(xGsmQueueId, MSG_GSM_LOGIN, 0, 0);
						
				ret = 1;
				break;
			default:
				break;
		}

	}
	
	return ret;
}


//组位置信息包
void aw_serialize_location(unsigned char *buf, unsigned short *buflen)
{
	Gbroad_msg msg = {0};
	unsigned char datbuf[256] = {0};
	
	msg.msgattr = oa_JT_serialize_gps(datbuf,0x01);
	msg.msgattr &= ~MSG_MULTY;
	msg.msg = datbuf;
	*buflen = oa_JT_locate(MSG_GPRS_GPSMSG,msg,buf);
}

//握手
bool aw_udp_hangup(void)
{
	unsigned char sendbuf[128]={0};
	unsigned short len = 0;

	if(g_register_sign == 0)//注册
	{
		len = oa_JT_create_register_msg(sendbuf);
	}
	else//登录
	{
		len = oa_JT_create_login_msg(sLoginsel,sendbuf);
	}
	modem_send_gprsData((char*)sendbuf,"OK",30,len);

	return TRUE;
}

//心跳
bool aw_udp_keepalive(void)
{
	unsigned char sendbuf[32]={0};
	unsigned short len = 0;
	Gbroad_msg msg = {0};

	len = oa_JT_locate(MSG_GPRS_KEEPALIVE,msg,(unsigned char *)sendbuf);
	modem_send_gprsData((char*)sendbuf,"OK",50,len);

	return TRUE;
}


//盲区
void oa_serialize_location_history(unsigned char *buf, unsigned short *buflen)
{
	Gbroad_msg msg = {0};
	unsigned char gpsbuf[256] ={0};
	unsigned short gpslen = 0;
	
	gpslen = oa_JT_serialize_gps(&gpsbuf[5],0x01);
	
	gpsbuf[0] = 0x00;
	gpsbuf[1] = 0x01;
	gpsbuf[2] = 0x01;
	gpsbuf[3] = (unsigned char)(gpslen>>8);
	gpsbuf[4] = (unsigned char)(gpslen&0xFF);

	msg.msgattr = gpslen+5;
	msg.msgattr &= ~MSG_MULTY;
	msg.msg = gpsbuf;
	*buflen = oa_JT_locate(MSG_GPRS_BATCHUPLOAD,msg,buf);
}

//位置信息
void aw_send_location_msg(void)
{
	unsigned char buf[256]={0};
	unsigned short len=0;
	if(modem_GetState() == GSM_ONLINE_STATE)
	{
		aw_serialize_location(buf,&len);
	}
	else
	{
		oa_serialize_location_history(buf,&len);
	}
	MsgGsm_InputMsg(buf,len);
}

//gps数据组包
int oa_JT_serialize_gpsrfid(unsigned char *pout,unsigned char sign,unsigned char type)
{
	gpsinfo pfull;
	gpsinfo *lastgpsdata=sys_GetLastGpsData(); 
	double fTmp = 0.0;
	unsigned int tmp=0;
	int p=0;
	
	if(sys_state_get_gpsloc())
	{
		//实时更新位置信息
		if(gps_update_location_condition() == TRUE)
		{
			memcpy_t((char *)&pfull,(char *)gps_GetGpsInfo(),sizeof(gpsinfo));
		}
		//最后一点的位置
		else
		{
			memcpy_t((char *)&pfull,(char *)lastgpsdata,sizeof(gpsinfo));
			strcpy_t(pfull.speed_str,"00.00");
			gps_GetGpsTime(&pfull.date);
		}
		//上传时间为gps时间
		LOG("[msg] gps time:%02d-%02d-%02d  %02d:%02d:%02d\r\n",pfull.date.year,pfull.date.month,pfull.date.day,pfull.date.hour,pfull.date.minute,pfull.date.second);
	}
	else
	{
		memcpy_t((char *)&pfull,(char *)lastgpsdata,sizeof(gpsinfo));
		strcpy_t(pfull.speed_str,"00.00");
		
		//上传时间为rtc时间
		memcpy_t(&pfull.date,sys_GetSysRTC(),sizeof(TIME));
		LOG("[msg] rtc time:%02d-%02d-%02d  %02d:%02d:%02d\r\n",pfull.date.year,pfull.date.month,pfull.date.day,pfull.date.hour,pfull.date.minute,pfull.date.second);
	}

	pout[p++] = type;
	
	//报警标志,状态标志
	p += uint32_to_uint8(sys_state_get_alarm(),&pout[p]);

	p += uint32_to_uint8(sys_state_get_car(),&pout[p]);


	//纬度(4)
	parse_lat_or_lon_to_double(pfull.latitude_str,&fTmp);
	tmp = (unsigned int)(fTmp*1000000);
	p += uint32_to_uint8(tmp,&pout[p]);

	//经度(4)
	parse_lat_or_lon_to_double(pfull.longitude_str,&fTmp);
	tmp = (unsigned int)(fTmp*1000000);
	p += uint32_to_uint8(tmp,&pout[p]);
	
	//高度(2)
	tmp = gps_GetGpsAltitude();
	p += uint16_to_uint8(tmp,&pout[p]);

	//速度(2)
	tmp = (unsigned int)(myatof(pfull.speed_str)*1.852f*10);//km/h
	p += uint16_to_uint8(tmp,&pout[p]);

	//方向(2)
	tmp = (unsigned int)(myatoi(pfull.head_str));
	p += uint16_to_uint8(tmp,&pout[p]);
	
	//年月日时分秒
	pout[p++] = HexToCompactBCD(pfull.date.year-2000);
	pout[p++] = HexToCompactBCD(pfull.date.month);
	pout[p++] = HexToCompactBCD(pfull.date.day);
	pout[p++] = HexToCompactBCD(pfull.date.hour);
	pout[p++] = HexToCompactBCD(pfull.date.minute);
	pout[p++] = HexToCompactBCD(pfull.date.second);

	if(sign)
	{
		memcpy_t(&pout[p],Byte_buf,8);
		p += 8;
	}

	return p;
}


//组位置信息包
void aw_serialize_rfid(unsigned char *buf, unsigned short *buflen,unsigned char type)
{
	Gbroad_msg msg = {0};
	unsigned char datbuf[256] = {0};
	
	msg.msgattr = oa_JT_serialize_gpsrfid(datbuf,0x01,type);
	msg.msgattr &= ~MSG_MULTY;
	msg.msg = datbuf;
	*buflen = oa_JT_locate(0x0501,msg,buf);
}

//RFID 信息上报
void aw_send_rfid_msg(unsigned char type)
{
	unsigned short len=0;

	memset_t(g_sendinfo,0,sizeof(g_sendinfo));
	aw_serialize_rfid(g_sendinfo,&len,type);
	MsgGsm_InputMsg(g_sendinfo,len);
}

//报警信息
void aw_send_alarm_msg(void)
{
	unsigned short len=0;
	memset_t(g_sendinfo,0,sizeof(g_sendinfo));
	aw_serialize_location(g_sendinfo,&len);
	MsgGsm_InputMsg(g_sendinfo,len);
}


//位置信息应答确认
void oa_JT_CheckAckInfo(unsigned short cmd)
{
	ackmodem_info *info=NULL;
	
	info = modem_GetAckModemInfo();
	if((info->ack==1)&&(info->cmd==cmd))//校验
	{
		modem_SetAckModem(2);
	}
}

//查询发送数据是否为协议内容
int oa_JT_query_filter_msg(unsigned char *msg, unsigned short msglen)
{
	unsigned short buflen=0;
	unsigned short i=0;
	unsigned char verify=0;
	
	if(msg[0] != 0x7E)
	{
		return -1;
	}
	memset_t(g_escapepack,0,sizeof(g_escapepack));
	//反报文转义
	for(i=0,buflen=0;i<msglen;i++,buflen++)
	{
		if(msg[i] == 0x7D)	//0x01 -> 0x7D    0x02->0x7E
		{
			i++;
			if(msg[i] == 0x01)
			{
				g_escapepack[buflen] = 0x7D;
			}
			else
			{
				g_escapepack[buflen] = 0x7E;
			}
		}
		else
		{
			g_escapepack[buflen] = msg[i];
		}
	}
	//校验
	verify = xor_check((unsigned char*)&g_escapepack[1],buflen-3)&0xFF;
	if(verify != g_escapepack[buflen-2])
	{
		return -1;
	}
	
	return 0;
}


//平台通用应答
int oa_trac_rev_general_ack_deal(Gbroad_msg msg)
{
	unsigned short cmdid;

	cmdid = uint8_to_uint16(&msg.msg[2]);
	switch(cmdid)
	{
		case MSG_GPRS_KEEPALIVE:
			g_modem_runtime.nCenterCount = 0;
			break;
		case MSG_GPRS_GPSMSG:
			oa_JT_CheckAckInfo(cmdid);
			if(msg.msg[4] == 0x04)	//取消所有报警
			{
				sys_clear_alarm();
			}
			if(msg.msg[4] == 0x00)	//成功/确认
			{
			}
			break;
		default:
			break;
	}

	return MSG_IS_OK;
}

//心跳间隔设置
int oa_trac_rev_para_set_keepalive(TLV tlv)
{
//	unsigned int i;
	
	if(tlv.len != 4)
	{
		return MSG_IS_ERR;
	}
//	i = uint8_to_uint32((unsigned char *)tlv.msg);//心跳间隔
//	g_runtime.g_gpsrun.nKeepAlive = i;
	
//	sys_saveRuning();	
	return MSG_IS_OK;
}


//设置APN
int oa_trac_rev_para_set_apn(TLV tlv)
{
	unsigned char *pch=NULL;

	if(tlv.len > APN_LEN)
	{
		return MSG_IS_ERR;
	}

	switch(tlv.type)
	{
		case MSG_GPRS_PARA_APNMAIN:			//0x0010	//主服务器 APN
			pch = g_sys.g_comm.sApn;
			break;
		case MSG_GPRS_PARA_USRMAIN:			//0x0011	//主服务器无线通信拨号用户名
			pch = g_sys.g_comm.sUser;
			break;
		case MSG_GPRS_PARA_PWDMAIN:			//0x0012	//主服务器无线通信拨号密码]
			pch = g_sys.g_comm.sPasswd;
			break;
		case MSG_GPRS_PARA_APNSUB:			//0x0014	//备份服务器APN
			break;
		case MSG_GPRS_PARA_USRSUB:			//0x0015	//备份服务器无线通信拨号用户名
			break;
		case MSG_GPRS_PARA_PWDSUB:			//0x0016	//备份服务器无线通信拨号密码
			break;
		default:
			pch = NULL;
			break;
	}

	if(pch)
	{
		memset_t(pch,0,APN_LEN);
		memcpy_t(pch,tlv.msg,tlv.len);

		sys_saveSys();
		modem_send_disconnect(0);
	}
	return MSG_IS_OK;
}

//设置ip或域名
int oa_trac_rev_para_set_addr(TLV tlv)
{
	unsigned char *pch = NULL;

	switch(tlv.type)
	{
		case MSG_GPRS_PARA_IPMAIN:			//0x0013	//主服务器地址,IP或域名
			sys_ClearUrl();
			sys_ClrIp();
			if(is_ip_address((char *)tlv.msg,tlv.len) == 0)
			{
				pch = g_sys.g_comm.sCenterIp;
			}
			else
			{
				pch = g_sys.g_comm.sURL;
			}
			break;
		case MSG_GPRS_PARA_IPSUB:			//0x0017	//备份服务器地址,IP或域名
			sys_ClearUrlBak();
			sys_ClrIpBak();
			if(is_ip_address((char *)tlv.msg,tlv.len) == 0)
			{
				pch = g_sys.g_comm.sCenterIpBak;
			}
			else
			{
				pch = g_sys.g_comm.sURLBak;
			}
			break;
		default:
			pch = NULL;
			break;
	}
	if(pch)
	{
		memcpy_t(pch,tlv.msg,tlv.len);
		
		sys_saveSys();
		modem_send_disconnect(0);
	}
	return MSG_IS_OK;
}


//设置端口
int oa_trac_rev_para_set_port(TLV tlv)
{
	unsigned int tmp;

	if(tlv.len != 4)
	{
		return MSG_IS_ERR;
	}
	
	tmp = uint8_to_uint32(tlv.msg);
	switch(tlv.type)
	{
		case MSG_GPRS_PARA_PORTTCP:			//0x0018	//服务器TCP端口
			if(tmp)
			{
				sys_SetLinkType(1);				//tcp
				sys_SetPort(tmp);
			}
			break;
		case MSG_GPRS_PARA_PORTUDP:			//0x0019	//服务器UDP端口
			if(tmp)
			{
				sys_SetLinkType(0);				//udp
				sys_SetPort(tmp);
			}
			break;
		default:
			break;
	}
	
	sys_saveSys();
	modem_send_disconnect(0);

	return MSG_IS_OK;
}

//位置汇报策略
int oa_trac_rev_para_set_report_watch(TLV tlv)
{
	unsigned int tmp;

	if(tlv.len != 4)
	{
		return MSG_IS_ERR;
	}
	
	tmp = uint8_to_uint32(tlv.msg);
	switch(tlv.type)
	{
		case MSG_GPRS_PARA_WATTYPE:			//0x0020	//位置汇报策略
			sys_SetWatchType(tmp+1);
			break;
		default:
			break;
	}

	sys_saveRuning();
	
	return MSG_IS_OK;
}

//休眠时汇报时间间隔
int oa_trac_rev_para_set_sleep_time(TLV tlv)
{
	unsigned int tmp;

	if(tlv.len != 4)
	{
		return MSG_IS_ERR;
	}
	
	tmp = uint8_to_uint32(tlv.msg);
	switch(tlv.type)
	{
		case MSG_GPRS_PARA_SLEEPTIMER:		//0x0027	//休眠时汇报时间间隔
			sys_SetAccStateOffInterval(tmp);
			sys_SetWatchType(1);
			break;
		default:
			break;
	}

	sys_saveRuning();
	
	return MSG_IS_OK;
}


//设置定时回传位置信息时间间隔
int oa_trac_rev_para_set_watInt(TLV tlv)
{
	unsigned int tmp;

	if(tlv.len != 4)
	{
		return MSG_IS_ERR;
	}

	tmp = uint8_to_uint32(tlv.msg);
	switch(tlv.type)
	{
		case MSG_GPRS_PARA_WATTIMEDEF:	//0x0029	//缺省时的汇报时间间隔(s)
			sys_SetAccStateOnInterval(tmp);
			sys_SetWatchType(1);
			break;
		case MSG_GPRS_PARA_WATDISDEF:	//0x002C	//缺省时的汇报距离间隔(m)
			sys_SetDistanceInterval(tmp);
			sys_SetWatchType(2);
			break;
		default:
			break;
	}
	
	sys_saveRuning();
	
	return MSG_IS_OK;
}

//设置拐点
int oa_trac_rev_para_set_redeem(TLV tlv)
{
	unsigned int tmp;

	if(tlv.len != 4)
	{
		return MSG_IS_ERR;
	}

	tmp = uint8_to_uint32(tlv.msg);
	switch(tlv.type)
	{
		case MSG_GPRS_PARA_REDEEM:	//0x0030//拐点补传角度
			sys_SetRedeem(tmp);
			break;
		default:
			break;
	}
	
	sys_saveRuning();
	
	return MSG_IS_OK;
}

//设置位移半径
int oa_trac_rev_para_set_move_radius(TLV tlv)
{
	unsigned short tmp;

	if(tlv.len != 2)
	{
		return MSG_IS_ERR;
	}

	tmp = uint8_to_uint16(tlv.msg);
	switch(tlv.type)
	{
		case MSG_GPRS_PARA_MOVE_RADIUS:	//0x0031//电子围栏半径（非法位移阈值）
			sys_SetMoveDistance(tmp);
			break;
		default:
			break;
	}
	
	sys_saveRuning();
	
	return MSG_IS_OK;
}


//自动接听
int oa_trac_rev_para_set_phoneLim(TLV tlv)
{
	unsigned int tmp;

	if(tlv.len != 4)
	{
		return MSG_IS_ERR;
	}
	
	tmp = uint8_to_uint32(tlv.msg);
	switch(tlv.type)
	{
		case MSG_GPRS_PARA_PHONERVTYPE:		//0x0045	//终端电话接听策略
			if(tmp == 0)
			{
				
			}
			break;
		default:
			break;
	}
	return MSG_IS_OK;
}

//监听号码
int oa_trac_rev_para_set_phoneLinsten(TLV tlv)
{
	if(tlv.len > 15)
	{
		return MSG_IS_ERR;
	}
	if(Is_Digit((char *)tlv.msg,tlv.len) == 0)
	{
		return MSG_IS_ERR;
	}
	switch(tlv.type)
	{
		case MSG_GPRS_PARA_PHONECEN:			//0x0040	//监控平台电话号码
			break;
		case MSG_GPRS_PARA_PHONEREBOOT:			//0x0041	//复位电话号码
			break;
		case MSG_GPRS_PARA_PHONERESET:			//0x0042	//恢复出厂设置电话号码
			break;
		case MSG_GPRS_PARA_PHONECENSMS:			//0x0043	//监控平台SMS 电话号码
			break;
		case MSG_GPRS_PARA_PHONESMSALM:			//0x0044	//接收终端SMS 文本报警号码
			break;
		case MSG_GPRS_PARA_PHONELISTENED:		//0x0048	//监听电话号码
			break;
		case MSG_GPRS_PARA_PHONERIGHT:			//0x0049	//监管平台特权号码
			break;
		default:
			break;
	}

	return MSG_IS_OK;
}

//报警参数设置
int oa_trac_rev_para_set_alarm(TLV tlv)
{
	unsigned int tmp;

	if(tlv.len != 4)
	{
		return MSG_IS_ERR;
	}

	tmp = uint8_to_uint32(tlv.msg);
	switch(tlv.type)
	{
		case MSG_GPRS_PARA_ALMENABEL:			//0x0050	//报警屏蔽字
			break;
		case MSG_GPRS_PARA_ALMENPIC:			//0x0052	//报警拍摄开关
			break;
		case MSG_GPRS_PARA_ALMENPICSAVE:		//0x0053	//报警拍摄存储标志
			break;
		case MSG_GPRS_PARA_ALMSPEEDOVER:		//0x0055	//最高速度(km/h)
			sys_SetOverSpeed(tmp);
			sys_state_offAlarm(ALARMST_SPEED);
			sys_state_offAlarm(ALARMST_WARN_SPEED);
			break;
		case MSG_GPRS_PARA_ALMSPEEDTIME:		//0x0056	//超速持续时间(s)
			sys_SetOverSpeedTime(tmp);
			break;
		case MSG_GPRS_PARA_ALMDRVTIMEONCE:		//0x0057	//连续驾驶时间门限(s)
			sys_SetDriveTimeOut(tmp);
			sys_state_offAlarm(ALARMST_DRIVETIMEOUT);
			sys_state_offAlarm(ALARMST_WARN_DRIVETIMEOUT); 
			break;
		case MSG_GPRS_PARA_ALMDRVRESET:			//0x0059//最小休息时间(s)
			sys_SetRestTimeOut(tmp);
			break;
		case MSG_GPRS_PARA_ALMSTOPTIME:			//0x005A//最长停车时间(s)
//			sys_SetStopTime(tmp);
			sys_state_offAlarm(ALARMST_STOP);
			break;
		default:
			break;
	}

	sys_saveRuning();
	
	return MSG_IS_OK;
}


//里程设置
int oa_trac_rev_para_set_carMillage(TLV tlv)
{
	unsigned int mile;
	
	if(tlv.len != 4)
	{
		return MSG_IS_ERR;
	}
	mile = uint8_to_uint32(tlv.msg)*100;
	
	sys_SetMileAge(mile);
	gps_mileage_save();

	return MSG_IS_OK;
}

//省域、市域iD
int oa_trac_rev_para_set_carID(TLV tlv)
{
	unsigned short tmp;

	if(tlv.len != 2)
	{
		return MSG_IS_ERR;
	}
	
	tmp = uint8_to_uint16(tlv.msg);
	switch(tlv.type)
	{
		case MSG_GPRS_PARA_PROVINCE:			//0x0081	//车辆所在的省域ID
			sys_SetProvinceId(tmp);
			break;
		case MSG_GPRS_PARA_CITY:				//0x0082	//车辆所在的市域ID
			sys_SetCityId(tmp);
			break;
		default:
			break;
	}

	sys_saveSys();
	modem_send_disconnect(0);

	return MSG_IS_OK;
}

//设置车牌号
int oa_trac_rev_para_set_carLicence(TLV tlv)
{
	if(tlv.len > SET_CAR_LICENCES_LEN)
	{
		return MSG_IS_ERR;
	}

	memset_t((char *)g_sys.g_login.sCarLicences,0,SET_CAR_LICENCES_LEN);
	memcpy_t((char *)g_sys.g_login.sCarLicences,(char*)tlv.msg,tlv.len);

	sys_saveSys();
	modem_send_disconnect(0);

	return MSG_IS_OK;
}

//设置车牌颜色
int oa_trac_rev_para_set_carColor(TLV tlv)
{
	if(tlv.len != 1)
	{
		return MSG_IS_ERR;
	}
	sys_SetCarColor(tlv.msg[0]);

	sys_saveSys();
	modem_send_disconnect(0);

	return MSG_IS_OK;
}

//通讯参数设置
int oa_trac_rev_para_set(Gbroad_msg msg)
{
	unsigned short ret;
	unsigned char *pch=NULL;
	unsigned short msglen;
	unsigned short i;
	TLV tlv;

	msglen = msg.msgattr&MSG_LENINVALID;
	if(msglen < 6)
	{
		return MSG_IS_ERR;
	}

	pch = &msg.msg[1];
	msglen -= 1;
	for(ret=0;ret<msg.msg[0] && msglen>5;ret++)
	{
		i = oa_get_TLV(pch,&tlv);
		if(i > msglen)
		{
			break;
		}
		pch += i;
		msglen -= i;

		switch(tlv.type)
		{
			case MSG_GPRS_PARA_KEEPALIVE:			//0x0001	//终端心跳(s)
				oa_trac_rev_para_set_keepalive(tlv);
				break;
			case MSG_GPRS_PARA_APNMAIN:				//0x0010	//主服务器 APN
			case MSG_GPRS_PARA_USRMAIN:				//0x0011	//主服务器无线通信拨号用户名
			case MSG_GPRS_PARA_PWDMAIN:				//0x0012	//主服务器无线通信拨号密码
			case MSG_GPRS_PARA_APNSUB:				//0x0014	//备份服务器APN
			case MSG_GPRS_PARA_USRSUB:				//0x0015	//备份服务器无线通信拨号用户名
			case MSG_GPRS_PARA_PWDSUB:				//0x0016	//备份服务器无线通信拨号密码
				oa_trac_rev_para_set_apn(tlv);
				break;
			case MSG_GPRS_PARA_IPMAIN:				//0x0013	//主服务器地址,IP或域名***
				oa_trac_rev_para_set_addr(tlv);
				break;
			case MSG_GPRS_PARA_PORTTCP:				//0x0018	//服务器TCP端口
			case MSG_GPRS_PARA_PORTUDP:				//0x0019	//服务器UDP端口
				oa_trac_rev_para_set_port(tlv);
				break;
			case MSG_GPRS_PARA_WATTYPE:				//0x0020	//位置汇报策略
				oa_trac_rev_para_set_report_watch(tlv);
				break;
			case MSG_GPRS_PARA_SLEEPTIMER:			//0x0027	//休眠时汇报时间间隔
				oa_trac_rev_para_set_sleep_time(tlv);
				break;
			case MSG_GPRS_PARA_WATTIMEDEF:			//0x0029	//缺省时的汇报时间间隔(s)
			case MSG_GPRS_PARA_WATDISDEF:			//0x002C	//缺省时的汇报距离间隔(m)
				oa_trac_rev_para_set_watInt(tlv);
				break;
			case MSG_GPRS_PARA_REDEEM:				//0x0030//拐点补传角度
				oa_trac_rev_para_set_redeem(tlv);
				break;
			case MSG_GPRS_PARA_MOVE_RADIUS:			//0x0031//电子围栏半径（非法位移阈值）
				oa_trac_rev_para_set_move_radius(tlv);
				break;
			case MSG_GPRS_PARA_PHONERVTYPE:			//0x0045	//终端电话接听策略
				oa_trac_rev_para_set_phoneLim(tlv);
				break;
			case MSG_GPRS_PARA_PHONELISTENED:   	//0x0048    //终端监听
				oa_trac_rev_para_set_phoneLinsten(tlv);
				break;
			case MSG_GPRS_PARA_ALMENABEL:			//0x0050	//报警屏蔽字
			case MSG_GPRS_PARA_ALMENPIC:			//0x0052	//报警拍摄开关
			case MSG_GPRS_PARA_ALMENPICSAVE:		//0x0053	//报警拍摄存储标志
			case MSG_GPRS_PARA_ALMSPEEDOVER:		//0x0055	//最高速度(km/h)
			case MSG_GPRS_PARA_ALMSPEEDTIME:		//0x0056	//超速持续时间(s)
			case MSG_GPRS_PARA_ALMDRVTIMEONCE:		//0x0057	//连续驾驶时间门限(s)
			case MSG_GPRS_PARA_ALMDRVRESET:			//0x0059 //最小休息时间(s)
			case MSG_GPRS_PARA_ALMSTOPTIME:			//0x005A //最长停车时间(s)
				oa_trac_rev_para_set_alarm(tlv);
				break;
			case MSG_GPRS_PARA_MILLAGE:				//0x0080	//车辆里程表读数(1/10km)
				oa_trac_rev_para_set_carMillage(tlv);
				break;
			case MSG_GPRS_PARA_PROVINCE:			//0x0081	//车辆所在的省域ID
			case MSG_GPRS_PARA_CITY:				//0x0082	//车辆所在的市域ID
				oa_trac_rev_para_set_carID(tlv);
				break;
			case MSG_GPRS_PARA_CARLICENCE:			//0x0083	//车牌
				oa_trac_rev_para_set_carLicence(tlv);
				break;
			case MSG_GPRS_PARA_CARLICENCECOL:		//0x0084	//车牌颜色
				oa_trac_rev_para_set_carColor(tlv);
				break;
			case 0x0085:
				
				break;
			case 0x0086:
				
				break;
			case 0x0087:
				
				break;
			case 0x0088:
				
				break;
			default:
				break;
		}

	}

	return MSG_IS_OK;
}

//终端参数查询应答
int oa_trac_rev_para_check(Gbroad_msg msg)
{
	unsigned char buf[256] = {0};

	unsigned short p = 0,len = 0;

	memset_t(g_sendinfo,0,sizeof(g_sendinfo));
	
	p += uint16_to_uint8(msg.msgsel, &buf[p]);
	buf[p++] = 19;

	p += uint32_to_uint8(MSG_GPRS_PARA_KEEPALIVE, &buf[p]);
	buf[p++] = 4;
	p += uint32_to_uint8(/*g_runtime.g_gpsrun.nKeepAlive*/0, &buf[p]);
	
	p += uint32_to_uint8(MSG_GPRS_PARA_APNMAIN, &buf[p]);
	buf[p++] = strlen_t((char *)g_sys.g_comm.sApn);
	p += sprintf((char *)&buf[p], (char *)g_sys.g_comm.sApn, strlen_t((char *)g_sys.g_comm.sApn));

	p += uint32_to_uint8(MSG_GPRS_PARA_USRMAIN, &buf[p]);
	buf[p++] = strlen_t((char *)g_sys.g_comm.sUser);
	p += sprintf((char *)&buf[p], (char *)g_sys.g_comm.sUser, strlen_t((char *)g_sys.g_comm.sUser));

	p += uint32_to_uint8(MSG_GPRS_PARA_PWDMAIN, &buf[p]);
	buf[p++] = strlen_t((char *)g_sys.g_comm.sPasswd);
	p += sprintf((char *)&buf[p], (char *)g_sys.g_comm.sPasswd, strlen_t((char *)g_sys.g_comm.sPasswd));
	
	p += uint32_to_uint8(MSG_GPRS_PARA_IPMAIN, &buf[p]);
	buf[p++] = strlen_t((char *)g_sys.g_comm.sCenterIp);
	p += sprintf((char *)&buf[p], (char *)g_sys.g_comm.sCenterIp, strlen_t((char *)g_sys.g_comm.sCenterIp));

	if(sys_GetLinkType())
	{
		p += uint32_to_uint8(MSG_GPRS_PARA_PORTTCP, &buf[p]);
	}
	else
	{
		p += uint32_to_uint8(MSG_GPRS_PARA_PORTUDP, &buf[p]);
	}
	buf[p++] = 4;
	p += uint32_to_uint8(sys_GetPort(), &buf[p]);

	p += uint32_to_uint8(MSG_GPRS_PARA_SLEEPTIMER, &buf[p]);
	buf[p++] = 4;
	p += uint32_to_uint8(sys_GetAccStateOffInterval(), &buf[p]);
	
	p += uint32_to_uint8(MSG_GPRS_PARA_WATTIMEDEF, &buf[p]);
	buf[p++] = 4;
	p += uint32_to_uint8(sys_GetAccStateOnInterval(), &buf[p]);

	p += uint32_to_uint8(MSG_GPRS_PARA_REDEEM, &buf[p]);
	buf[p++] = 4;
	p += uint32_to_uint8(sys_GetRedeem(), &buf[p]);

	p += uint32_to_uint8(MSG_GPRS_PARA_ALMSPEEDOVER, &buf[p]);
	buf[p++] = 4;
	p += uint32_to_uint8(sys_GetOverSpeed(), &buf[p]);

	p += uint32_to_uint8(MSG_GPRS_PARA_ALMSPEEDTIME, &buf[p]);
	buf[p++] = 4;
	p += uint32_to_uint8(sys_GetOverSpeedTime(), &buf[p]);

	p += uint32_to_uint8(MSG_GPRS_PARA_ALMDRVTIMEONCE, &buf[p]);
	buf[p++] = 4;
	p += uint32_to_uint8(sys_GetDriveTimeOut(), &buf[p]);

	p += uint32_to_uint8(MSG_GPRS_PARA_ALMDRVRESET, &buf[p]);
	buf[p++] = 4;
	p += uint32_to_uint8(sys_GetRestTimeOut(), &buf[p]);

	p += uint32_to_uint8(MSG_GPRS_PARA_ALMSTOPTIME, &buf[p]);
	buf[p++] = 4;
	p += uint32_to_uint8(/*sys_GetStopTime()*/0, &buf[p]);

	p += uint32_to_uint8(MSG_GPRS_PARA_MILLAGE, &buf[p]);
	buf[p++] = 4;
	p += uint32_to_uint8(sys_GetMileAge()/100, &buf[p]);

	p += uint32_to_uint8(MSG_GPRS_PARA_PROVINCE, &buf[p]);
	buf[p++] = 2;
	p += uint16_to_uint8(sys_GetProvinceId(), &buf[p]);

	p += uint32_to_uint8(MSG_GPRS_PARA_CITY, &buf[p]);
	buf[p++] = 2;
	p += uint16_to_uint8(sys_GetCityId(), &buf[p]);

	p += uint32_to_uint8(MSG_GPRS_PARA_CARLICENCE, &buf[p]);
	buf[p++] = strlen_t((char *)g_sys.g_login.sCarLicences);
	p += sprintf((char *)&buf[p], (char *)g_sys.g_login.sCarLicences, strlen_t((char *)g_sys.g_login.sCarLicences));
	
	p += uint32_to_uint8(MSG_GPRS_PARA_CARLICENCECOL, &buf[p]);
	buf[p++] = 1;
	buf[p++] = sys_GetCarColor();
	
	msg.msg = buf;
	msg.msgattr = p;
	
	len = oa_JT_locate(MSG_GPRS_CHECKACK,msg,g_sendinfo);
	modem_send_gprsData((char*)g_sendinfo,"OK",30,len);
	
	return MSG_IS_OK;
}


//远程升级
int oa_trac_rev_update_init(Gbroad_msg msg)
{
	unsigned char ret;
	char *pch = NULL;
	char *pend = NULL;
	char filename[32] = {0};
	int len,plen;

	len = msg.msgattr&MSG_LENINVALID;
	pch = (char *)&msg.msg[1];
	for(ret=0;ret<11 && len;ret++)
	{
		if(ret < 10)
		{
			pend = strchr_t(pch,';');
			if(pend == NULL)
			{
				break;
			}
			plen = pend - pch;
		}
		else
		{
			plen = len;
		}
		if(plen > len)
		{
			goto out;
		}
		len -= plen;

		switch(ret)
		{
			case 0:	//url address
				break;
			case 1:	//apn
				break;
			case 2:	//user
				break;
			case 3:	//pwd
				break;
			case 4:	//ip address
				if(plen > 15)
				{
					goto out;
				}
				if(plen)
				{
					update_SetIp((unsigned char *)pch,pend-pch);
				}
				break;
			case 5:	//TCP PORT
				break;
			case 6:	//UDP PORT
				#if 1//string & word
				if(plen > 5)
				{
					goto out;
				}
				sscanf(pch,"%d",&plen);
				if(plen > 0xFFFF)
				{
					goto out;
				}
				#else	//word
				if(plen != 2)
				{
					goto out;
				}
				plen = MC_Get_Uint16_Val(&pch[0]);
				#endif
				update_SetPort(plen);
				break;
			case 7:	//制造商ID
				break;
			case 8:	//硬件版本
				break;
			case 9:	//固件版本
				strncpy_t(filename, pch, pend-pch);
				update_SetFilename((unsigned char *)filename);
				break;
			case 10://连接到服务器时限
				break;
			default:
				break;
		}
		if(pend)
		{
			pch = pend + 1;
		}
	}

	update_GetLastState(modem_GetState());
	update_SetUpdateFlag(1);
	update_SetTFTPFlag(0);
	OS_SendMessage(xGsmQueueId, MSG_GSM_UPDATE_START, 0, 0);	

	return MSG_IS_OK;

out:
	return MSG_IS_ERR;
}

//终端控制
int oa_trac_rev_terminal_ctrl(Gbroad_msg msg)
{
	int status = MSG_IS_FAIL;
	unsigned char buf[64] = {0};

	switch(msg.msg[0])
	{
		case 0x01:	//远程升级
			status = oa_trac_rev_update_init(msg);
			break;
		case 0x02:	//指定连接服务器
			break;
		case 0x04:	//复位
			status = oa_JT_create_general_ack(msg,MSG_IS_OK,buf);		//通用回复
			modem_send_gprsData((char*)buf,"OK",30,status);

			OS_TaskDelayMs(3000);
			sys_Restart();
		case 0x05:	//恢复出厂
			status = oa_JT_create_general_ack(msg,MSG_IS_OK,buf);		//通用回复
			modem_send_gprsData((char*)buf,"OK",30,status);

			OS_TaskDelayMs(3000);
			sys_ResetFacitory();
			sys_Restart();
		case 0x03:	//关机
		case 0x06:	//关闭数据通信
		case 0x07:	//关闭所有无线通信
			break;
		case 0x64://Dout 3 断开油路
			gpio_set(DOUT3, 1);
			break;
		case 0x65://Dout 3 恢复油路
			gpio_set(DOUT3, 0);
			break;
		case 0x80://Dout1:0：恢复，1：断开
			gpio_set(DOUT1, (msg.msg[1]==0)?0:1);
			break;
		case 0x81://Dout2:0：恢复，1：断开
			gpio_set(DOUT2, (msg.msg[1]==0)?0:1);
			break;
		case 0x83://Dout4:0：恢复，1：断开
			
			break;
		default:
			break;
	}

	return status;

}

//终端参数查询应答
int oa_trac_rev_para_check_specail(Gbroad_msg msg)
{
	unsigned char buf[256] = {0};

	unsigned short p = 0,len = 0;
	unsigned char *pMsg = NULL;
	unsigned char i,no = 0,count = 0;
	unsigned int id = 0;

	memset_t(g_sendinfo,0,sizeof(g_sendinfo));
	
	count = msg.msg[0];
	p += uint16_to_uint8(msg.msgsel, &buf[p]);
	buf[p++] = count;

	pMsg = &msg.msg[1];
	for(i=0; i<count; i++)
	{
		id = uint8_to_uint32(pMsg);
		pMsg += 4;
		if(id == MSG_GPRS_PARA_KEEPALIVE)
		{
			p += uint32_to_uint8(MSG_GPRS_PARA_KEEPALIVE, &buf[p]);
			buf[p++] = 4;
			p += uint32_to_uint8(/*g_runtime.g_gpsrun.nKeepAlive*/0, &buf[p]);
		}
		else if(id == MSG_GPRS_PARA_APNMAIN)
		{
			p += uint32_to_uint8(MSG_GPRS_PARA_APNMAIN, &buf[p]);
			buf[p++] = strlen_t((char *)g_sys.g_comm.sApn);
			p += sprintf((char *)&buf[p], (char *)g_sys.g_comm.sApn, strlen_t((char *)g_sys.g_comm.sApn));
		}
		else if(id == MSG_GPRS_PARA_USRMAIN)
		{
			p += uint32_to_uint8(MSG_GPRS_PARA_USRMAIN, &buf[p]);
			buf[p++] = strlen_t((char *)g_sys.g_comm.sUser);
			p += sprintf((char *)&buf[p], (char *)g_sys.g_comm.sUser, strlen_t((char *)g_sys.g_comm.sUser));
		}
		else if(id == MSG_GPRS_PARA_PWDMAIN)
		{
			p += uint32_to_uint8(MSG_GPRS_PARA_PWDMAIN, &buf[p]);
			buf[p++] = strlen_t((char *)g_sys.g_comm.sPasswd);
			p += sprintf((char *)&buf[p], (char *)g_sys.g_comm.sPasswd, strlen_t((char *)g_sys.g_comm.sPasswd));
		}
		else if(id == MSG_GPRS_PARA_IPMAIN)
		{
			p += uint32_to_uint8(MSG_GPRS_PARA_IPMAIN, &buf[p]);
			buf[p++] = strlen_t((char *)g_sys.g_comm.sCenterIp);
			p += sprintf((char *)&buf[p], (char *)g_sys.g_comm.sCenterIp, strlen_t((char *)g_sys.g_comm.sCenterIp));
		}
		else if((id == MSG_GPRS_PARA_PORTTCP)||(id == MSG_GPRS_PARA_PORTUDP))
		{
			if(sys_GetLinkType())
			{
				p += uint32_to_uint8(MSG_GPRS_PARA_PORTTCP, &buf[p]);
			}
			else
			{
				p += uint32_to_uint8(MSG_GPRS_PARA_PORTUDP, &buf[p]);
			}
			buf[p++] = 4;
			p += uint32_to_uint8(sys_GetPort(), &buf[p]);
		}
		else if(id == MSG_GPRS_PARA_WATTYPE)
		{
			p += uint32_to_uint8(MSG_GPRS_PARA_WATTYPE, &buf[p]);
			buf[p++] = 4;
			p += uint32_to_uint8(sys_GetWatchType()-1, &buf[p]);
		}
		else if(id == MSG_GPRS_PARA_WATMODE)
		{
			p += uint32_to_uint8(MSG_GPRS_PARA_WATMODE, &buf[p]);
			buf[p++] = 4;
			p += uint32_to_uint8(0, &buf[p]);
		}
		else if(id == MSG_GPRS_PARA_SLEEPTIMER)
		{
			p += uint32_to_uint8(MSG_GPRS_PARA_SLEEPTIMER, &buf[p]);
			buf[p++] = 4;
			p += uint32_to_uint8(sys_GetAccStateOffInterval(), &buf[p]);
		}
		else if(id == MSG_GPRS_PARA_WATTIMEDEF)
		{
			p += uint32_to_uint8(MSG_GPRS_PARA_WATTIMEDEF, &buf[p]);
			buf[p++] = 4;
			p += uint32_to_uint8(sys_GetAccStateOnInterval(), &buf[p]);
		}
		else if(id == MSG_GPRS_PARA_REDEEM)
		{
			p += uint32_to_uint8(MSG_GPRS_PARA_REDEEM, &buf[p]);
			buf[p++] = 4;
			p += uint32_to_uint8(sys_GetRedeem(), &buf[p]);
		}
		else if(id == MSG_GPRS_PARA_ALMSPEEDOVER)
		{
			p += uint32_to_uint8(MSG_GPRS_PARA_ALMSPEEDOVER, &buf[p]);
			buf[p++] = 4;
			p += uint32_to_uint8(g_runtime.g_gpsrun.nOverSpeed, &buf[p]);
		}
		else if(id == MSG_GPRS_PARA_ALMSPEEDTIME)
		{
			p += uint32_to_uint8(MSG_GPRS_PARA_ALMSPEEDTIME, &buf[p]);
			buf[p++] = 4;
			p += uint32_to_uint8(g_runtime.g_gpsrun.nOverSpeedTime, &buf[p]);
		}
		else if(id == MSG_GPRS_PARA_ALMDRVTIMEONCE)
		{
			p += uint32_to_uint8(MSG_GPRS_PARA_ALMDRVTIMEONCE, &buf[p]);
			buf[p++] = 4;
			p += uint32_to_uint8(g_runtime.g_gpsrun.nDriveTimeOut, &buf[p]);
		}
		else if(id == MSG_GPRS_PARA_ALMDRVRESET)
		{
			p += uint32_to_uint8(MSG_GPRS_PARA_ALMDRVRESET, &buf[p]);
			buf[p++] = 4;
			p += uint32_to_uint8(g_runtime.g_gpsrun.nRestTimeOut, &buf[p]);
		}
		else if(id == MSG_GPRS_PARA_ALMSTOPTIME)
		{
			p += uint32_to_uint8(MSG_GPRS_PARA_ALMSTOPTIME, &buf[p]);
			buf[p++] = 4;
			p += uint32_to_uint8(/*g_runtime.g_gpsrun.nStopTime*/0, &buf[p]);
		}
		else if(id == MSG_GPRS_PARA_PROVINCE)
		{
			p += uint32_to_uint8(MSG_GPRS_PARA_PROVINCE, &buf[p]);
			buf[p++] = 2;
			p += uint16_to_uint8(sys_GetProvinceId(), &buf[p]);
		}
		else if(id == MSG_GPRS_PARA_CITY)
		{
			p += uint32_to_uint8(MSG_GPRS_PARA_CITY, &buf[p]);
			buf[p++] = 2;
			p += uint16_to_uint8(sys_GetCityId(), &buf[p]);
		}
		else if(id == MSG_GPRS_PARA_CARLICENCE)
		{
			p += uint32_to_uint8(MSG_GPRS_PARA_CARLICENCE, &buf[p]);
			buf[p++] = strlen_t((char *)g_sys.g_login.sCarLicences);
			p += sprintf((char *)&buf[p], (char *)g_sys.g_login.sCarLicences, strlen_t((char *)g_sys.g_login.sCarLicences));
		}
		else if(id == MSG_GPRS_PARA_CARLICENCECOL)
		{
			p += uint32_to_uint8(MSG_GPRS_PARA_CARLICENCECOL, &buf[p]);
			buf[p++] = 1;
			buf[p++] = sys_GetCarColor();
		}
		else
		{
			no ++;
		}
	}
	buf[2] = count-no;
	msg.msg = buf;
	msg.msgattr = p;
	
	len = oa_JT_locate(MSG_GPRS_CHECKACK,msg,g_sendinfo);
	modem_send_gprsData((char*)g_sendinfo,"OK",30,len);
	
	return MSG_IS_OK;
}


//终端属性查询应答
int oa_trac_rev_ter_check(Gbroad_msg msg)
{
	unsigned char buf[128] = {0};

	unsigned short p = 0,len = 0;
	
	memset_t(g_sendinfo,0,sizeof(g_sendinfo));
	
	p += uint16_to_uint8(0x0080, &buf[p]);				//终端类型

	memcpy_t(&buf[p],sys_GetManuId(),5);				//制造商ID
	p += 5;

	memcpy_t(&buf[p],sys_GetTermType(),20);				//终端型号20bytes
	p += 20;
	
	memcpy_t(&buf[p],sys_GetTermId(),7);				//终端ID
	p += 7;

	String2Byte(modem_GetCcid(),&buf[p],20);			//CCID
	p += 10;

	buf[p++] = 7;										//终端硬件版本长度
	memcpy_t(&buf[p],"HW-V1.2",7);						//终端硬件版本
	p += 7;

	buf[p++] = strlen_t(SMS_VER);						//终端固件版本长度
	memcpy_t(&buf[p],SMS_VER,strlen_t(SMS_VER));		//终端固件版本
	p += strlen_t(SMS_VER);

	buf[p++] = 0x03;									//GNSS 模块属性

	buf[p++] = 0x01;									//通信模块属性
	
	msg.msg = buf;
	msg.msgattr = p;
	
	len = oa_JT_locate(MSG_GPRS_CHECKACK_APPEND,msg,g_sendinfo);
	modem_send_gprsData((char*)g_sendinfo,"OK",30,len);
	
	return MSG_IS_OK;
}

//透传信息
void oa_trac_send_general_msg(unsigned short type,unsigned char *pin,unsigned short len) 
{
	unsigned char buf[512]={0};

	unsigned short msglen;
	Gbroad_msg msg = {0};

	if(modem_GetState() != GSM_ONLINE_STATE)
	{
		return;
	}
	
	memset_t(g_sendinfo,0,sizeof(g_sendinfo));
	
	msg.msg = buf;

	msg.msgattr = oa_JT_create_TLV_GeneralExt(type,pin,len,(unsigned char *)&msg.msg[0]);
	msglen = oa_JT_locate(MSG_GPRS_GENERAL_UPLOAD,msg,(unsigned char *)g_sendinfo);
	modem_send_gprsData((char*)g_sendinfo,"OK",30,msglen);
}

//发送调度信息
void oa_trac_send_msg_to_platform(unsigned short type,unsigned char *pin,unsigned short len) 
{
	unsigned char buf[320]={0};
	unsigned short msglen;
	Gbroad_msg msg = {0};
	
	memset_t(g_sendinfo,0,sizeof(g_sendinfo));
	
	msg.msg = buf;

	msg.msgattr = oa_JT_create_TLV_AWExt(type,pin,len,(unsigned char *)&msg.msg[0]);
	msglen = oa_JT_locate(0x6006/*MSG_GPRS_AWEXTUP*/,msg,(unsigned char *)g_sendinfo);
	modem_send_gprsData((char*)g_sendinfo,"OK",30,msglen);
}


//调度信息
int oa_trac_rev_post_scheduling(Gbroad_msg msg)
{
	int len;
	char *pdata=NULL;
	unsigned char datbuf[256]={0};
	unsigned short msglen;

	unsigned char sign,ret=0;
	
	len = msg.msgattr&MSG_LENINVALID;
	if(len > 400)
	{
		return (MSG_IS_NOT_SUPORT);
	}
	
	memset_t(g_sendinfo,0,sizeof(g_sendinfo));
	
	len -= 1;

	msglen = oa_JT_create_general_ack(msg,MSG_IS_OK&0x0F,g_sendinfo);
	modem_send_gprsData((char*)g_sendinfo,"OK",30,msglen);

	sign = msg.msg[0];
	if((0 == memcmp_t(&msg.msg[1],"*F10#",5)) && (len > 5))
	{
		msg.msg[len+1] = 0;
		pdata = (char*)&msg.msg[1];
		ret = handset_ConfigSet((unsigned char *)pdata, len, datbuf);

		if(strlen_t((char*)datbuf) > 0)
		{
			oa_trac_send_msg_to_platform(0x0003,datbuf,strlen_t((char*)datbuf));
		}
		if(ret == PC_DISCONNECT_TCPIP)
		{
			modem_send_disconnect(0);			// 断网
		}
		else if(ret == PC_RESTART)
		{
			OS_TaskDelayMs(3000);
			sys_Restart();                		// 重启
		}
	}
	else
	{
		if(sign&0x01)	//紧急
		{
		}
		if(sign&0x04)	//调度屏
		{
		}
		if(sign&0x08)	//TTS
		{
		}
		if(sign&0x10)	//LED
		{
		}
	}
	
	return MSG_IS_OK;
}

//监听
int oa_trac_rev_monitor(Gbroad_msg msg)
{
	unsigned char len;

	len = msg.msgattr&MSG_LENINVALID;
	if(len<3 ||len >20 || msg.msg[0] > 0x01)
	{
		return MSG_IS_ERR;
	}

	if(Is_Digit((char*)&msg.msg[1],len-1) == 0)
	{
		return MSG_IS_ERR;
	}

	msg.msg[len] = 0;

	if(msg.msg[0] == 0x00)	//普通电话
	{
		;
	}
	else //呼出监听
	{
		modem_CallPhone(&msg.msg[1], 0);
		OS_SendMessage(xGsmQueueId, MSG_GSM_CALL, MSG_GSM_RING, 0);
	}

	return MSG_IS_OK;
}

//点名
int oa_trac_rev_rollCall(Gbroad_msg msg)
{
	unsigned char datbuf[160]={0};
	unsigned short len;

	memset_t(g_sendinfo,0,sizeof(g_sendinfo));
	
	len = uint16_to_uint8(msg.msgsel,datbuf);
	len += oa_JT_serialize_gps(&datbuf[len],0x01);
	msg.msgattr = len;
	msg.msg = datbuf;
	
	len = oa_JT_locate(MSG_GPRS_ROLLCALLUP,msg,g_sendinfo);
	modem_send_gprsData((char*)g_sendinfo,"OK",30,len);
	
	return MSG_IS_OK;
}

//下行透传
int oa_trac_rev_general_aw_msg(Gbroad_msg msg)
{
	unsigned char *pch=NULL;
	TLV tlv;

	pch = &msg.msg[0];

	oa_get_ext_TLV(pch,&tlv);

	switch(tlv.type)
	{
		case MSG_GPRS_GENERAL_DOWNLOAD_LED:
			
			break;
		default:
			break;
	}

	return MSG_IS_OK;
}

//星安扩展协议应用
unsigned short oa_JT_create_TLV_AWExt(unsigned short type,unsigned char *pin,unsigned short len,unsigned char *pout)
{
	unsigned short p;

	switch(type)
	{
		case 0x0003://终端文本信息上传
			p = uint16_to_uint8(type,&pout[0]);
			p += uint16_to_uint8(len,&pout[p]);
			if(pin && len)
			{
				memcpy_t((char*)&pout[p],pin,len);
				p += len;
			}
			break;
		case 0x0006://终端版本应答
			p = uint16_to_uint8(type,&pout[0]);
			p += uint16_to_uint8(len,&pout[p]);
			memcpy_t((char*)&pout[p],pin,len);
			p += len;
			break;
		case 0x0008://油路状态上报
			p = uint16_to_uint8(type,&pout[0]);
			p += uint16_to_uint8(len,&pout[p]);
			pout[p++] = 0;
			p += uint32_to_uint8(0,&pout[p]);
			break;
		default:
			break;
	}

	return p;
}

//JT808内部扩展协议
int oa_trac_rev_expend_aw_msg(Gbroad_msg msg)
{
	unsigned char *pch=NULL;
	unsigned char buf[128]={0};
	unsigned char version[64]={0};
	TLV tlv;

	pch = &msg.msg[0];

	oa_get_ext_TLV(pch,&tlv);

	switch(tlv.type)
	{
		case MSG_GPRS_CHECK_VER:
			app_GetVersion(version);
			
			buf[0] = 0x00;
			buf[1] = 0x01;
			buf[2] = 0x00;
			buf[3] = strlen_t((char *)version);
			memcpy_t((buf+4), version, strlen_t((char *)version));
			oa_trac_send_msg_to_platform(0x0006,buf,(strlen_t((char *)version)+4));
			break;
		case MSG_GPRS_OIL_VOLUME:
		case MSG_GPRS_OIL_TEMP:
		case MSG_GPRS_OIL_DAMP:
			
			break;
		default:
			break;
	}

	return MSG_IS_OK;
}

//版本号
int oa_trac_rev_expend_aw_version(void)
{
	unsigned char buf[128]={0};
	unsigned char version[64]={0};

	app_GetVersion(version);
	
	buf[0] = 0x00;
	buf[1] = 0x01;
	buf[2] = 0x00;
	buf[3] = strlen_t((char *)version);
	memcpy_t((buf+4), version, strlen_t((char *)version));
	oa_trac_send_msg_to_platform(0x0006,buf,(strlen_t((char *)version)+4));

	return MSG_IS_OK;
} 

//接收处理
void oa_trac_recv_msg_handle(Gbroad_msg msg)
{
	int status = 0;
	unsigned short msglen;
	
	memset_t(g_sendinfo,0,sizeof(g_sendinfo));
	
	status = MSG_IS_OK;
	switch(msg.cmdid)
	{
		case MSG_GPRS_CENGENACK:			//0x8001	//平台通用应答
			oa_trac_rev_general_ack_deal(msg);
			break;
		case MSG_GPRS_PARA_SET:				//0x8103	//设置终端参数
			status = oa_trac_rev_para_set(msg);
			break;
		case MSG_GPRS_PARA_CHECK:			//0x8104	//查询终端参数
			status = oa_trac_rev_para_check(msg);
			break;
		case MSG_GPRS_TERCONTRL:			//0x8105	//终端控制
			status = oa_trac_rev_terminal_ctrl(msg);
			break;
		case MSG_GPRS_TER_CHECK:			//0x8107	//查询终端属性
			status = oa_trac_rev_ter_check(msg);
			break;
		case MSG_GPRS_ROLLCALL:				//0x8201	//位置信息查询(点名)
			status = oa_trac_rev_rollCall(msg);
			break;
		case MSG_GPRS_SCHEDDOWN:			//0x8300	//文本信息下发
			status = oa_trac_rev_post_scheduling(msg);
			break;
		case MSG_GPRS_PHONELISTEN:			//0x8400	//电话回拨
			status = oa_trac_rev_monitor(msg);
			break;
		case MSG_GPRS_CARCONTROL:			//0x8500	//车辆控制
			break;
		case MSG_GPRS_GENERAL_DOWNLOAD:		//0x8900//数据下行透传
			status = oa_trac_rev_general_aw_msg(msg);
			break;
		case MSG_GPRS_AWVERSION:			//0x8702//星安扩展应用
			status = oa_trac_rev_expend_aw_version();
			break;
		case MSG_GPRS_AWEXTDOWN:			//0x8FFE//星安扩展应用
			status = oa_trac_rev_expend_aw_msg(msg);
			break;
		default:
			status = MSG_IS_NOT_SUPORT;
			break;
	}

	//通用应答
	switch(msg.cmdid)
	{
		case MSG_GPRS_PARA_SET:				//0x8103	//设置终端参数
		case MSG_GPRS_TERCONTRL:			//0x8105	//终端控制
		case MSG_GPRS_PHONELISTEN:			//0x8400	//电话回拨
		case MSG_GPRS_CARCONTROL:			//0x8500	//车辆控制
		case MSG_GPRS_GENERAL_DOWNLOAD:		//0x8900	//数据下行透传
		case MSG_GPRS_AWVERSION:			//0x8702//星安扩展应用
		case MSG_GPRS_AWEXTDOWN:			//0x8FFE	//星安扩展应用
			msglen = oa_JT_create_general_ack(msg,status&0x0F,g_sendinfo);		//通用回复
			modem_send_gprsData((char*)g_sendinfo,"OK",30,msglen);
			break;
		default:
			break;
	}
}

//接收处理
void oa_recv_Handle(char* buf,unsigned short buflen)
{
	int msglen = 0;

	Gbroad_msg msg_rcv = {0};
	
	if((buf==NULL) || (buflen==0))
	{
	   	return;
	}

	if(modem_GetState() == GSM_FOTA_STATE) //远程升级中消息的处理
	{
		update_handle(0,(unsigned char *)buf,buflen);  
		return;
	}

	LOG("[gsm] ip1 rcv:");
	LOGX((unsigned char *)buf,buflen);
	
	memset_t(g_sendinfo,0,sizeof(g_sendinfo));
	
	msg_rcv.msg = g_sendinfo;
	while(buflen >= 14)
	{
		msglen = oa_JT_deal_msg(buf,buflen,&msg_rcv);
		if(msglen == 0)
		{
			break;
		}
		if(msglen & 0x80000)
		{
			msglen &= 0xFFFF;
			if(buflen >= msglen)
			{
				buf += msglen;
				buflen -= msglen;
			}
			else
			{
				break;
			}
			continue;
		}

		if(buflen >= msglen)
		{
			buf += msglen;
			buflen -= msglen;
		}
		else
		{
			buflen = 0;
		}
		oa_trac_recv_msg_handle(msg_rcv);	
	}

}

