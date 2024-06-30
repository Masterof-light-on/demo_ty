#include "includes.h"

const static unsigned int io_pin[17]=
{
	GPIO_PIN_0,	GPIO_PIN_1,GPIO_PIN_2,GPIO_PIN_3,GPIO_PIN_4,\
	GPIO_PIN_5,	GPIO_PIN_6,GPIO_PIN_7,GPIO_PIN_8,GPIO_PIN_9,\
	GPIO_PIN_10,GPIO_PIN_11,GPIO_PIN_12,GPIO_PIN_13,GPIO_PIN_14,\
	GPIO_PIN_15,GPIO_PIN_ALL
};
const static unsigned int gpio_type[7]=
{
	GPIOA,GPIOB,GPIOC,\
	GPIOD,GPIOE,GPIOF,\
	GPIOG
};
const static unsigned int rcu_type[7]=
{
	RCU_GPIOA,RCU_GPIOB,RCU_GPIOC,\
	RCU_GPIOD,RCU_GPIOE,RCU_GPIOF,\
	RCU_GPIOG
};

/*
	pin: PA2、PD12表示:102,412
	dir: 0:输入 1:输出
*/
void gpio_dir(unsigned short pin,unsigned char dir)
{
	unsigned short GPIOPORT = 0,GPIOPOS = 0;

	GPIOPORT = pin/100;
	GPIOPOS  = pin%100;
	
	if(dir == 1)
	{
		rcu_periph_clock_enable((rcu_periph_enum)rcu_type[GPIOPORT-1]);
		gpio_mode_set(gpio_type[GPIOPORT-1], GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, io_pin[GPIOPOS]);
    	gpio_output_options_set(gpio_type[GPIOPORT-1], GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, io_pin[GPIOPOS]);
	}
	else
	{
		rcu_periph_clock_enable((rcu_periph_enum)rcu_type[GPIOPORT-1]);
		gpio_mode_set(gpio_type[GPIOPORT-1], GPIO_MODE_INPUT, GPIO_PUPD_NONE, io_pin[GPIOPOS]);
	}
}

/*
	pin: PA2、PD12表示:102,412
	val: 0:低电平 1:高电平
*/
void gpio_set(unsigned short pin,unsigned char val)
{
	unsigned char GPIOPORT = 0,GPIOPOS = 0;

	GPIOPORT = pin/100;
	GPIOPOS  = pin%100;

	if(val == 1)
	{
		gpio_bit_set(gpio_type[GPIOPORT-1], io_pin[GPIOPOS]);
	}
	else
	{
		gpio_bit_reset(gpio_type[GPIOPORT-1], io_pin[GPIOPOS]);
	}
}

/*
	pin: PA2、PD12表示:102,412
*/
unsigned char gpio_val(unsigned short pin)
{
	unsigned char GPIOPORT = 0,GPIOPOS = 0;

	GPIOPORT = pin/100;
	GPIOPOS  = pin%100;
	
	return gpio_input_bit_get(gpio_type[GPIOPORT-1], io_pin[GPIOPOS]);
}


/*
	pin: PA2、PD12表示:102,412
*/
unsigned char gpio_valo(unsigned short pin)
{
	unsigned char GPIOPORT = 0,GPIOPOS = 0;

	GPIOPORT = pin/100;
	GPIOPOS  = pin%100;
	
	return gpio_output_bit_get(gpio_type[GPIOPORT-1], io_pin[GPIOPOS]);
}

