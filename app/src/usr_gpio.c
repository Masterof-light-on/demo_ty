#include "includes.h"

void gpioInit(void)
{
	gpio_dir(GSM_POW, 1);
	gpio_set(GSM_POW, 0);

	gpio_dir(GSM_WAKEUP, 1);
	gpio_set(GSM_WAKEUP, 0);
	
	gpio_dir(GPS_POW, 1);
	gpio_set(GPS_POW, 0);

	gpio_dir(BT_POW, 1);
	gpio_set(BT_POW, 1);
	
	gpio_dir(R232_POW, 1);
	gpio_set(R232_POW, 1);
	
	gpio_dir(CAN_POW, 1);
	gpio_set(CAN_POW, 0);
	
	gpio_dir(R485_POW, 1);
	gpio_set(R485_POW, 1);
	
	gpio_dir(DOUT1, 1);
	gpio_set(DOUT1, 0);
	gpio_dir(DOUT2, 1);
	gpio_set(DOUT2, 0);
	gpio_dir(DOUT3, 1);
	gpio_set(DOUT3, 0);
	
	gpio_dir(GPS_LED, 1);
	gpio_set(GPS_LED, 0);
	gpio_dir(GSM_LED, 1);
	gpio_set(GSM_LED, 0);
	gpio_dir(BT_LED, 1);
	gpio_set(BT_LED, 0);
	
//	TM_GPIO_Configuration();
	gpio_dir(IB5V_POW, 1);
	gpio_set(IB5V_POW, 1);
	gpio_dir(LOUT, 1);
	gpio_set(LOUT, 0);
	
//	gpio_dir(LOUT, 0);
//	gpio_dir(LIN, 0);

	gpio_dir(SOS, 0);
	gpio_dir(ACC, 0);
	gpio_dir(DIN1, 0);
	gpio_dir(DIN2, 0);
	gpio_dir(DIN3, 0);

	gpio_dir(WDI, 1);
}

void gpio_232_powerCtrl(unsigned char on)
{
	(on == 0)?gpio_set(R232_POW,0):gpio_set(R232_POW,1);
}

void gpio_bt_powerCtrl(unsigned char on)
{
	(on == 1)?gpio_set(BT_POW,0):gpio_set(BT_POW,1);
}


void gpio_Gps_powerCtrl(unsigned char on)
{
	(on == 0)?gpio_set(GPS_POW,0):gpio_set(GPS_POW,1);
}


void gpio_Gsm_powerCtrl(unsigned char on)
{
	(on == 0)?gpio_set(GSM_POW,0):gpio_set(GSM_POW,1);
}

//led
void gpio_modemLedCtrl(unsigned char on)
{
	(on == 0)?gpio_set(GSM_LED,0):gpio_set(GSM_LED,1);
}

//led闪烁处理
void gpio_modemLed_proc(void)
{
	static unsigned char gsm_init=0;
	static unsigned char gsm_connect=0;

	//上线
	if(modem_GetState() == GSM_ONLINE_STATE)
	{
		if(gsm_connect == 0)
		{
			gpio_modemLedCtrl(1);
			gsm_connect = 1;
		}
		else
		{
			gpio_modemLedCtrl(0);
			if(gsm_connect++ > 10)
			{
				gsm_connect = 0;
			}
		}
		return;
	}
	//搜网
	if((modem_GetCgreg() == 1)||(modem_GetCgreg() == 5))
	{
		if(gsm_connect == 0)
		{
			gpio_modemLedCtrl(1);
			gsm_connect = 1;
		}
		else
		{
			gpio_modemLedCtrl(0);
			if(gsm_connect++ > 4)
			{
				gsm_connect = 0;
			}
		}
	}
	else
	{
		if(gsm_init == 0)
		{
			gpio_modemLedCtrl(1);
			gsm_init = 1;
		}
		else
		{
			gpio_modemLedCtrl(0);
			gsm_init = 0;
		}
	}
}
//gps led
void gpio_gpsLedCtrl(unsigned char on)
{
	(on == 0)?gpio_set(GPS_LED,0):gpio_set(GPS_LED,1);
}
//gps闪烁处理
void gpio_gpsLed_proc(void)
{
	static unsigned char gps_flag=0;
	static unsigned char gsm_connect=0;
	if(sys_state_get_gpsloc())//定位
	{
		gps_flag = 0;

		if(gsm_connect == 0)
		{
			gpio_gpsLedCtrl(1);
			gsm_connect = 1;
		}
		else
		{
			gpio_gpsLedCtrl(0);
			if(gsm_connect++ > 10)
			{
				gsm_connect = 0;
			}
		}
		return;
	}
	gsm_connect = 0;
	gps_flag ^= 1;
	gpio_gpsLedCtrl(gps_flag);
}
//bt led
void gpio_btLedCtrl(unsigned char on)
{
	(on == 0)?gpio_set(BT_LED,0):gpio_set(BT_LED,1);
}
//bt闪烁处理
void gpio_btLed_proc(void)
{
	static unsigned char bt_connect=0;

	if(bt_connect == 0)
	{
		gpio_btLedCtrl(1);
		bt_connect = 1;
	}
	else
	{
		gpio_btLedCtrl(0);
		if(bt_connect++ > 10)
		{
			bt_connect = 0;
		}
	}
}

//led闪烁处理
void gpio_led_proc(void)
{
	gpio_modemLed_proc();
	gpio_gpsLed_proc();
	gpio_btLed_proc();
}


//gpio检测
unsigned int gpio_ReadPinStatus(void)
{
	unsigned int st=0;
	//ACC
	if(gpio_val(ACC) == 0)
	{
		st |= GPIO_ACC_ST;
	}
	if(gpio_val(SOS))
	{
		st |= GPIO_SOS_ST;
	}
	if(gpio_val(DIN1))
	{
		st |= GPIO_DI1_ST;
	}
	if(gpio_val(DIN2) == 0)
	{
		st |= GPIO_DI2_ST;
	}
	if(gpio_val(DIN3))
	{
		st |= GPIO_DI3_ST;
	}
	if(gpio_valo(DOUT1))
	{
		st |= GPIO_DO1_ST;
	}
	if(gpio_valo(DOUT2) == 0)
	{
		st |= GPIO_DO2_ST;
	}
	if(gpio_valo(DOUT3))
	{
		st |= GPIO_DO3_ST;
	}
	return st;
}

void gpio_check(void)
{
	static unsigned int sTimerGpioCurSt=0,sTimerGpioLastSt=0,sTimerGpioSt=0;
	static unsigned char sTimerGpioDebounce=0;
	
	sTimerGpioCurSt = gpio_ReadPinStatus();
	if(sTimerGpioSt != sTimerGpioCurSt)
	{
		sTimerGpioSt = sTimerGpioCurSt;
		sTimerGpioDebounce = 0;
	}
	else
	{
		//防抖动时间800ms
		if(++sTimerGpioDebounce >= 8)
		{
			sTimerGpioDebounce = 0;
			if(sTimerGpioLastSt != sTimerGpioCurSt)
			{
				sTimerGpioLastSt = sTimerGpioCurSt;
				OS_SendMessageFromISR(xMainQueueId, MSG_MAIN_GPIO_REQ, sTimerGpioLastSt, 0);
			}
		}
	}
}
//IO口处理
void gpio_handle(unsigned int st)
{
	unsigned char ret = 0;
	ret = ((st & GPIO_ACC_ST) ? 1:0);
	sys_state_acc9(ret);//acc
	ret = ((st & GPIO_SOS_ST) ? 1:0);
	sys_state_sos9(ret);//acc
	ret = ((st & GPIO_DI1_ST) ? 1:0);
	sys_state_di19(ret);//acc
	ret = ((st & GPIO_DI2_ST) ? 1:0);
	sys_state_di29(ret);//acc
	ret = ((st & GPIO_DI3_ST) ? 1:0);
	sys_state_di39(ret);//acc
	
	ret = ((st & GPIO_DO1_ST) ? 1:0);
	sys_state_do19(ret);//acc
	ret = ((st & GPIO_DO2_ST) ? 1:0);
	sys_state_do29(ret);//acc
	ret = ((st & GPIO_DO3_ST) ? 1:0);
	sys_state_do39(ret);//acc
}

void gpio_watchDog(void)
{
	static unsigned char wdi = 1;

	if(gc_watchdog == 1)//硬件看门狗测试
	{
		iwdg_feed();
	}
	else
	{
		iwdg_feed();
		gpio_set(WDI,wdi);
		wdi ^= 1;
	}
}

