#ifndef __USR_GPIO_H__
#define __USR_GPIO_H__

#define GSM_POW			PD14
#define GSM_WAKEUP		PC0

#define GPS_POW			PD6//PA0
#define BT_POW			PA1

#define R232_POW		PC7
#define CAN_POW			PC8
#define R485_POW		PC9

#define IB5V_POW		PA8

#define LIN				PD1
#define LOUT			PD0

#define SOS				PD7
#define ACC				PA0//PD6
#define DIN1			PB7
#define DIN2			PB6
#define DIN3			PB5

#define DOUT1			PD5
#define DOUT2			PD4
#define DOUT3			PD3

#define GPS_LED			PE10
#define GSM_LED			PE9
#define BT_LED			PE8

#define WDI			PC6

typedef enum
{
	GPIO_ACC_ST 		= 0x01,
	GPIO_SOS_ST 		= 0x02,
	GPIO_DI1_ST 		= 0x04,
	GPIO_DI2_ST 		= 0x08,
	GPIO_DI3_ST 		= 0x10,
	GPIO_DO1_ST 		= 0x20,
	GPIO_DO2_ST 		= 0x40,
	GPIO_DO3_ST 		= 0x80,
}eGPIO_STATUS;


void gpioInit(void);
void gpio_232_powerCtrl(unsigned char on);
void gpio_bt_powerCtrl(unsigned char on);
void gpio_Gsm_powerCtrl(unsigned char on);
void gpio_Gps_powerCtrl(unsigned char on);

void gpio_led_proc(void);
void gpio_check(void);
void gpio_handle(unsigned int st);
void gpio_watchDog(void);


#endif
