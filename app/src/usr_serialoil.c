#include "includes.h"

static g_oil_buf g_oil;
static g_oil_check g_oilcheck;

static unsigned short sAccOffBaseOilPercent[MAX_OIL_CH] = {0};
// 1min 1个点,以第1个点为基准点,比较后5个点,若有1个点异常报警
static unsigned short sAccOnTimeWindowOilPercent[MAX_OIL_CH][6] = {0};
static unsigned short sAccOnTimeCount[MAX_OIL_CH] = {0};

//油耗初始化
void serialoil_Init(void)          
{	
	serialoil_PowerOn();
	
	serialoil_UartInit();
}

//油耗电源开
void serialoil_PowerOn(void)
{
	if(serialoil_enable_urtfun(COM0) == ENABLE)
	{
    	gpio_232_powerCtrl(1);
	}
	if(serialoil_enable_urtfun(COM4) == ENABLE)
	{
    	gpio_232_powerCtrl(1);
	}
}

//油耗电源关
void serialoil_PowerOff(void)
{
	if(serialoil_enable_urtfun(COM0) == ENABLE)
	{
    	gpio_232_powerCtrl(0);
	}
	if(serialoil_enable_urtfun(COM4) == ENABLE)
	{
    	gpio_232_powerCtrl(0);
	}
}

//油耗串口初始化
void serialoil_UartInit(void)
{
	if(serialoil_enable_urtfun(COM0) == ENABLE)
	{
    	uart1_init(9600);
	}
	if(serialoil_enable_urtfun(COM4) == ENABLE)
	{
		uart4_init(9600);
	}
}

//串口发送
void serialoil_uart_write(unsigned char type, unsigned char* data, unsigned int writeLen)
{
	if(type == JT)
	{
		if((sys_GetUartFunc(0)&URT_MASK_JT_OIL) == URT_MASK_JT_OIL)
		{
			print1(data,writeLen);
		}
		if((sys_GetUartFunc(1)&URT_MASK_JT_OIL) == URT_MASK_JT_OIL)
		{
			print4(data,writeLen);
		}
	}
	if(type == CR)
	{
		if((sys_GetUartFunc(0)&URT_MASK_CR_OIL) == URT_MASK_CR_OIL)
		{
			print1(data,writeLen);
		}
		if((sys_GetUartFunc(1)&URT_MASK_CR_OIL) == URT_MASK_CR_OIL)
		{
			print4(data,writeLen);
		}
	}
	if(type == XD)
	{
		if((sys_GetUartFunc(0)&URT_MASK_XD_OIL) == URT_MASK_XD_OIL)
		{
			print1(data,writeLen);
		}
		if((sys_GetUartFunc(1)&URT_MASK_XD_OIL) == URT_MASK_XD_OIL)
		{
			print4(data,writeLen);
		}
	}
}

//清零
void serialoil_noinit_clear(void)
{
	memset_t(&g_oil, 0, sizeof(g_oil));
	memset_t(&g_oilcheck, 0, sizeof(g_oilcheck));
}

//校验和
unsigned char sum_check(unsigned char *data, unsigned int len)
{
	unsigned char result = data[0];
	unsigned int i = 0;
	
	while(++i < len)
	{
		result = result + data[i];
	}

	return result;
}

unsigned short sumTo16_check(unsigned char *data, unsigned int len)
{
	unsigned short result = data[0];
	unsigned int i = 0;
	
	while(++i < len)
	{
		result = result + data[i];
	}

	return result;
}
//检测使能
unsigned char serialoil_enable_urtfun(unsigned char type)
{
	if(type == COM0)
	{
		if(((sys_GetUartFunc(0)&URT_MASK_JT_OIL) == URT_MASK_JT_OIL) || \
			((sys_GetUartFunc(0)&URT_MASK_CR_OIL) == URT_MASK_CR_OIL) || \
			((sys_GetUartFunc(0)&URT_MASK_XD_OIL) == URT_MASK_XD_OIL))
		{
			return ENABLE;
		}
	}
	if(type == COM4 )
	{
		if(((sys_GetUartFunc(1)&URT_MASK_JT_OIL) == URT_MASK_JT_OIL) || \
			((sys_GetUartFunc(1)&URT_MASK_CR_OIL) == URT_MASK_CR_OIL) || \
			((sys_GetUartFunc(1)&URT_MASK_XD_OIL) == URT_MASK_XD_OIL))
		{
			return ENABLE;
		}
	}
	if(type == ALL)
	{
		if(((sys_GetUartFunc(0)&URT_MASK_JT_OIL) == URT_MASK_JT_OIL) || \
			((sys_GetUartFunc(1)&URT_MASK_JT_OIL) == URT_MASK_JT_OIL) || \
			((sys_GetUartFunc(0)&URT_MASK_CR_OIL) == URT_MASK_CR_OIL) || \
			((sys_GetUartFunc(1)&URT_MASK_CR_OIL) == URT_MASK_CR_OIL) || \
			((sys_GetUartFunc(0)&URT_MASK_XD_OIL) == URT_MASK_XD_OIL) || \
			((sys_GetUartFunc(1)&URT_MASK_XD_OIL) == URT_MASK_XD_OIL))
		{
			return ENABLE;
		}
	}

	return DISABLE;
}

//获取油耗类型
unsigned char serialoil_get_type(unsigned char type)
{
	if(type == JT)
	{
		if(((sys_GetUartFunc(0)&URT_MASK_JT_OIL) == URT_MASK_JT_OIL) || \
			((sys_GetUartFunc(1)&URT_MASK_JT_OIL) == URT_MASK_JT_OIL))
		{
			return JT;
		}
	}
	if(type == CR)
	{
		if(((sys_GetUartFunc(0)&URT_MASK_CR_OIL) == URT_MASK_CR_OIL) || \
			((sys_GetUartFunc(1)&URT_MASK_CR_OIL) == URT_MASK_CR_OIL))
		{
			return CR;
		}
	}
	if(type == XD)
	{
		if(((sys_GetUartFunc(0)&URT_MASK_XD_OIL) == URT_MASK_XD_OIL) || \
			((sys_GetUartFunc(1)&URT_MASK_XD_OIL) == URT_MASK_XD_OIL))
		{
			return XD;
		}
	}

	return NUL;
}

static unsigned char g_oil_ack_flag;
static unsigned char g_oil_ack_waittime;
static unsigned short g_oil_ack_volume;
static unsigned short g_oil_ack_damp;

/***************************久通***************************/
//读取油耗
void serialoil_read_cmd_jt(unsigned char id)
{
	char buf[24] = {0};
	unsigned char crc;

	sprintf(buf, "%02dE00",id);
	crc = sum_check((unsigned char *)buf, strlen_t(buf));
	sprintf(buf, "@%02dE00%02X#",id,crc);

	serialoil_uart_write(JT, (unsigned char *)buf, strlen_t(buf));
}

//设置空箱标定
void serialoil_empty_cmd_jt(unsigned char id)
{
	char buf[24] = {0};
	unsigned char crc;
	
	sprintf(buf, "%02dB011",id);
	crc = sum_check((unsigned char *)buf, strlen_t(buf));
	sprintf(buf, "@%02dB011%02X#",id,crc);

	serialoil_uart_write(JT, (unsigned char *)buf, strlen_t(buf));
}

//查询空箱标定
void serialoil_empty_check_jt(unsigned char id)
{
	char buf[24] = {0};
	unsigned char crc;
	
	sprintf(buf, "%02dB010",id);
	crc = sum_check((unsigned char *)buf, strlen_t(buf));
	sprintf(buf, "@%02dB010%02X#",id,crc);

	serialoil_uart_write(JT, (unsigned char *)buf, strlen_t(buf));
}

//设置满箱标定
void serialoil_full_cmd_jt(unsigned char id)
{
	char buf[24] = {0};
	unsigned char crc;
	
	sprintf(buf, "%02dC011",id);
	crc = sum_check((unsigned char *)buf, strlen_t(buf));
	sprintf(buf, "@%02dC011%02X#",id,crc);

	serialoil_uart_write(JT, (unsigned char *)buf, strlen_t(buf));
}

//查询满箱标定
void serialoil_full_check_jt(unsigned char id)
{
	char buf[24] = {0};
	unsigned char crc;
	
	sprintf(buf, "%02dC010",id);
	crc = sum_check((unsigned char *)buf, strlen_t(buf));
	sprintf(buf, "@%02dC010%02X#",id,crc);

	serialoil_uart_write(JT, (unsigned char *)buf, strlen_t(buf));
}

//设置邮箱体积
void serialoil_volume_cmd_jt(unsigned char id, unsigned short volume)
{
	char buf[24] = {0};
	unsigned char crc;
	
	sprintf(buf, "%02dD061%05d",id,volume*100);
	crc = sum_check((unsigned char *)buf, strlen_t(buf));
	sprintf(buf, "@%02dD061%05d%02X#",id,volume*100,crc);

	serialoil_uart_write(JT, (unsigned char *)buf, strlen_t(buf));
}

//查询邮箱体积
void serialoil_volume_check_jt(unsigned char id)
{
	char buf[24] = {0};
	unsigned char crc;
	
	sprintf(buf, "%02dD06000000",id);
	crc = sum_check((unsigned char *)buf, strlen_t(buf));
	sprintf(buf, "@%02dD06000000%02X#",id,crc);

	serialoil_uart_write(JT, (unsigned char *)buf, strlen_t(buf));
}

//设置阻尼
void serialoil_damp_cmd_jt(unsigned char id, unsigned short timedamp)
{
	char buf[24] = {0};
	unsigned char crc;
	
	if(timedamp < 5)
	{
		timedamp = 5;
	}
	if(timedamp > 600)
	{
		timedamp = 600;
	}
	sprintf(buf, "%02dH051%04d",id,timedamp);
	crc = sum_check((unsigned char *)buf, strlen_t(buf));
	sprintf(buf, "@%02dH051%04d%02X#",id,timedamp,crc);

	serialoil_uart_write(JT, (unsigned char *)buf, strlen_t(buf));
}

//查询阻尼
void serialoil_damp_check_jt(unsigned char id)
{
	char buf[24] = {0};
	unsigned char crc;
	
	sprintf(buf, "%02dH0500000",id);
	crc = sum_check((unsigned char *)buf, strlen_t(buf));
	sprintf(buf, "@%02dH0500000%02X#",id,crc);

	serialoil_uart_write(JT, (unsigned char *)buf, strlen_t(buf));
}

//设置ID
void serialoil_setid_cmd_jt(unsigned char preid, unsigned char nowid)
{
	char buf[24] = {0};
	unsigned char crc;
	
	sprintf(buf, "%02dG031%02d",preid,nowid);
	crc = sum_check((unsigned char *)buf, strlen_t(buf));
	sprintf(buf, "@%02dG031%02d%02X#",preid,nowid,crc);

	serialoil_uart_write(JT, (unsigned char *)buf, strlen_t(buf));
}

//查询ID
void serialoil_checkid_cmd_jt(void)
{
	char buf[24] = {0};
	unsigned char crc;
	
	sprintf(buf, "00G03000");
	crc = sum_check((unsigned char *)buf, strlen_t(buf));
	sprintf(buf, "@00G03000%02X#",crc);

	serialoil_uart_write(JT, (unsigned char *)buf, strlen_t(buf));
}

//串口设置参数
/*
	x = id, y = value
	
	*J:0,x,y,  	设置油箱体积
	*J:1,x		设置调零值
	*J:2,x		设置调满值
	*J:3,x,y,	设置阻尼值(5--600)
	*J:4,x		查询体积、阻尼
	*J:5,x1,x2	设置ID(只能接单个设置)x1:当前的ID  x2:修改的ID(从1开始)
	*J:6		查询ID(只能接单个查询)
*/
void serialoil_uart_set_jt(unsigned char *data, unsigned char *pOut)
{
	unsigned char id;
	unsigned char preid;
	unsigned short value;
	
	if(data[0] == '0')//设置油箱体积
	{
		id = data[2] - '0';
		value = myatoi((char *)&data[4]);

		sys_SetOilVolume(id,value);
		sys_saveRuning();

		serialoil_volume_cmd_jt(id, value);
		g_oil_ack_flag = 0;
		g_oil_ack_waittime = 0;
		while((g_oil_ack_flag == 0)&&(g_oil_ack_waittime < 100))
		{
			OS_TaskDelayMs(10);
			g_oil_ack_waittime++;
		}
		if(g_oil_ack_waittime < 100)
		{
			sprintf((char *)pOut,"CH%d Oil = Volume Ok",id);
		}
		else
		{
			sprintf((char *)pOut,"CH%d Oil = Volume Fail",id);
		}
	}
	else if(data[0] == '1')//设置调零值
	{
		id = data[2] - '0';
		
		serialoil_empty_cmd_jt(id);
		g_oil_ack_flag = 0;
		g_oil_ack_waittime = 0;
		while((g_oil_ack_flag == 0)&&(g_oil_ack_waittime < 100))
		{
			OS_TaskDelayMs(10);
			g_oil_ack_waittime++;
		}
		if(g_oil_ack_waittime < 100)
		{
			sprintf((char *)pOut,"CH%d Oil = Empty Ok",id);
		}
		else
		{
			sprintf((char *)pOut,"CH%d Oil = Empty Fail",id);
		}
	}
	else if(data[0] == '2')//设置调满值
	{
		id = data[2] - '0';
		
		serialoil_full_cmd_jt(id);
		g_oil_ack_flag = 0;
		g_oil_ack_waittime = 0;
		while((g_oil_ack_flag == 0)&&(g_oil_ack_waittime < 100))
		{
			OS_TaskDelayMs(10);
			g_oil_ack_waittime++;
		}
		if(g_oil_ack_waittime < 100)
		{
			sprintf((char *)pOut,"CH%d Oil = Full Ok",id);
		}
		else
		{
			sprintf((char *)pOut,"CH%d Oil = Full Fail",id);
		}
	}
	else if(data[0] == '3')//设置阻尼值
	{
		id = data[2] - '0';
		value = myatoi((char *)&data[4]);

		serialoil_damp_cmd_jt(id, value);
		g_oil_ack_flag = 0;
		g_oil_ack_waittime = 0;
		while((g_oil_ack_flag == 0)&&(g_oil_ack_waittime < 100))
		{
			OS_TaskDelayMs(10);
			g_oil_ack_waittime++;
		}
		if(g_oil_ack_waittime < 100)
		{
			sprintf((char *)pOut,"CH%d Oil = Damp Ok",id);
		}
		else
		{
			sprintf((char *)pOut,"CH%d Oil = Damp Fail",id);
		}
	}
	else if(data[0] == '4')//查询
	{
		id = data[2] - '0';

		serialoil_volume_check_jt(id);
		g_oil_ack_flag = 0;
		g_oil_ack_waittime = 0;
		while((g_oil_ack_flag == 0)&&(g_oil_ack_waittime < 100))
		{
			OS_TaskDelayMs(10);
			g_oil_ack_waittime++;
		}
		if(g_oil_ack_waittime < 100)
		{
			sprintf((char *)pOut,"CH%d Volume:%d  ",id,g_oil_ack_volume);
		}
		else
		{
			sprintf((char *)pOut,"CH%d Volume Fail  ",id);
		}

		serialoil_damp_check_jt(id);
		g_oil_ack_flag = 0;
		g_oil_ack_waittime = 0;
		while((g_oil_ack_flag == 0)&&(g_oil_ack_waittime < 100))
		{
			OS_TaskDelayMs(10);
			g_oil_ack_waittime++;
		}
		if(g_oil_ack_waittime < 100)
		{
			sprintf((char *)pOut+strlen_t((char *)pOut),"CH%d Damp:%d  ",id,g_oil_ack_damp);
		}
		else
		{
			sprintf((char *)pOut+strlen_t((char *)pOut),"CH%d Damp Fail  ",id);
		}
	}
	else if(data[0] == '5')//设置ID
	{
		preid = data[2] - '0';

		id = data[4] - '0';

		serialoil_setid_cmd_jt(preid,id);
		g_oil_ack_flag = 0;
		g_oil_ack_waittime = 0;
		while((g_oil_ack_flag == 0)&&(g_oil_ack_waittime < 100))
		{
			OS_TaskDelayMs(10);
			g_oil_ack_waittime++;
		}
		if(g_oil_ack_waittime < 100)
		{
			memset_t(&g_oilcheck, 0, sizeof(g_oil_check));
			sprintf((char *)pOut,"SET CH%d Success  ",id);
		}
		else
		{
			sprintf((char *)pOut,"SET CH%d Fail  ",id);
		}
	}
	else if(data[0] == '6')//查询ID
	{
		serialoil_checkid_cmd_jt();
		g_oil_ack_flag = 0;
		g_oil_ack_waittime = 0;
		while((g_oil_ack_flag == 0)&&(g_oil_ack_waittime < 100))
		{
			OS_TaskDelayMs(10);
			g_oil_ack_waittime++;
		}
		if(g_oil_ack_waittime < 100)
		{
			sprintf((char *)pOut,"Check ID Success  ");
		}
		else
		{
			sprintf((char *)pOut,"Check ID Fail  ");
		}
	}
	else
	{
		strcpy((char *)pOut,"PARA ERR");
	}
	
}

//返回数据处理函数
void serialoil_handle_jt(unsigned char com_type, unsigned char* data, unsigned int len)
{
	unsigned char i,id;
	unsigned char crc,RetCrc;
	unsigned int tmpoil;

	if(serialoil_enable_urtfun(com_type) == DISABLE)
	{
		return;
	}

	if((data == NULL) || (len < 4))
	{
		return;
	}
	//LOG("[JT Oil] rsv[%d]: %s\r\n",len,data);
	
	//返回读取油耗命令
	if((data[0] == '@')&&(data[3] == 'E'))
	{		
		crc = sum_check(data+1, len-4);

		RetCrc = CHAR2HEX(data[len-3]);
		RetCrc <<= 4;
		RetCrc |= CHAR2HEX(data[len-2]);

		if(crc == RetCrc)
		{
			//取id号
			id = 0;
			for(i=0; i<2; i++)
			{
				id = id*10;
				id += CHAR2DEC(data[1+i]);
			}
			if((id == 0) || (id > MAX_OIL_CH))
			{
				return;
			}
			// 2路232油耗同时开启
			if((serialoil_enable_urtfun(COM0) == ENABLE) && \
				(serialoil_enable_urtfun(COM4) == ENABLE))
			{
				//规定COM0的油耗对应协议的第一路
				if(com_type == COM0)
				{
					id = 1;
				}
				//规定COM4的油耗对应协议的第二路
				if(com_type == COM4)
				{
					id = 2;
				}
			}
			//置一表示已经正常通信
			g_oilcheck.nOffLineAlarmCount[id-1] = 1;
			g_oilcheck.nOffLineAlarmSt &= ~(1<<(id-1));
			
			//取油耗万分比
			tmpoil = 0;
            for(i=0; i<5; i++)
            {
				tmpoil = tmpoil*10;
				tmpoil += CHAR2DEC(data[6+i]);
			}

			g_oil.nCurOilPercent[id-1] = tmpoil;
			LOG("[serialoil] CH%d Oil Percent=%d\r\n",id,tmpoil);

			//取油耗AD
			tmpoil = 0;
            for(i=0; i<4; i++)
            {
				tmpoil = tmpoil*10;
				tmpoil += CHAR2DEC(data[11+i]);
			}

			g_oil.nCurOilAD[id-1] = tmpoil;
			LOG("[serialoil] CH%d Oil AD=%d\r\n",id,tmpoil);
			
			//取油耗升数
			tmpoil = 0;
            for(i=0; i<6; i++)
            {
				tmpoil = tmpoil*10;
				tmpoil += CHAR2DEC(data[15+i]);
			}

			g_oil.nCurOilVolume[id-1] = tmpoil;
			LOG("[serialoil] CH%d Oil Volume =%d\r\n",id,tmpoil);
		}
	}
	//返回满箱标定
	if((data[0] == '@')&&(data[3] == 'C'))
	{
		crc = sum_check(data+1, len-4);

		RetCrc = CHAR2HEX(data[len-3]);
		RetCrc <<= 4;
		RetCrc |= CHAR2HEX(data[len-2]);

		if(crc == RetCrc)
		{
			//应答成功
			g_oil_ack_flag = 1;
			
			//取id号
			id = 0;
			for(i=0; i<2; i++)
			{
				id = id*10;
				id += CHAR2DEC(data[1+i]);
			}
			if((id == 0) || (id > MAX_OIL_CH))
			{
				return;
			}
			LOG("[serialoil] CH%d Full Oil Calibration Ok\r\n",id);
		}
	}
	//返回空箱标定
	if((data[0] == '@')&&(data[3] == 'B'))
	{
		crc = sum_check(data+1, len-4);

		RetCrc = CHAR2HEX(data[len-3]);
		RetCrc <<= 4;
		RetCrc |= CHAR2HEX(data[len-2]);

		if(crc == RetCrc)
		{
			//应答成功
			g_oil_ack_flag = 1;
			
			//取id号
			id = 0;
			for(i=0; i<2; i++)
			{
				id = id*10;
				id += CHAR2DEC(data[1+i]);
			}
			if((id == 0) || (id > MAX_OIL_CH))
			{
				return;
			}
			LOG("[serialoil] CH%d Empty Oil Calibration Ok\r\n",id);
		}
	}
	//返回体积
	if((data[0] == '@')&&(data[3] == 'D'))
	{
		crc = sum_check(data+1, len-4);

		RetCrc = CHAR2HEX(data[len-3]);
		RetCrc <<= 4;
		RetCrc |= CHAR2HEX(data[len-2]);

		if(crc == RetCrc)
		{
			//应答成功
			g_oil_ack_flag = 1;
			
			//取id号
			id = 0;
			for(i=0; i<2; i++)
			{
				id = id*10;
				id += CHAR2DEC(data[1+i]);
			}
			if((id == 0) || (id > MAX_OIL_CH))
			{
				return;
			}
			//取油耗体积
			tmpoil = 0;
			for(i=0; i<6; i++)
			{
				tmpoil = tmpoil*10;
				tmpoil += CHAR2DEC(data[6+i]);
			}
			g_oil_ack_volume = tmpoil/100;
			LOG("[serialoil] CH%d Volume Value=%d\r\n",id,tmpoil/100);
		}
	}
	//返回阻尼
	if((data[0] == '@')&&(data[3] == 'H'))
	{
		crc = sum_check(data+1, len-4);

		RetCrc = CHAR2HEX(data[len-3]);
		RetCrc <<= 4;
		RetCrc |= CHAR2HEX(data[len-2]);

		if(crc == RetCrc)
		{
			//应答成功
			g_oil_ack_flag = 1;
			
			//取id号
			id = 0;
			for(i=0; i<2; i++)
			{
				id = id*10;
				id += CHAR2DEC(data[1+i]);
			}
			if((id == 0) || (id > MAX_OIL_CH))
			{
				return;
			}
			//取油耗阻尼值
			tmpoil = 0;
			for(i=0; i<4; i++)
			{
				tmpoil = tmpoil*10;
				tmpoil += CHAR2DEC(data[6+i]);
			}
			g_oil_ack_damp = tmpoil;
			LOG("[serialoil] CH%d Damp Value=%d\r\n",id,tmpoil);
		}
	}
	//返回查询ID
	if((data[0] == '@')&&(data[3] == 'G'))
	{
		crc = sum_check(data+1, len-4);

		RetCrc = CHAR2HEX(data[len-3]);
		RetCrc <<= 4;
		RetCrc |= CHAR2HEX(data[len-2]);

		if(crc == RetCrc)
		{
			//应答成功
			g_oil_ack_flag = 1;
			
			//取id号
			id = 0;
			for(i=0; i<2; i++)
			{
				id = id*10;
				id += CHAR2DEC(data[6+i]);
			}
			if((id == 0) || (id > MAX_OIL_CH))
			{
				return;
			}
			
			LOG("[serialoil] ID=%d\r\n",id);
		}
	}
}

unsigned short serialoil_get_percent_jt(unsigned char id)
{
	unsigned short AD;
	
	if((id < 1) || (id > MAX_OIL_CH))
	{
		return 0;
	}
	
	AD = g_oil.nCurOilAD[id-1];
	
	return AD;
}
/***************************END***************************/

/***************************长润***************************/
//读取油耗
void serialoil_read_cmd_cr(unsigned char id)
{
	char buf[24] = {0};
	unsigned char crc;

	sprintf(buf, "$!RY%02d",id);
	crc = sum_check((unsigned char *)buf, strlen_t(buf));
	sprintf(buf, "$!RY%02d%02X\r\n",id,crc);

	serialoil_uart_write(CR, (unsigned char *)buf, strlen_t(buf));
}

//设置阻尼
void serialoil_damp_cmd_cr(unsigned char id, unsigned short timedamp)
{
	char buf[24] = {0};
	unsigned char crc;
	
	if(timedamp > 9)
	{
		timedamp = 9;
	}
	sprintf(buf, "$!Z%d%02d",timedamp,id);
	crc = sum_check((unsigned char *)buf, strlen_t(buf));
	sprintf(buf, "$!Z%d%02d%02X\r\n",timedamp,id,crc);

	serialoil_uart_write(CR, (unsigned char *)buf, strlen_t(buf));
}

//设置ID
void serialoil_setid_cmd_cr(unsigned char id)
{
	char buf[24] = {0};
	unsigned char crc;
	
	if(id > 99)
	{
		id = 1;
	}
	sprintf(buf, "$!ID%02d",id);
	crc = sum_check((unsigned char *)buf, strlen_t(buf));
	sprintf(buf, "$!ID%02d%02X\r\n",id,crc);

	serialoil_uart_write(CR, (unsigned char *)buf, strlen_t(buf));
}

//串口设置参数
/*
	x = id, y = value
	
	*J:3,x,y,	设置阻尼值(0--9)
	*J:5,ID		设置ID(只能接单个设置)ID:修改的ID(从1开始)
*/
void serialoil_uart_set_cr(unsigned char *data, unsigned char *pOut)
{
	unsigned char id;
	unsigned short value;
	
	if(data[0] == '3')//设置阻尼值
	{
		id = data[2] - '0';
		value = myatoi((char *)&data[4]);

		serialoil_damp_cmd_cr(id, value);
		g_oil_ack_flag = 0;
		g_oil_ack_waittime = 0;
		while((g_oil_ack_flag == 0)&&(g_oil_ack_waittime < 100))
		{
			OS_TaskDelayMs(10);
			g_oil_ack_waittime++;
		}
		if(g_oil_ack_waittime < 100)
		{
			sprintf((char *)pOut,"CH%d Oil = Damp Ok",id);
		}
		else
		{
			sprintf((char *)pOut,"CH%d Oil = Damp Fail",id);
		}
	}
	else if(data[0] == '5')//设置ID
	{
		id = data[2] - '0';

		serialoil_setid_cmd_cr(id);
		g_oil_ack_flag = 0;
		g_oil_ack_waittime = 0;
		while((g_oil_ack_flag == 0)&&(g_oil_ack_waittime < 100))
		{
			OS_TaskDelayMs(10);
			g_oil_ack_waittime++;
		}
		if(g_oil_ack_waittime < 100)
		{
			memset_t(&g_oilcheck, 0, sizeof(g_oil_check));
			sprintf((char *)pOut,"SET CH%d Success  ",id);
		}
		else
		{
			sprintf((char *)pOut,"SET CH%d Fail  ",id);
		}
	}
	else
	{
		strcpy((char *)pOut,"PARA ERR");
	}
}

//返回数据处理函数
void serialoil_handle_cr(unsigned char com_type, unsigned char* data, unsigned int len)
{
	unsigned char i,id;
	unsigned char crc,RetCrc;
	unsigned int tmpoil;

	if(serialoil_enable_urtfun(com_type) == DISABLE)
	{
		return;
	}

	if((data == NULL) || (len < 4))
	{
		return;
	}
	LOG("[CR Oil] rsv[%d]: %s\r\n",len,data);
	
	//返回读取油耗命令
	if((data[0] == '*')&&(data[1] == 'C')&&(data[2] == 'F')&&(data[3] == 'V'))
	{		
		crc = sum_check(data, len-4);

		RetCrc = CHAR2HEX(data[len-4]);
		RetCrc <<= 4;
		RetCrc |= CHAR2HEX(data[len-3]);

		if(crc == RetCrc)
		{
			//取id号
			id = 0;
			for(i=0; i<2; i++)
			{
				id = id*10;
				id += CHAR2DEC(data[4+i]);
			}
			if((id == 0) || (id > MAX_OIL_CH))
			{
				return;
			}
			// 2路232油耗同时开启
			if((serialoil_enable_urtfun(COM0) == ENABLE) && \
				(serialoil_enable_urtfun(COM4) == ENABLE))
			{
				//规定COM0的油耗对应协议的第一路
				if(com_type == COM0)
				{
					id = 1;
				}
				//规定COM4的油耗对应协议的第二路
				if(com_type == COM4)
				{
					id = 2;
				}
			}
			//置一表示已经正常通信
			g_oilcheck.nOffLineAlarmCount[id-1] = 1;
			g_oilcheck.nOffLineAlarmSt &= ~(1<<(id-1));
			
			//取油耗万分比
			tmpoil = 0;
            for(i=0; i<6; i++)
            {
				tmpoil <<= 4;
				tmpoil |= CHAR2HEX(data[6+i]);
			}

			g_oil.nCurOilAD[id-1] = tmpoil;
			LOG("[serialoil] CH%d Oil AD=%d\r\n",id,tmpoil);
		}
	}
	//返回阻尼
	if((data[0] == '*')&&(data[1] == 'S')&&(data[2] == 'Z')&&(data[3] == 'N'))
	{
		crc = sum_check(data, len-4);

		RetCrc = CHAR2HEX(data[len-4]);
		RetCrc <<= 4;
		RetCrc |= CHAR2HEX(data[len-3]);

		if(crc == RetCrc)
		{
			//应答成功
			g_oil_ack_flag = 1;
			
			//取id号
			id = 0;
			for(i=0; i<2; i++)
			{
				id = id*10;
				id += CHAR2DEC(data[4+i]);
			}
			if((id == 0) || (id > MAX_OIL_CH))
			{
				return;
			}
			if((data[6] == 'O')&&(data[7] == 'K'))
			{
				LOG("[serialoil] CH%d Damp Success\r\n",id);
			}
			else
			{
				LOG("[serialoil] CH%d Damp Fail\r\n",id);
			}
		}
	}
	//返回ID
	if((data[0] == '*')&&(data[1] == 'S')&&(data[2] == 'I')&&(data[3] == 'D'))
	{
		crc = sum_check(data, len-4);

		RetCrc = CHAR2HEX(data[len-4]);
		RetCrc <<= 4;
		RetCrc |= CHAR2HEX(data[len-3]);

		if(crc == RetCrc)
		{
			//应答成功
			g_oil_ack_flag = 1;
			
			//取id号
			id = 0;
			for(i=0; i<2; i++)
			{
				id = id*10;
				id += CHAR2DEC(data[4+i]);
			}
			if((id == 0) || (id > MAX_OIL_CH))
			{
				return;
			}
			if((data[6] == 'O')&&(data[7] == 'K'))
			{
				LOG("[serialoil] CH%d Damp Success\r\n",id);
			}
			else
			{
				LOG("[serialoil] CH%d Damp Fail\r\n",id);
			}
		}
	}
}

unsigned short serialoil_get_percent_cr(unsigned char id)
{
	unsigned short AD;
	
	if((id < 1) || (id > MAX_OIL_CH))
	{
		return 0;
	}
	
	AD = g_oil.nCurOilAD[id-1];
	
	return AD;
}
/***************************END***************************/

/***************************忻德***************************/

//返回数据处理函数
void serialoil_handle_xd(unsigned char com_type, unsigned char* data, unsigned int len)
{
	unsigned char id;
	unsigned int tmpoil;
	unsigned short crc,RetCrc;
	if(serialoil_enable_urtfun(com_type) == DISABLE)
	{
		return;
	}

	if((data == NULL) || (len < 4))
	{
		return;
	}
	LOG("[XD Oil] rsv[%d]: %s\r\n",len,data);
	
	//返回读取油耗命令
	if((data[0] == '*')&&(data[1] == 'X')&&(data[2] == 'D'))
	{		
		/*  *XD,0000,01,1786,1796,0000,0320,1371#   */
		crc = sumTo16_check(&data[4], len-9);
		RetCrc = myatoi((char*)&data[len-5]);

		if(crc == RetCrc)
		{
			// 2路232油耗同时开启
//			if((serialoil_enable_urtfun(COM0) == ENABLE) && \
//				(serialoil_enable_urtfun(COM4) == ENABLE))
//			{
//				//规定COM0的油耗对应协议的第一路
//				if(com_type == COM0)
//				{
//					id = 1;
//				}
//				//规定COM4的油耗对应协议的第二路
//				if(com_type == COM4)
//				{
//					id = 2;
//				}
//			}
//			else
//			{
				id = 1;
//			}
			//置一表示已经正常通信
			g_oilcheck.nOffLineAlarmCount[id-1] = 1;
			g_oilcheck.nOffLineAlarmSt &= ~(1<<(id-1));
			
			g_oil.nCurOilAD[id-1] = myatoi((char*)&data[12]);
			g_oil.nCurOilAD[id] =  myatoi((char*)&data[17]);
			LOG("[serialoil] CH%d Oil HIGH=%d\r\n",id,tmpoil);
		}
	}
}

unsigned short serialoil_get_percent_xd(unsigned char id)
{
	unsigned short AD;
	
	if((id < 1) || (id > MAX_OIL_CH))
	{
		return 0;
	}
	
	AD = g_oil.nCurOilAD[id-1];
	
	return AD;
}
/***************************END***************************/

//断线报警
void serialoil_offalarm_check(void)
{
	unsigned char i;

	if(serialoil_enable_urtfun(ALL) == DISABLE)
	{
		for(i=0; i<MAX_OIL_CH; i++)
		{
			g_oilcheck.nOffLineAlarmCount[i] = 0;
		}
		return;
	}
	
	for(i=0; i<MAX_OIL_CH; i++)
	{
		//正常转异常且60s均无应答,报警
		if(g_oilcheck.nOffLineAlarmCount[i])
		{
			g_oilcheck.nOffLineAlarmCount[i]++;
			if(g_oilcheck.nOffLineAlarmCount[i] > 60)
			{
				g_oilcheck.nOffLineAlarmSt |= (1<<i);
				g_oilcheck.nOffLineAlarmCount[i] = 0;
				g_oil.nCurOilPercent[i] = 0;
				g_oil.nCurOilVolume[i]  = 0;
				g_oil.nCurOilAD[i]      = 0;
				
				LOG("[serialoil] offalarm[CH: %d]\r\n",i+1);
			}
		}
	}
}

//油位异常报警
/*
1、ACC关,进行一次采样,做为油位比较基准值.
2、ACC开,以时间窗口的方式检测油位
*/
void serialoil_abnormal_accoff(unsigned char id)
{
	if(serialoil_get_percent(id) == 0)
	{
		sAccOffBaseOilPercent[id-1] = 0;
		return;
	}
	if(sAccOffBaseOilPercent[id-1] == 0)
	{
		//基准点
		sAccOffBaseOilPercent[id-1] = serialoil_get_percent(id);
	}
	if(sAccOffBaseOilPercent[id-1] > sys_GetOilToSteal())
	{
		if(sAccOffBaseOilPercent[id-1] >= serialoil_get_percent(id))
		{
			if((sAccOffBaseOilPercent[id-1] - serialoil_get_percent(id)) >= sys_GetOilToSteal())
			{
				sys_state_newAlarm(ALARMST_OIL_ABNO);//偷油报警
			}
		}
	}
	else
	{
		//偷油比例比实际油量比例大
		if(sAccOffBaseOilPercent[id-1] >= serialoil_get_percent(id))
		{
			if((sAccOffBaseOilPercent[id-1] - serialoil_get_percent(id)) >= (sAccOffBaseOilPercent[id-1]/2))
			{
				sys_state_newAlarm(ALARMST_OIL_ABNO);//偷油报警
			}
		}
	}
}
void serialoil_abnormal_accon(unsigned char id)
{
#define SECOND_PER_MINUTE 60
	
	if(serialoil_get_percent(id) == 0)
	{
		sAccOnTimeCount[id-1] = 0;
		return;
	}
	switch(sAccOnTimeCount[id-1])
	{
		case (0*60):
			sAccOnTimeWindowOilPercent[id-1][0] = serialoil_get_percent(id);
			break;
		case (1*60):
		case (2*60):
		case (3*60):
		case (4*60):
		case (5*60):
			sAccOnTimeWindowOilPercent[id-1][sAccOnTimeCount[id-1]/SECOND_PER_MINUTE] = serialoil_get_percent(id);
			if(sAccOnTimeWindowOilPercent[id-1][0] > sAccOnTimeWindowOilPercent[id-1][sAccOnTimeCount[id-1]/SECOND_PER_MINUTE])
			{
				if((sAccOnTimeWindowOilPercent[id-1][0] - sAccOnTimeWindowOilPercent[id-1][sAccOnTimeCount[id-1]/SECOND_PER_MINUTE]) >= sys_GetOilToSteal())
				{
					sys_state_newAlarm(ALARMST_OIL_ABNO);//偷油报警
				}
			}
			break;
		default:
			break;
	}
	sAccOnTimeCount[id-1] ++;
	if(sAccOnTimeCount[id-1] > (5*60))
	{
		sAccOnTimeCount[id-1] = 0;
	}
}
void serialoil_abnormal_check(void)
{	
	if(serialoil_enable_urtfun(ALL) == DISABLE)
	{
		return;
	}
	//偷油
	if(sys_GetOilToSteal())
	{
		// 1.
		if(sys_state_get_acc() == 0)
		{
			memset_t(sAccOnTimeCount, 0, sizeof(sAccOnTimeCount));
			
			if(sys_state_checkAlarm(ALARMST_OIL_ABNO) == 0)
			{
//				//485
//				if(serialoil_enable_urtfun(COM3) == ENABLE)
//				{
//					serialoil_abnormal_accoff(1);
//					serialoil_abnormal_accoff(2);
//					serialoil_abnormal_accoff(3);
//					serialoil_abnormal_accoff(4);
//				}
//				//232
//				else
//				{
					serialoil_abnormal_accoff(1);
//				}
			}
		}
		// 2.
		else
		{
			memset_t(sAccOffBaseOilPercent, 0, sizeof(sAccOffBaseOilPercent));

			if(sys_state_checkAlarm(ALARMST_OIL_ABNO) == 0)
			{
//				//485
//				if(serialoil_enable_urtfun(COM3) == ENABLE)
//				{
//					serialoil_abnormal_accon(1);
//					serialoil_abnormal_accon(2);
//					serialoil_abnormal_accon(3);
//					serialoil_abnormal_accon(4);
//				}
//				//232
//				else
//				{
					serialoil_abnormal_accon(1);
//				}
			}
		}
	}
}

//读取油耗
void serialoil_read_cmd(unsigned char id)
{
	if(serialoil_get_type(JT) == JT)
	{
		serialoil_read_cmd_jt(id);
	}
	if(serialoil_get_type(CR) == CR)
	{
		serialoil_read_cmd_cr(id);
	}
	if(serialoil_get_type(XD) == XD)
	{
		;
	}
}

//轮询发送
void serialoil_poll_send(void)
{
	if(serialoil_enable_urtfun(ALL) == DISABLE)
	{
		return;
	}

//	//485
//	if(serialoil_enable_urtfun(COM3) == ENABLE)
//	{
//		switch(DevPollSendTime)
//		{
//			case DEV_OIL_CH1_TIME:
//				serialoil_read_cmd(1);
//				break;
//			case DEV_OIL_CH2_TIME:
//				serialoil_read_cmd(2);
//				break;
//			case DEV_OIL_CH3_TIME:
//				serialoil_read_cmd(3);
//				break;
//			case DEV_OIL_CH4_TIME:
//				serialoil_read_cmd(4);
//				break;
//			default:
//				break;
//		}
//	}
//	//232
//	else
//	{
		if(DevPollSendTime)
		{
			if((DevPollSendTime%DEV_OIL_CH1_TIME) == 0)
			{
				serialoil_read_cmd(1);
			}
		}
//	}
}

//返回数据处理函数
void serialoil_handle(unsigned char com_type, unsigned char* data, unsigned int len)
{
	if(serialoil_get_type(JT) == JT)
	{
		serialoil_handle_jt(com_type, data, len);
	}
	if(serialoil_get_type(CR) == CR)
	{
		serialoil_handle_cr(com_type, data, len);
	}
	if(serialoil_get_type(XD) == XD)
	{
		serialoil_handle_xd(com_type, data, len);
	}
}

//串口设置
void serialoil_uart_set(unsigned char *data, unsigned char *pOut)
{
	if(serialoil_get_type(JT) == JT)
	{
		serialoil_uart_set_jt(data, pOut);
	}
	if(serialoil_get_type(CR) == CR)
	{
		serialoil_uart_set_cr(data, pOut);
	}
	if(serialoil_get_type(XD) == XD)
	{
		;
	}
}

//获取油耗值
unsigned short serialoil_get_percent(unsigned char id)
{
	// 2路232油耗同时开启
	if((serialoil_enable_urtfun(COM0) == ENABLE) && \
		(serialoil_enable_urtfun(COM4) == ENABLE))
	{
		if(id == 1)
		{
			if((sys_GetUartFunc(0)&URT_MASK_JT_OIL) == URT_MASK_JT_OIL)
			{
				return serialoil_get_percent_jt(id);
			}
			if((sys_GetUartFunc(0)&URT_MASK_CR_OIL) == URT_MASK_CR_OIL)
			{
				return serialoil_get_percent_cr(id);
			}
			if((sys_GetUartFunc(0)&URT_MASK_XD_OIL) == URT_MASK_XD_OIL)
			{
				return serialoil_get_percent_xd(id);
			}
		}
		if(id == 2)
		{
			if((sys_GetUartFunc(1)&URT_MASK_JT_OIL) == URT_MASK_JT_OIL)
			{
				return serialoil_get_percent_jt(id);
			}
			if((sys_GetUartFunc(1)&URT_MASK_CR_OIL) == URT_MASK_CR_OIL)
			{
				return serialoil_get_percent_cr(id);
			}
			if((sys_GetUartFunc(1)&URT_MASK_XD_OIL) == URT_MASK_XD_OIL)
			{
				return serialoil_get_percent_xd(id);
			}
		}
	}
	else
	{
		if(serialoil_get_type(JT) == JT)
		{
			return serialoil_get_percent_jt(id);
		}
		if(serialoil_get_type(CR) == CR)
		{
			return serialoil_get_percent_cr(id);
		}
		if(serialoil_get_type(XD) == XD)
		{
			return serialoil_get_percent_xd(id);
		}
	}

	return 0xFFFF;
}

