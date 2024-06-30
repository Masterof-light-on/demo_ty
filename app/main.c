/*!
    \file    main.c
    \brief   led spark with systick
    
    \version 2016-08-15, V1.0.0, firmware for GD32F4xx
    \version 2018-12-12, V2.0.0, firmware for GD32F4xx
    \version 2020-09-30, V2.1.0, firmware for GD32F4xx
*/


#include "includes.h"

//�������ȼ�
#define Main_TASK_PRIORITY 				( tskIDLE_PRIORITY + 4 )
#define Gsm_TASK_PRIORITY 				( tskIDLE_PRIORITY + 3 )
#define Gps_TASK_PRIORITY 				( tskIDLE_PRIORITY + 2 )
#define Dev_TASK_PRIORITY 				( tskIDLE_PRIORITY + 1 )

//�����ջ��С
//ջ��С=configMINIMAL_STACK_SIZE*4
#define configTASK_STACK_SIZE_2048		( ( unsigned short ) 2048 )
#define configTASK_STACK_SIZE_1024		( ( unsigned short ) 1024 )
#define configTASK_STACK_SIZE_512		( ( unsigned short ) 512 )
#define configTASK_STACK_SIZE_256		( ( unsigned short ) 256 )
#define configTASK_STACK_SIZE_128		( ( unsigned short ) 128 )

void bsp_init(void)
{
	nvic_init();

	queue_init();

	gpioInit();
	
	DWT_Init();

	log_Init();

	spi_flash_init();

	adc_init();

	timer1_init();

	iwdg_init();

	sys_Init();

	dev_Init();

	gps_Init();

	g_flag.bit.InitFinshFlag = INIT_OK;
}

int main(void)
{
	nvic_vector_table_set(NVIC_VECTTAB_FLASH, 0x8000);
	
	nvic_priority_group_set(NVIC_PRIGROUP_PRE4_SUB0);

    /* ���д��� */
	vMainTaskQueueCreat();
	vGsmTaskQueueCreat();
	vGpsTaskQueueCreat();
	vDevTaskQueueCreat();

	/* �ⲿFlash Lock��ʼ�� */
	NorFlash_LockInit(xFlashMutex);
	/* GSM send Lock��ʼ��*/
	GsmSend_LockInit(xSendMutex);
	/* Msg Lock��ʼ��*/
	Msg_LockInit(xMsgMutex);
	/* Log Lock��ʼ��*/
	Log_LockInit(xLogMutex);

	/* sem��ʼ�� */
	GsmWait_SemInit(xGsmSem);
	SocketWait_SemInit(xSocketSem);

    /* �������� */
    xTaskCreate(vMainTask,	"Main",	  configTASK_STACK_SIZE_1024, NULL, Main_TASK_PRIORITY,   NULL);
	xTaskCreate(vGsmTask,  	"GSM",    configTASK_STACK_SIZE_1024, NULL, Gsm_TASK_PRIORITY,    NULL);
	xTaskCreate(vGpsTask,  	"GPS",    configTASK_STACK_SIZE_1024, NULL, Gps_TASK_PRIORITY,    NULL);
	xTaskCreate(vDevTask, 	"DEV",    configTASK_STACK_SIZE_1024, NULL, Dev_TASK_PRIORITY,    NULL);

	/* ����OSͬʱҲ������������ */
    vTaskStartScheduler();

    for( ;; );
}

void app_PrintVersion(void)
{
	LOG("[version] %s\r\n",APP_VER);
}

void app_GetVersion(unsigned char *buf)
{
	sprintf((char *)buf, "Version:%s  IMEI:%s  CCID:%s FLASH:0x%x", APP_VER,modem_GetImei(),modem_GetCcid(),SPI_Flash_ReadID());
}

