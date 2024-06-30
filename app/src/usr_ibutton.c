#include "includes.h"

unsigned char Byte_buf[8]={0};
IBottom_Typedef ibottom_sn={0};

/*****************************************************************************
函 数 名: TM_GPIO_Configuration
功能描述: TM钥匙的GPIO配置
输入输出: 
修改记录：
*****************************************************************************/
void TM_GPIO_Configuration(void)
{
	rcu_periph_clock_enable(RCU_GPIOD);
	gpio_mode_set(GPIOD, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, TM_PORT);
    gpio_output_options_set(GPIOD, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, TM_PORT);

	ibottom_high;
}
/*****************************************************************************
函 数 名: data_input_set
功能描述: TM钥匙的GPIO配置
输入输出: 
修改记录：
*****************************************************************************/
void data_output_set(void)
{
	gpio_mode_set(GPIOD, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, TM_PORT);
    gpio_output_options_set(GPIOD, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, TM_PORT);
}
/*****************************************************************************
函 数 名: data_input_set
功能描述: TM钥匙的GPIO配置
输入输出: 
修改记录：
*****************************************************************************/
void data_input_set(void)
{
	gpio_mode_set(GPIOD, GPIO_MODE_INPUT, GPIO_PUPD_NONE, TM_PORT);
}

/*****************************************************************************
函 数 名: TMSN_ReadROM
功能描述: 
输入输出: 操作命令 0x33 高位在前
修改记录：
*****************************************************************************/

void TMSN_ReadROM(void)
{
	unsigned char crc=0;
	unsigned char i;
    if(ibottom_init()==TRUE)//检测到有TM钥匙接触到探头
    {
		g_flag.bit.RfidLink =  1;
		rfidtimeout = 3;
        TM_Send_Byte(TM_Read_ROM);
        for(i=0;i<7;i++)
        {
            Byte_buf[i] = TM_Read_Byte();
            crc=Data_Crc(Byte_buf[i],crc);
        }
        Byte_buf[7] = TM_Read_Byte();
        if(Byte_buf[7] == crc)//校验合格
        {
            ibottom_sn.Family_Code = Byte_buf[0];
            ibottom_sn.crc = Byte_buf[7];
            for(i=0;i<6;i++)
            {
                ibottom_sn.Serial_Number[i] = Byte_buf[6-i];
            }
			if(g_flag.bit.rfidbuf == 0)
			{
				OS_SendMessage(xMainQueueId, MSG_MAIN_ALARM_INDICATION, 3, 0);
			}
			g_flag.bit.rfidbuf =  1;
			
        }
    }
}
/*****************************************************************************
函 数 名: ibottom_init
功能描述: 初始化 所有的操作前需要初始化
输入输出: 
修改记录：
*****************************************************************************/
unsigned char ibottom_init(void)
{
    unsigned short ucErrTime = 0;
    data_output_set();
    ibottom_low;
//    delay_us(500);//≥480us 的低电压去复位DS1990A
	delay_us(760);//≥480us 的低电压去复位DS1990A
    ibottom_high;//拉高
    delay_us(27);

    data_input_set();//切换成RX 电阻拉高后
    //检测从机应答ACK 等待最长60us时间后去检测总线是否为低电平了，去检测点的时间要掐在总线被电阻拉高后的60~75us之间
    while(ibottom_input)
    {
        ucErrTime++;
        if(ucErrTime > 500)
        {
            return  FALSE;
        }
        delay_us(1);
    }
    delayus(300);//等待应答结束
    data_output_set();
    return TRUE;
}
/*****************************************************************************
函 数 名: TM_Send_Byte
功能描述: 
输入输出: 操作命令 0x33 低位在前 先发低位
修改记录：
*****************************************************************************/
void TM_Send_Byte(unsigned char dat)
{
	unsigned char t;
    data_output_set();
    for(t=0;t<8;t++)//0X33  0011 0011
	{
        if((dat&0x01))//发1
        {
            ibottom_low;
            delay_us(14);//低电平小于15-ε δ us 表示1 
            ibottom_high;
            delay_us(70);//一个bit时间段 205us
        }
        else//发0
        {
            ibottom_low;
            delay_us(72);//低电平在60~120us 之间表示0
            ibottom_high;
            delay_us(12);
        }
        dat>>=1; 
	}
}
/*****************************************************************************
函 数 名: TM_Send_Byte
功能描述: 
输入输出: 操作命令 0x33 低位在前  先读低位
修改记录：放在main主函数轮询即可
*****************************************************************************/
unsigned char TM_Read_Byte(void)
{
	unsigned char i;
    unsigned char receive = 0;
    for(i=0;i<8;i++)
    {
        data_output_set();
        ibottom_low;
        delay_us(2);
        data_input_set();
        delay_us(1);
        receive>>=1;
        if(ibottom_input)//采样点检测到高电平 
        {
			receive|=0x80;
        }
        delay_us(80);
    }
    data_output_set();
    return receive;
}

/*****************************************************************************
函 数 名: TM_Send_Byte
功能描述: 
输入输出: 操作命令 0x33 低位在前  先读低位
修改记录：
*****************************************************************************/
unsigned char Data_Crc(unsigned char indata,unsigned char crc)
{
	unsigned char i,b = 0;
     for (i=8;i>0;--i)
     {
		if ( b == (indata^crc)&0x01 )     //判断与x7异或的结果(x8)
			crc^=0x18;                  //反馈到x5 x4
		crc>>=1;                            //移位
		if (b)
			crc|=0x80;                  //x7异或的结果送x0
		indata>>=1;
     }
     return(crc);
}

void rfid_timeout(void)
{
	if(rfidtimeout)
	{
		rfidtimeout--;
		if(0 == rfidtimeout)
		{
			memset_t(Byte_buf,0,sizeof(Byte_buf));
			g_flag.bit.RfidLink =  0;
			OS_SendMessage(xMainQueueId, MSG_MAIN_ALARM_INDICATION, 0, 0);
		}
	}
	
}
