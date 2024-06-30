#include "includes.h"

static unsigned short g_up_seq_num=0;					//������ˮ��
static unsigned short g_register_seq=0;				//������ˮ��
static unsigned char  sLoginsel[SET_LOGIN_SEL_LEN+1];	//�ն˼�Ȩ��
static unsigned char g_register_sign=0;				//ע���־

static unsigned char g_sendpack[DATALEN+1];				//
static unsigned char g_sendinfo[512];				//
static unsigned char g_escapepack[DATALEN+1];				//
//����Ϣ��
int oa_JT_general_msg(Gbroad_msg msg,unsigned char* pdst)
{
	unsigned short p,j,len;

	memset_t(g_escapepack,0,DATALEN+1);
	
	len = (msg.msgattr&MSG_LENINVALID);
	msg.msgattr &= ~MSG_ENCODE;//������

	p = 0;
	//��ϢID
	p += uint16_to_uint8(msg.cmdid,&g_escapepack[p]);
	//��Ϣ����
	p += uint16_to_uint8(msg.msgattr,&g_escapepack[p]);
	//�ն��ֻ���
	memcpy_t((char *)&g_escapepack[p],sys_GetId1(),FAKE_IP_LE);
	p += FAKE_IP_LE;
	//��ˮ��
	p += uint16_to_uint8(msg.msgsel,&g_escapepack[p]);
	if(msg.msgattr&MSG_MULTY)
	{
		//��Ϣ����װ��
		p += uint16_to_uint8(msg.msgpack.total,&g_escapepack[p]);
		p += uint16_to_uint8(msg.msgpack.cur_bag,&g_escapepack[p]);
	}
	//��Ϣ��
	if(msg.msg && len)
	{
		memcpy_t(&g_escapepack[p],msg.msg,len);
		p += len;
	}

	//У��
	g_escapepack[p] = xor_check(&g_escapepack[0],p);
	p ++;

	//����ת��
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

	pdst[0] = GBROAD_MSG_SIGN;		//֡ͷ
	pdst[len++] = GBROAD_MSG_SIGN;	//֡β
	p = len;
	
	return p;
}


//��ˮ�Ź���
unsigned short oa_change_UpSel(unsigned short sel)
{
	unsigned short sel_cur;

	sel_cur = g_up_seq_num;
	g_up_seq_num += sel;

	return sel_cur;
}

//��ȡTLV
int oa_get_TLV(unsigned char *msg,TLV *tlv)
{
	tlv->type = uint8_to_uint32(&msg[0]);
	tlv->len = msg[4];
	tlv->msg = (unsigned char*)&msg[5];

	return (5+tlv->len);
}

//��ȡext TLV
void oa_get_ext_TLV(unsigned char *msg,TLV *tlv)
{
	tlv->type = uint8_to_uint16(&msg[0]);
	tlv->len = uint8_to_uint16(&msg[2]);
	tlv->msg = (unsigned char*)&msg[4];
}

//type:�ָߵ�16λʹ��,��16λ��ʾcmd id
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


//ע��
int oa_JT_create_register_msg(unsigned char *pout)
{
	Gbroad_msg msg = {0};
	unsigned char buf[128]={0};

	int p;

	p = 0;
	p += uint16_to_uint8(sys_GetProvinceId(),&buf[p]);	//ʡID
	p += uint16_to_uint8(sys_GetCityId(),&buf[p]);		//��ID
	memcpy_t(&buf[p],sys_GetManuId(),5);				//������ID
	p += 5;

	memcpy_t(&buf[p],sys_GetTermType(),20);				//�ն��ͺ�20bytes
	p += 20;

	memcpy_t(&buf[p],sys_GetTermId(),7);				//�ն�ID
	p += 7;
	buf[p++] = 1;
	
	memcpy_t(&buf[p],g_sys.g_login.sImeiDevID,8);			//���� VIN
	p += 8;

	msg.msgsel = oa_change_UpSel(1);
	g_register_seq = msg.msgsel;
	msg.msgattr = p;
	msg.msg = buf;
	p = oa_JT_locate(MSG_GPRS_TERREG|MSG_UP_ACK,msg,pout);

	return p;
}

//��¼
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

//ע��
int oa_JT_create_logout_msg(unsigned char *pout)
{
	Gbroad_msg msg = {0};
	unsigned short ret;	

	ret = oa_JT_locate(MSG_GPRS_TERLOGOUT,msg,pout);

	return ret;
}


//����������Ϣ
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

//͸��Э��������
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

//gps�������
int oa_JT_serialize_gps(unsigned char *pout,unsigned char sign)
{
	gpsinfo pfull;
	gpsinfo *lastgpsdata=sys_GetLastGpsData(); 
	double fTmp = 0.0;
	unsigned int tmp=0;
	int p=0;
	
	if(sys_state_get_gpsloc())
	{
		//ʵʱ����λ����Ϣ
		if(gps_update_location_condition() == TRUE)
		{
			memcpy_t((char *)&pfull,(char *)gps_GetGpsInfo(),sizeof(gpsinfo));
		}
		//���һ���λ��
		else
		{
			memcpy_t((char *)&pfull,(char *)lastgpsdata,sizeof(gpsinfo));
			strcpy_t(pfull.speed_str,"00.00");
			gps_GetGpsTime(&pfull.date);
		}
		//�ϴ�ʱ��Ϊgpsʱ��
		LOG("[msg] gps time:%02d-%02d-%02d  %02d:%02d:%02d\r\n",pfull.date.year,pfull.date.month,pfull.date.day,pfull.date.hour,pfull.date.minute,pfull.date.second);
	}
	else
	{
		memcpy_t((char *)&pfull,(char *)lastgpsdata,sizeof(gpsinfo));
		strcpy_t(pfull.speed_str,"00.00");
		
		//�ϴ�ʱ��Ϊrtcʱ��
		memcpy_t(&pfull.date,sys_GetSysRTC(),sizeof(TIME));
		LOG("[msg] rtc time:%02d-%02d-%02d  %02d:%02d:%02d\r\n",pfull.date.year,pfull.date.month,pfull.date.day,pfull.date.hour,pfull.date.minute,pfull.date.second);
	}

	//������־,״̬��־
	uint32_to_uint8(sys_state_get_alarm(),(unsigned char*)&pout[0]);
	uint32_to_uint8(sys_state_get_car(),(unsigned char*)&pout[4]);
	p = 8;

	//γ��(4)
	parse_lat_or_lon_to_double(pfull.latitude_str,&fTmp);
	tmp = (unsigned int)(fTmp*1000000);
	p += uint32_to_uint8(tmp,&pout[p]);

	//����(4)
	parse_lat_or_lon_to_double(pfull.longitude_str,&fTmp);
	tmp = (unsigned int)(fTmp*1000000);
	p += uint32_to_uint8(tmp,&pout[p]);
	
	//�߶�(2)
	tmp = gps_GetGpsAltitude();
	p += uint16_to_uint8(tmp,&pout[p]);

	//�ٶ�(2)
	tmp = (unsigned int)(myatof(pfull.speed_str)*1.852f*10);//km/h
	p += uint16_to_uint8(tmp,&pout[p]);

	//����(2)
	tmp = (unsigned int)(myatoi(pfull.head_str));
	p += uint16_to_uint8(tmp,&pout[p]);
	
	//������ʱ����
	pout[p++] = HexToCompactBCD(pfull.date.year-2000);
	pout[p++] = HexToCompactBCD(pfull.date.month);
	pout[p++] = HexToCompactBCD(pfull.date.day);
	pout[p++] = HexToCompactBCD(pfull.date.hour);
	pout[p++] = HexToCompactBCD(pfull.date.minute);
	pout[p++] = HexToCompactBCD(pfull.date.second);

	if(sign)
	{
		//������Ϣ
		//���,100m
		p += oa_JT_create_append_Ext(GPRS_APPEND_Millage,4,sys_GetMileAge()/100,&pout[p]);	

		//CSQ
		p += oa_JT_create_append_Ext(GPRS_APPEND_CSQ,1,modem_GetCsq(),&pout[p]);
		
		//����
		p += oa_JT_create_append_Ext(GPRS_APPEND_GPS_STA,1,gps_GetGpsStarNum(),&pout[p]);

		//ģ����
//		if(serialoil_enable_urtfun(ALL) == ENABLE)
//		{
//			p += oa_JT_create_append_Ext(GPRS_APPEND_AD,4,serialoil_get_percent(1),&pout[p]);
		
			pout[p++] = GPRS_APPEND_AD;
			pout[p++] = 4;
			p += uint16_to_uint8(serialoil_get_percent(1),&pout[p]);
			p += uint16_to_uint8(serialoil_get_percent(2),&pout[p]);
	
//		}
			
		pout[p++] = 0xef;//����
		pout[p++] = 0x04;
		pout[p++] = 0x06;//id
		pout[p++] = 0x02;//id
		p += uint16_to_uint8(xdevinfo.tempbt+4000,&pout[p]);
		
		pout[p++] = 0xf0;//ain1 ��ѹ
		pout[p++] = 0x02;
		p += uint16_to_uint8(sDevPowerVolt[4]*10,&pout[p]);
		
		pout[p++] = 0xf1;//ain2 ��ѹ
		pout[p++] = 0x02;
		p += uint16_to_uint8(sDevPowerVolt[3]*10,&pout[p]);
		
		pout[p++] = 0xf2;//ain3 ��ѹ
		pout[p++] = 0x02;
		p += uint16_to_uint8(sDevPowerVolt[0]*10,&pout[p]);
	}

	return p;
}


//�Խ��յ�ƽ̨�Ľ�������ת��,У��,����,��ѹ
int oa_JT_deal_msg(char *msg,int len,Gbroad_msg *pout)
{
	//�ݰ���Ϣ��Ϊ�ְ��󵥶�����ѹ������
	int msglen;
	unsigned short ret;
	unsigned int plen = 0;

	if(msg[0] != GBROAD_MSG_SIGN)
	{
		return 0;
	}
	//ȡ����һ�����ݰ�
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

	//����ת��
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

	//У��
	ret = xor_check((unsigned char*)&msg[1],msglen-3)&0xFF;
	if(ret != msg[msglen-2])
	{
		goto end;
	}
	//�Ƚ��ն�ID
	if(memcmp_t(sys_GetId1(),&msg[5],FAKE_IP_LE))
	{
		goto end;
	}

	ret = 1;
	//��ϢID
	pout->cmdid = uint8_to_uint16((unsigned char*)&msg[ret]);
	ret += 2;
	//��Ϣ����
	pout->msgattr = uint8_to_uint16((unsigned char*)&msg[ret]);
	ret += 2;
	if((pout->msgattr&MSG_LENINVALID)+19 != msglen && (pout->msgattr&MSG_LENINVALID)+15 != msglen)
	{
		goto end;
	}
	//�ն��ֻ���msg[5]~msg[10]
	ret += 6;
	//��ˮ��
	pout->msgsel = uint8_to_uint16((unsigned char*)&msg[ret]);
	ret += 2;
	if(pout->msgattr&MSG_MULTY)
	{
		//��Ϣ����װ��
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

//ͨ��Ӧ��
int oa_JT_create_general_ack(Gbroad_msg msg,unsigned char status,unsigned char *pout)
{
	unsigned char buf[5]={0};

	uint16_to_uint8(msg.msgsel,&buf[0]);
	uint16_to_uint8(msg.cmdid,&buf[2]);
	buf[4] = status;

	msg.msg = buf;
	msg.msgattr = 5;	//���ְ�,������

	return oa_JT_locate(MSG_GPRS_TERGENACK,msg,(unsigned char *)pout);
}


//��½���
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
		if(status & 0x80000)	//�ն�ID����
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
			case MSG_GPRS_CENGENACK:		//0x8001	//ƽ̨ͨ��Ӧ��
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
							LOG("[ip1] �ն˼�Ȩ�ɹ�\r\n");	
						}
						OS_Timer_Stop(GSM_TIMER_ID);
						OS_SendMessage(xGsmQueueId, MSG_GSM_CENTER, 0, 0);
					}
					else
					{
						g_register_sign = 0;		//��Ч
						modem_send_disconnect(0);
					}
				}
				break;
			case MSG_GPRS_CENREGACK:		//0x8100	//�ն�ע��
				if((msg_rcv.msgattr&MSG_LENINVALID) > SET_LOGIN_SEL_LEN+3)
				{
					msglen = oa_JT_create_general_ack(msg_rcv,MSG_IS_NOT_SUPORT&0x0F,sendbuf);		//ͨ�ûظ�
					modem_send_gprsData((char*)sendbuf,"OK",30,msglen);
					break;
				}
				if(msg_rcv.msg[2] == 2 || msg_rcv.msg[2] == 4)
				{
					//ƽ̨δ�Ǽ�
					LOG("[ip1] �޸ó���/�ն���Ϣ\r\n");
					break;
				}
				if(msg_rcv.msg[2] == 1 || msg_rcv.msg[2] == 3)
				{
					LOG("[ip1] ����/�ն��ѱ�ע��\r\n");
					g_register_sign = 0;		//��Ч
					ret = 0;
					msglen = oa_JT_create_logout_msg(sendbuf);
					modem_send_gprsData((char*)sendbuf,"OK",30,msglen);
					break;
				}
				LOG("[ip1] ����/�ն�ע��ɹ�\r\n");
				memset_t((char*)sLoginsel,0,SET_LOGIN_SEL_LEN+1);
//				memcpy_t((char*)sLoginsel,(char*)&msg_rcv.msg[3],(msg_rcv.msgattr&MSG_LENINVALID)-3);
				memcpy_t((char*)sLoginsel,(char*)&msg_rcv.msg[4],(msg_rcv.msgattr&MSG_LENINVALID)-4);
				g_register_sign = 1;			//��Ч
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


//��λ����Ϣ��
void aw_serialize_location(unsigned char *buf, unsigned short *buflen)
{
	Gbroad_msg msg = {0};
	unsigned char datbuf[256] = {0};
	
	msg.msgattr = oa_JT_serialize_gps(datbuf,0x01);
	msg.msgattr &= ~MSG_MULTY;
	msg.msg = datbuf;
	*buflen = oa_JT_locate(MSG_GPRS_GPSMSG,msg,buf);
}

//����
bool aw_udp_hangup(void)
{
	unsigned char sendbuf[128]={0};
	unsigned short len = 0;

	if(g_register_sign == 0)//ע��
	{
		len = oa_JT_create_register_msg(sendbuf);
	}
	else//��¼
	{
		len = oa_JT_create_login_msg(sLoginsel,sendbuf);
	}
	modem_send_gprsData((char*)sendbuf,"OK",30,len);

	return TRUE;
}

//����
bool aw_udp_keepalive(void)
{
	unsigned char sendbuf[32]={0};
	unsigned short len = 0;
	Gbroad_msg msg = {0};

	len = oa_JT_locate(MSG_GPRS_KEEPALIVE,msg,(unsigned char *)sendbuf);
	modem_send_gprsData((char*)sendbuf,"OK",50,len);

	return TRUE;
}


//ä��
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

//λ����Ϣ
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

//gps�������
int oa_JT_serialize_gpsrfid(unsigned char *pout,unsigned char sign,unsigned char type)
{
	gpsinfo pfull;
	gpsinfo *lastgpsdata=sys_GetLastGpsData(); 
	double fTmp = 0.0;
	unsigned int tmp=0;
	int p=0;
	
	if(sys_state_get_gpsloc())
	{
		//ʵʱ����λ����Ϣ
		if(gps_update_location_condition() == TRUE)
		{
			memcpy_t((char *)&pfull,(char *)gps_GetGpsInfo(),sizeof(gpsinfo));
		}
		//���һ���λ��
		else
		{
			memcpy_t((char *)&pfull,(char *)lastgpsdata,sizeof(gpsinfo));
			strcpy_t(pfull.speed_str,"00.00");
			gps_GetGpsTime(&pfull.date);
		}
		//�ϴ�ʱ��Ϊgpsʱ��
		LOG("[msg] gps time:%02d-%02d-%02d  %02d:%02d:%02d\r\n",pfull.date.year,pfull.date.month,pfull.date.day,pfull.date.hour,pfull.date.minute,pfull.date.second);
	}
	else
	{
		memcpy_t((char *)&pfull,(char *)lastgpsdata,sizeof(gpsinfo));
		strcpy_t(pfull.speed_str,"00.00");
		
		//�ϴ�ʱ��Ϊrtcʱ��
		memcpy_t(&pfull.date,sys_GetSysRTC(),sizeof(TIME));
		LOG("[msg] rtc time:%02d-%02d-%02d  %02d:%02d:%02d\r\n",pfull.date.year,pfull.date.month,pfull.date.day,pfull.date.hour,pfull.date.minute,pfull.date.second);
	}

	pout[p++] = type;
	
	//������־,״̬��־
	p += uint32_to_uint8(sys_state_get_alarm(),&pout[p]);

	p += uint32_to_uint8(sys_state_get_car(),&pout[p]);


	//γ��(4)
	parse_lat_or_lon_to_double(pfull.latitude_str,&fTmp);
	tmp = (unsigned int)(fTmp*1000000);
	p += uint32_to_uint8(tmp,&pout[p]);

	//����(4)
	parse_lat_or_lon_to_double(pfull.longitude_str,&fTmp);
	tmp = (unsigned int)(fTmp*1000000);
	p += uint32_to_uint8(tmp,&pout[p]);
	
	//�߶�(2)
	tmp = gps_GetGpsAltitude();
	p += uint16_to_uint8(tmp,&pout[p]);

	//�ٶ�(2)
	tmp = (unsigned int)(myatof(pfull.speed_str)*1.852f*10);//km/h
	p += uint16_to_uint8(tmp,&pout[p]);

	//����(2)
	tmp = (unsigned int)(myatoi(pfull.head_str));
	p += uint16_to_uint8(tmp,&pout[p]);
	
	//������ʱ����
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


//��λ����Ϣ��
void aw_serialize_rfid(unsigned char *buf, unsigned short *buflen,unsigned char type)
{
	Gbroad_msg msg = {0};
	unsigned char datbuf[256] = {0};
	
	msg.msgattr = oa_JT_serialize_gpsrfid(datbuf,0x01,type);
	msg.msgattr &= ~MSG_MULTY;
	msg.msg = datbuf;
	*buflen = oa_JT_locate(0x0501,msg,buf);
}

//RFID ��Ϣ�ϱ�
void aw_send_rfid_msg(unsigned char type)
{
	unsigned short len=0;

	memset_t(g_sendinfo,0,sizeof(g_sendinfo));
	aw_serialize_rfid(g_sendinfo,&len,type);
	MsgGsm_InputMsg(g_sendinfo,len);
}

//������Ϣ
void aw_send_alarm_msg(void)
{
	unsigned short len=0;
	memset_t(g_sendinfo,0,sizeof(g_sendinfo));
	aw_serialize_location(g_sendinfo,&len);
	MsgGsm_InputMsg(g_sendinfo,len);
}


//λ����ϢӦ��ȷ��
void oa_JT_CheckAckInfo(unsigned short cmd)
{
	ackmodem_info *info=NULL;
	
	info = modem_GetAckModemInfo();
	if((info->ack==1)&&(info->cmd==cmd))//У��
	{
		modem_SetAckModem(2);
	}
}

//��ѯ���������Ƿ�ΪЭ������
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
	//������ת��
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
	//У��
	verify = xor_check((unsigned char*)&g_escapepack[1],buflen-3)&0xFF;
	if(verify != g_escapepack[buflen-2])
	{
		return -1;
	}
	
	return 0;
}


//ƽ̨ͨ��Ӧ��
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
			if(msg.msg[4] == 0x04)	//ȡ�����б���
			{
				sys_clear_alarm();
			}
			if(msg.msg[4] == 0x00)	//�ɹ�/ȷ��
			{
			}
			break;
		default:
			break;
	}

	return MSG_IS_OK;
}

//�����������
int oa_trac_rev_para_set_keepalive(TLV tlv)
{
//	unsigned int i;
	
	if(tlv.len != 4)
	{
		return MSG_IS_ERR;
	}
//	i = uint8_to_uint32((unsigned char *)tlv.msg);//�������
//	g_runtime.g_gpsrun.nKeepAlive = i;
	
//	sys_saveRuning();	
	return MSG_IS_OK;
}


//����APN
int oa_trac_rev_para_set_apn(TLV tlv)
{
	unsigned char *pch=NULL;

	if(tlv.len > APN_LEN)
	{
		return MSG_IS_ERR;
	}

	switch(tlv.type)
	{
		case MSG_GPRS_PARA_APNMAIN:			//0x0010	//�������� APN
			pch = g_sys.g_comm.sApn;
			break;
		case MSG_GPRS_PARA_USRMAIN:			//0x0011	//������������ͨ�Ų����û���
			pch = g_sys.g_comm.sUser;
			break;
		case MSG_GPRS_PARA_PWDMAIN:			//0x0012	//������������ͨ�Ų�������]
			pch = g_sys.g_comm.sPasswd;
			break;
		case MSG_GPRS_PARA_APNSUB:			//0x0014	//���ݷ�����APN
			break;
		case MSG_GPRS_PARA_USRSUB:			//0x0015	//���ݷ���������ͨ�Ų����û���
			break;
		case MSG_GPRS_PARA_PWDSUB:			//0x0016	//���ݷ���������ͨ�Ų�������
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

//����ip������
int oa_trac_rev_para_set_addr(TLV tlv)
{
	unsigned char *pch = NULL;

	switch(tlv.type)
	{
		case MSG_GPRS_PARA_IPMAIN:			//0x0013	//����������ַ,IP������
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
		case MSG_GPRS_PARA_IPSUB:			//0x0017	//���ݷ�������ַ,IP������
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


//���ö˿�
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
		case MSG_GPRS_PARA_PORTTCP:			//0x0018	//������TCP�˿�
			if(tmp)
			{
				sys_SetLinkType(1);				//tcp
				sys_SetPort(tmp);
			}
			break;
		case MSG_GPRS_PARA_PORTUDP:			//0x0019	//������UDP�˿�
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

//λ�û㱨����
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
		case MSG_GPRS_PARA_WATTYPE:			//0x0020	//λ�û㱨����
			sys_SetWatchType(tmp+1);
			break;
		default:
			break;
	}

	sys_saveRuning();
	
	return MSG_IS_OK;
}

//����ʱ�㱨ʱ����
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
		case MSG_GPRS_PARA_SLEEPTIMER:		//0x0027	//����ʱ�㱨ʱ����
			sys_SetAccStateOffInterval(tmp);
			sys_SetWatchType(1);
			break;
		default:
			break;
	}

	sys_saveRuning();
	
	return MSG_IS_OK;
}


//���ö�ʱ�ش�λ����Ϣʱ����
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
		case MSG_GPRS_PARA_WATTIMEDEF:	//0x0029	//ȱʡʱ�Ļ㱨ʱ����(s)
			sys_SetAccStateOnInterval(tmp);
			sys_SetWatchType(1);
			break;
		case MSG_GPRS_PARA_WATDISDEF:	//0x002C	//ȱʡʱ�Ļ㱨������(m)
			sys_SetDistanceInterval(tmp);
			sys_SetWatchType(2);
			break;
		default:
			break;
	}
	
	sys_saveRuning();
	
	return MSG_IS_OK;
}

//���ùյ�
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
		case MSG_GPRS_PARA_REDEEM:	//0x0030//�յ㲹���Ƕ�
			sys_SetRedeem(tmp);
			break;
		default:
			break;
	}
	
	sys_saveRuning();
	
	return MSG_IS_OK;
}

//����λ�ư뾶
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
		case MSG_GPRS_PARA_MOVE_RADIUS:	//0x0031//����Χ���뾶���Ƿ�λ����ֵ��
			sys_SetMoveDistance(tmp);
			break;
		default:
			break;
	}
	
	sys_saveRuning();
	
	return MSG_IS_OK;
}


//�Զ�����
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
		case MSG_GPRS_PARA_PHONERVTYPE:		//0x0045	//�ն˵绰��������
			if(tmp == 0)
			{
				
			}
			break;
		default:
			break;
	}
	return MSG_IS_OK;
}

//��������
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
		case MSG_GPRS_PARA_PHONECEN:			//0x0040	//���ƽ̨�绰����
			break;
		case MSG_GPRS_PARA_PHONEREBOOT:			//0x0041	//��λ�绰����
			break;
		case MSG_GPRS_PARA_PHONERESET:			//0x0042	//�ָ��������õ绰����
			break;
		case MSG_GPRS_PARA_PHONECENSMS:			//0x0043	//���ƽ̨SMS �绰����
			break;
		case MSG_GPRS_PARA_PHONESMSALM:			//0x0044	//�����ն�SMS �ı���������
			break;
		case MSG_GPRS_PARA_PHONELISTENED:		//0x0048	//�����绰����
			break;
		case MSG_GPRS_PARA_PHONERIGHT:			//0x0049	//���ƽ̨��Ȩ����
			break;
		default:
			break;
	}

	return MSG_IS_OK;
}

//������������
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
		case MSG_GPRS_PARA_ALMENABEL:			//0x0050	//����������
			break;
		case MSG_GPRS_PARA_ALMENPIC:			//0x0052	//�������㿪��
			break;
		case MSG_GPRS_PARA_ALMENPICSAVE:		//0x0053	//��������洢��־
			break;
		case MSG_GPRS_PARA_ALMSPEEDOVER:		//0x0055	//����ٶ�(km/h)
			sys_SetOverSpeed(tmp);
			sys_state_offAlarm(ALARMST_SPEED);
			sys_state_offAlarm(ALARMST_WARN_SPEED);
			break;
		case MSG_GPRS_PARA_ALMSPEEDTIME:		//0x0056	//���ٳ���ʱ��(s)
			sys_SetOverSpeedTime(tmp);
			break;
		case MSG_GPRS_PARA_ALMDRVTIMEONCE:		//0x0057	//������ʻʱ������(s)
			sys_SetDriveTimeOut(tmp);
			sys_state_offAlarm(ALARMST_DRIVETIMEOUT);
			sys_state_offAlarm(ALARMST_WARN_DRIVETIMEOUT); 
			break;
		case MSG_GPRS_PARA_ALMDRVRESET:			//0x0059//��С��Ϣʱ��(s)
			sys_SetRestTimeOut(tmp);
			break;
		case MSG_GPRS_PARA_ALMSTOPTIME:			//0x005A//�ͣ��ʱ��(s)
//			sys_SetStopTime(tmp);
			sys_state_offAlarm(ALARMST_STOP);
			break;
		default:
			break;
	}

	sys_saveRuning();
	
	return MSG_IS_OK;
}


//�������
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

//ʡ������iD
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
		case MSG_GPRS_PARA_PROVINCE:			//0x0081	//�������ڵ�ʡ��ID
			sys_SetProvinceId(tmp);
			break;
		case MSG_GPRS_PARA_CITY:				//0x0082	//�������ڵ�����ID
			sys_SetCityId(tmp);
			break;
		default:
			break;
	}

	sys_saveSys();
	modem_send_disconnect(0);

	return MSG_IS_OK;
}

//���ó��ƺ�
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

//���ó�����ɫ
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

//ͨѶ��������
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
			case MSG_GPRS_PARA_KEEPALIVE:			//0x0001	//�ն�����(s)
				oa_trac_rev_para_set_keepalive(tlv);
				break;
			case MSG_GPRS_PARA_APNMAIN:				//0x0010	//�������� APN
			case MSG_GPRS_PARA_USRMAIN:				//0x0011	//������������ͨ�Ų����û���
			case MSG_GPRS_PARA_PWDMAIN:				//0x0012	//������������ͨ�Ų�������
			case MSG_GPRS_PARA_APNSUB:				//0x0014	//���ݷ�����APN
			case MSG_GPRS_PARA_USRSUB:				//0x0015	//���ݷ���������ͨ�Ų����û���
			case MSG_GPRS_PARA_PWDSUB:				//0x0016	//���ݷ���������ͨ�Ų�������
				oa_trac_rev_para_set_apn(tlv);
				break;
			case MSG_GPRS_PARA_IPMAIN:				//0x0013	//����������ַ,IP������***
				oa_trac_rev_para_set_addr(tlv);
				break;
			case MSG_GPRS_PARA_PORTTCP:				//0x0018	//������TCP�˿�
			case MSG_GPRS_PARA_PORTUDP:				//0x0019	//������UDP�˿�
				oa_trac_rev_para_set_port(tlv);
				break;
			case MSG_GPRS_PARA_WATTYPE:				//0x0020	//λ�û㱨����
				oa_trac_rev_para_set_report_watch(tlv);
				break;
			case MSG_GPRS_PARA_SLEEPTIMER:			//0x0027	//����ʱ�㱨ʱ����
				oa_trac_rev_para_set_sleep_time(tlv);
				break;
			case MSG_GPRS_PARA_WATTIMEDEF:			//0x0029	//ȱʡʱ�Ļ㱨ʱ����(s)
			case MSG_GPRS_PARA_WATDISDEF:			//0x002C	//ȱʡʱ�Ļ㱨������(m)
				oa_trac_rev_para_set_watInt(tlv);
				break;
			case MSG_GPRS_PARA_REDEEM:				//0x0030//�յ㲹���Ƕ�
				oa_trac_rev_para_set_redeem(tlv);
				break;
			case MSG_GPRS_PARA_MOVE_RADIUS:			//0x0031//����Χ���뾶���Ƿ�λ����ֵ��
				oa_trac_rev_para_set_move_radius(tlv);
				break;
			case MSG_GPRS_PARA_PHONERVTYPE:			//0x0045	//�ն˵绰��������
				oa_trac_rev_para_set_phoneLim(tlv);
				break;
			case MSG_GPRS_PARA_PHONELISTENED:   	//0x0048    //�ն˼���
				oa_trac_rev_para_set_phoneLinsten(tlv);
				break;
			case MSG_GPRS_PARA_ALMENABEL:			//0x0050	//����������
			case MSG_GPRS_PARA_ALMENPIC:			//0x0052	//�������㿪��
			case MSG_GPRS_PARA_ALMENPICSAVE:		//0x0053	//��������洢��־
			case MSG_GPRS_PARA_ALMSPEEDOVER:		//0x0055	//����ٶ�(km/h)
			case MSG_GPRS_PARA_ALMSPEEDTIME:		//0x0056	//���ٳ���ʱ��(s)
			case MSG_GPRS_PARA_ALMDRVTIMEONCE:		//0x0057	//������ʻʱ������(s)
			case MSG_GPRS_PARA_ALMDRVRESET:			//0x0059 //��С��Ϣʱ��(s)
			case MSG_GPRS_PARA_ALMSTOPTIME:			//0x005A //�ͣ��ʱ��(s)
				oa_trac_rev_para_set_alarm(tlv);
				break;
			case MSG_GPRS_PARA_MILLAGE:				//0x0080	//������̱����(1/10km)
				oa_trac_rev_para_set_carMillage(tlv);
				break;
			case MSG_GPRS_PARA_PROVINCE:			//0x0081	//�������ڵ�ʡ��ID
			case MSG_GPRS_PARA_CITY:				//0x0082	//�������ڵ�����ID
				oa_trac_rev_para_set_carID(tlv);
				break;
			case MSG_GPRS_PARA_CARLICENCE:			//0x0083	//����
				oa_trac_rev_para_set_carLicence(tlv);
				break;
			case MSG_GPRS_PARA_CARLICENCECOL:		//0x0084	//������ɫ
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

//�ն˲�����ѯӦ��
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


//Զ������
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
			case 7:	//������ID
				break;
			case 8:	//Ӳ���汾
				break;
			case 9:	//�̼��汾
				strncpy_t(filename, pch, pend-pch);
				update_SetFilename((unsigned char *)filename);
				break;
			case 10://���ӵ�������ʱ��
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

//�ն˿���
int oa_trac_rev_terminal_ctrl(Gbroad_msg msg)
{
	int status = MSG_IS_FAIL;
	unsigned char buf[64] = {0};

	switch(msg.msg[0])
	{
		case 0x01:	//Զ������
			status = oa_trac_rev_update_init(msg);
			break;
		case 0x02:	//ָ�����ӷ�����
			break;
		case 0x04:	//��λ
			status = oa_JT_create_general_ack(msg,MSG_IS_OK,buf);		//ͨ�ûظ�
			modem_send_gprsData((char*)buf,"OK",30,status);

			OS_TaskDelayMs(3000);
			sys_Restart();
		case 0x05:	//�ָ�����
			status = oa_JT_create_general_ack(msg,MSG_IS_OK,buf);		//ͨ�ûظ�
			modem_send_gprsData((char*)buf,"OK",30,status);

			OS_TaskDelayMs(3000);
			sys_ResetFacitory();
			sys_Restart();
		case 0x03:	//�ػ�
		case 0x06:	//�ر�����ͨ��
		case 0x07:	//�ر���������ͨ��
			break;
		case 0x64://Dout 3 �Ͽ���·
			gpio_set(DOUT3, 1);
			break;
		case 0x65://Dout 3 �ָ���·
			gpio_set(DOUT3, 0);
			break;
		case 0x80://Dout1:0���ָ���1���Ͽ�
			gpio_set(DOUT1, (msg.msg[1]==0)?0:1);
			break;
		case 0x81://Dout2:0���ָ���1���Ͽ�
			gpio_set(DOUT2, (msg.msg[1]==0)?0:1);
			break;
		case 0x83://Dout4:0���ָ���1���Ͽ�
			
			break;
		default:
			break;
	}

	return status;

}

//�ն˲�����ѯӦ��
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


//�ն����Բ�ѯӦ��
int oa_trac_rev_ter_check(Gbroad_msg msg)
{
	unsigned char buf[128] = {0};

	unsigned short p = 0,len = 0;
	
	memset_t(g_sendinfo,0,sizeof(g_sendinfo));
	
	p += uint16_to_uint8(0x0080, &buf[p]);				//�ն�����

	memcpy_t(&buf[p],sys_GetManuId(),5);				//������ID
	p += 5;

	memcpy_t(&buf[p],sys_GetTermType(),20);				//�ն��ͺ�20bytes
	p += 20;
	
	memcpy_t(&buf[p],sys_GetTermId(),7);				//�ն�ID
	p += 7;

	String2Byte(modem_GetCcid(),&buf[p],20);			//CCID
	p += 10;

	buf[p++] = 7;										//�ն�Ӳ���汾����
	memcpy_t(&buf[p],"HW-V1.2",7);						//�ն�Ӳ���汾
	p += 7;

	buf[p++] = strlen_t(SMS_VER);						//�ն˹̼��汾����
	memcpy_t(&buf[p],SMS_VER,strlen_t(SMS_VER));		//�ն˹̼��汾
	p += strlen_t(SMS_VER);

	buf[p++] = 0x03;									//GNSS ģ������

	buf[p++] = 0x01;									//ͨ��ģ������
	
	msg.msg = buf;
	msg.msgattr = p;
	
	len = oa_JT_locate(MSG_GPRS_CHECKACK_APPEND,msg,g_sendinfo);
	modem_send_gprsData((char*)g_sendinfo,"OK",30,len);
	
	return MSG_IS_OK;
}

//͸����Ϣ
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

//���͵�����Ϣ
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


//������Ϣ
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
			modem_send_disconnect(0);			// ����
		}
		else if(ret == PC_RESTART)
		{
			OS_TaskDelayMs(3000);
			sys_Restart();                		// ����
		}
	}
	else
	{
		if(sign&0x01)	//����
		{
		}
		if(sign&0x04)	//������
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

//����
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

	if(msg.msg[0] == 0x00)	//��ͨ�绰
	{
		;
	}
	else //��������
	{
		modem_CallPhone(&msg.msg[1], 0);
		OS_SendMessage(xGsmQueueId, MSG_GSM_CALL, MSG_GSM_RING, 0);
	}

	return MSG_IS_OK;
}

//����
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

//����͸��
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

//�ǰ���չЭ��Ӧ��
unsigned short oa_JT_create_TLV_AWExt(unsigned short type,unsigned char *pin,unsigned short len,unsigned char *pout)
{
	unsigned short p;

	switch(type)
	{
		case 0x0003://�ն��ı���Ϣ�ϴ�
			p = uint16_to_uint8(type,&pout[0]);
			p += uint16_to_uint8(len,&pout[p]);
			if(pin && len)
			{
				memcpy_t((char*)&pout[p],pin,len);
				p += len;
			}
			break;
		case 0x0006://�ն˰汾Ӧ��
			p = uint16_to_uint8(type,&pout[0]);
			p += uint16_to_uint8(len,&pout[p]);
			memcpy_t((char*)&pout[p],pin,len);
			p += len;
			break;
		case 0x0008://��·״̬�ϱ�
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

//JT808�ڲ���չЭ��
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

//�汾��
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

//���մ���
void oa_trac_recv_msg_handle(Gbroad_msg msg)
{
	int status = 0;
	unsigned short msglen;
	
	memset_t(g_sendinfo,0,sizeof(g_sendinfo));
	
	status = MSG_IS_OK;
	switch(msg.cmdid)
	{
		case MSG_GPRS_CENGENACK:			//0x8001	//ƽ̨ͨ��Ӧ��
			oa_trac_rev_general_ack_deal(msg);
			break;
		case MSG_GPRS_PARA_SET:				//0x8103	//�����ն˲���
			status = oa_trac_rev_para_set(msg);
			break;
		case MSG_GPRS_PARA_CHECK:			//0x8104	//��ѯ�ն˲���
			status = oa_trac_rev_para_check(msg);
			break;
		case MSG_GPRS_TERCONTRL:			//0x8105	//�ն˿���
			status = oa_trac_rev_terminal_ctrl(msg);
			break;
		case MSG_GPRS_TER_CHECK:			//0x8107	//��ѯ�ն�����
			status = oa_trac_rev_ter_check(msg);
			break;
		case MSG_GPRS_ROLLCALL:				//0x8201	//λ����Ϣ��ѯ(����)
			status = oa_trac_rev_rollCall(msg);
			break;
		case MSG_GPRS_SCHEDDOWN:			//0x8300	//�ı���Ϣ�·�
			status = oa_trac_rev_post_scheduling(msg);
			break;
		case MSG_GPRS_PHONELISTEN:			//0x8400	//�绰�ز�
			status = oa_trac_rev_monitor(msg);
			break;
		case MSG_GPRS_CARCONTROL:			//0x8500	//��������
			break;
		case MSG_GPRS_GENERAL_DOWNLOAD:		//0x8900//��������͸��
			status = oa_trac_rev_general_aw_msg(msg);
			break;
		case MSG_GPRS_AWVERSION:			//0x8702//�ǰ���չӦ��
			status = oa_trac_rev_expend_aw_version();
			break;
		case MSG_GPRS_AWEXTDOWN:			//0x8FFE//�ǰ���չӦ��
			status = oa_trac_rev_expend_aw_msg(msg);
			break;
		default:
			status = MSG_IS_NOT_SUPORT;
			break;
	}

	//ͨ��Ӧ��
	switch(msg.cmdid)
	{
		case MSG_GPRS_PARA_SET:				//0x8103	//�����ն˲���
		case MSG_GPRS_TERCONTRL:			//0x8105	//�ն˿���
		case MSG_GPRS_PHONELISTEN:			//0x8400	//�绰�ز�
		case MSG_GPRS_CARCONTROL:			//0x8500	//��������
		case MSG_GPRS_GENERAL_DOWNLOAD:		//0x8900	//��������͸��
		case MSG_GPRS_AWVERSION:			//0x8702//�ǰ���չӦ��
		case MSG_GPRS_AWEXTDOWN:			//0x8FFE	//�ǰ���չӦ��
			msglen = oa_JT_create_general_ack(msg,status&0x0F,g_sendinfo);		//ͨ�ûظ�
			modem_send_gprsData((char*)g_sendinfo,"OK",30,msglen);
			break;
		default:
			break;
	}
}

//���մ���
void oa_recv_Handle(char* buf,unsigned short buflen)
{
	int msglen = 0;

	Gbroad_msg msg_rcv = {0};
	
	if((buf==NULL) || (buflen==0))
	{
	   	return;
	}

	if(modem_GetState() == GSM_FOTA_STATE) //Զ����������Ϣ�Ĵ���
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

