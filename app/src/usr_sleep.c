#include "includes.h"

#define RTC_CLOCK_SOURCE_LXTAL
#define BKP_VALUE    0x32F0

rtc_parameter_struct   rtc_initpara;
rtc_alarm_struct  rtc_alarm;
__IO uint32_t prescaler_a = 0, prescaler_s = 0;
unsigned int sleeptimeinterval= 0 ;
static unsigned int work_time = 0; 
/*!
    \brief      use hyperterminal to setup RTC time and alarm
    \param[in]  none
    \param[out] none
    \retval     none
*/
void rtc_setup(void)
{
    /* setup RTC time value */
    rtc_initpara.factor_asyn = prescaler_a;
    rtc_initpara.factor_syn = prescaler_s;
    rtc_initpara.year = 0x16;
    rtc_initpara.day_of_week = RTC_SATURDAY;
    rtc_initpara.month = RTC_APR;
    rtc_initpara.date = 0x30;
    rtc_initpara.display_format = RTC_24HOUR;
    rtc_initpara.am_pm = RTC_AM;
    /* current time input */
    rtc_initpara.hour = 23;
    rtc_initpara.minute = 59;
    rtc_initpara.second = 59;
    /* RTC current time configuration */
    if(ERROR == rtc_init(&rtc_initpara)){
    }else{
        RTC_BKP0 = BKP_VALUE;
    }
    /* setup RTC alarm */
    rtc_alarm_disable(RTC_ALARM0);
    rtc_alarm.alarm_mask = RTC_ALARM_ALL_MASK;
    rtc_alarm.weekday_or_date = RTC_ALARM_DATE_SELECTED;
    rtc_alarm.alarm_day = 0x31;
    rtc_alarm.am_pm = RTC_AM;

    /* RTC alarm input */
    rtc_alarm.alarm_hour = 23;
    rtc_alarm.alarm_minute = 59;
    rtc_alarm.alarm_second = 59;
    /* RTC alarm configuration */
    rtc_alarm_config(RTC_ALARM0,&rtc_alarm);
    rtc_interrupt_enable(RTC_INT_ALARM0);
    rtc_alarm_enable(RTC_ALARM0);
}

void rtc_AlarmInit(void)  
{  
	/* enable PMU clock */
    rcu_periph_clock_enable(RCU_PMU);
    /* enable the access of the RTC registers */
    pmu_backup_write_enable();

    #if defined (RTC_CLOCK_SOURCE_IRC32K)
          rcu_osci_on(RCU_IRC32K);
          rcu_osci_stab_wait(RCU_IRC32K);
          rcu_rtc_clock_config(RCU_RTCSRC_IRC32K);

          prescaler_s = 0x13F;
          prescaler_a = 0x63;
    #elif defined (RTC_CLOCK_SOURCE_LXTAL)
          rcu_osci_on(RCU_LXTAL);
          rcu_osci_stab_wait(RCU_LXTAL);
          rcu_rtc_clock_config(RCU_RTCSRC_LXTAL);

          prescaler_s = 0xFF;
          prescaler_a = 0x7F;
    #else
    #error RTC clock source should be defined.
    #endif /* RTC_CLOCK_SOURCE_IRC32K */

    rcu_periph_clock_enable(RCU_RTC);
    rtc_register_sync_wait();
    /* check if RTC has aready been configured */
    if (BKP_VALUE != RTC_BKP0){
        rtc_setup();
    }else{
        /* detect the reset source */
        if (RESET != rcu_flag_get(RCU_FLAG_PORRST)){

        }else if (RESET != rcu_flag_get(RCU_FLAG_EPRST)){

        }

    }

    rcu_all_reset_flag_clear();
    rtc_flag_clear(RTC_FLAG_ALRM0);
    exti_flag_clear(EXTI_17);
    /* RTC alarm interrupt configuration */
    exti_init(EXTI_17,EXTI_INTERRUPT,EXTI_TRIG_RISING);
    nvic_irq_enable(RTC_Alarm_IRQn,0,0);
}

/*!
    \brief      this function handles RTC interrupt request
    \param[in]  none
    \param[out] none
    \retval     none
*/
void RTC_Alarm_IRQHandler(void)
{
    if(RESET != rtc_flag_get(RTC_FLAG_ALRM0)){
        rtc_flag_clear(RTC_FLAG_ALRM0);
        exti_flag_clear(EXTI_17);
		sleeptimeinterval++;
		gpio_watchDog();
    } 
}
/*PA4 主电IO*/
void sleep_WakeupFromStopMode(void)
{
	//RTC唤醒CPU喂狗
	nvic_irq_enable(RTC_Alarm_IRQn,0,0);
	gpio_watchDog();
	gpio_dir(ACC,0);
	delayms(1);
	//唤醒ACC开，休眠时间到
	if((gpio_val(ACC) == 0) || (sleeptimeinterval >= g_runtime.g_watch.sleepTime))
	{
		sys_SoftReset();		
	}
}

void sleep_IntoStopMode(void)
{	
	LOG("!! Into stop mode...\r\n");
	//关闭定时器
	sys_saveRuning();
	timer_disable(TIMER1);
	//下电
	usart_disable(USART0);
	usart_disable(USART1);
	usart_disable(USART2);
	usart_disable(UART4);// no use
	usart_disable(UART3);//
	gpio_watchDog();
	//关闭LED
	gpio_bt_powerCtrl(0);
	gpio_Gps_powerCtrl(0);
	gpio_Gsm_powerCtrl(0);

//	gpio_modemLedCtrl(0);
	
	gpio_mode_set(GPIOA, GPIO_MODE_ANALOG, GPIO_PUPD_NONE, GPIO_PIN_ALL);
	
	gpio_mode_set(GPIOA, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO_PIN_2);
	gpio_output_options_set(GPIOA, GPIO_OTYPE_OD, GPIO_OSPEED_2MHZ, GPIO_PIN_2);
	gpio_mode_set(GPIOA, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO_PIN_3);
	gpio_output_options_set(GPIOA, GPIO_OTYPE_OD, GPIO_OSPEED_2MHZ, GPIO_PIN_3);
	gpio_mode_set(GPIOA, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO_PIN_9);
	gpio_output_options_set(GPIOA, GPIO_OTYPE_OD, GPIO_OSPEED_2MHZ, GPIO_PIN_9);
	gpio_mode_set(GPIOA, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO_PIN_10);
	gpio_output_options_set(GPIOA, GPIO_OTYPE_OD, GPIO_OSPEED_2MHZ, GPIO_PIN_10);
	
	gpio_bit_reset(GPIOA, GPIO_PIN_0);
	gpio_bit_reset(GPIOA, GPIO_PIN_1);
	gpio_bit_reset(GPIOA, GPIO_PIN_2);
	gpio_bit_reset(GPIOA, GPIO_PIN_3);
	gpio_bit_reset(GPIOA, GPIO_PIN_9);
	gpio_bit_reset(GPIOA, GPIO_PIN_10);
	

	gpio_mode_set(GPIOB, GPIO_MODE_ANALOG, GPIO_PUPD_NONE, GPIO_PIN_ALL);
	
	gpio_mode_set(GPIOB, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO_PIN_10);
	gpio_output_options_set(GPIOB, GPIO_OTYPE_OD, GPIO_OSPEED_2MHZ, GPIO_PIN_10);
	gpio_mode_set(GPIOB, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO_PIN_11);
	gpio_output_options_set(GPIOB, GPIO_OTYPE_OD, GPIO_OSPEED_2MHZ, GPIO_PIN_11);
	gpio_bit_reset(GPIOB, GPIO_PIN_10);
	gpio_bit_reset(GPIOB, GPIO_PIN_11);
	
	
	gpio_mode_set(GPIOC, GPIO_MODE_ANALOG, GPIO_PUPD_NONE, GPIO_PIN_ALL);
	
	gpio_mode_set(GPIOC, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO_PIN_10);
	gpio_output_options_set(GPIOC, GPIO_OTYPE_OD, GPIO_OSPEED_2MHZ, GPIO_PIN_10);
	gpio_mode_set(GPIOC, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO_PIN_11);
	gpio_output_options_set(GPIOC, GPIO_OTYPE_OD, GPIO_OSPEED_2MHZ, GPIO_PIN_11);
	gpio_bit_reset(GPIOC, GPIO_PIN_10);
	gpio_bit_reset(GPIOC, GPIO_PIN_11);
	
	gpio_mode_set(GPIOC, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO_PIN_12);
	gpio_output_options_set(GPIOC, GPIO_OTYPE_OD, GPIO_OSPEED_2MHZ, GPIO_PIN_12);
	gpio_bit_reset(GPIOC, GPIO_PIN_12);
	
	gpio_mode_set(GPIOD, GPIO_MODE_ANALOG, GPIO_PUPD_NONE, GPIO_PIN_ALL);

	gpio_mode_set(GPIOD, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO_PIN_2);
	gpio_output_options_set(GPIOD, GPIO_OTYPE_OD, GPIO_OSPEED_2MHZ, GPIO_PIN_2);
	gpio_bit_reset(GPIOD, GPIO_PIN_2);


	gpio_mode_set(GPIOE, GPIO_MODE_ANALOG, GPIO_PUPD_NONE, GPIO_PIN_ALL);
	
	portENTER_CRITICAL();
	//关闭滴答定时器
	SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;
	portEXIT_CRITICAL();
	
	//挂起所有的任务
	rtc_AlarmInit();
	
	while(1)
	{	
		//进入睡眠模式
		//rcu_periph_clock_enable(RCU_PMU);
		//pmu_to_sleepmode(WFI_CMD);
		//进入停止模式
		rcu_periph_clock_enable(RCU_PMU);
		pmu_to_deepsleepmode(PMU_LDO_NORMAL, WFI_CMD);  
		sleep_WakeupFromStopMode();
	}
}

//1s进一次
void sys_SleepCheck(void)
{

	
	//不休眠
	if((!sys_GetSleepAble()) || sys_state_get_acc() || (modem_GetState() == GSM_FOTA_STATE))
	{
		work_time = 0; 
		return;
	}

	/* 23*60*60+59*60+30 = 82800+3540+30 = 86370*/
	LOG("Enter sleep [%d] %d\r\n",g_runtime.g_watch.nsleepInterval,work_time);
	work_time++;
	if(work_time >= g_runtime.g_watch.nsleepInterval)
	{
		work_time = 0;
		sleep_IntoStopMode();
	}
}

