#ifndef __INCLUDES_H__
#define __INCLUDES_H__

#include "gd32f4xx.h"
#include <stdio.h>
#include <string.h>

#include "FreeRTOS.h"  
#include "task.h"  
#include "queue.h"
#include "semphr.h"

#include "main.h"
#include "usr_app.h"
#include "usr_public.h"
#include "usr_gpio.h"
#include "usr_timer.h"
#include "usr_flash.h"
#include "usr_gps.h"
#include "usr_gsm.h"
#include "usr_mile.h"
#include "usr_rtc.h"
#include "usr_aw.h"
#include "usr_msg.h"
#include "usr_dev.h"
#include "usr_sys.h"
#include "usr_sms.h"
#include "usr_fota.h"
#include "usr_handset.h"
#include "usr_usb.h"
#include "usr_ring_buffer.h"
#include "usr_softdog.h"
#include "usr_stdlib.h"
#include "usr_printf_stdarg.h"
#include "usr_comip.h"
#include "usr_ibutton.h"
#include "usr_serialoil.h"
#include "usr_sleep.h"
#include "usr_atv.h"


#include "bsp_adc.h"
#include "bsp_dwt.h"
#include "bsp_gpio.h"
#include "bsp_iap.h"
#include "bsp_iwdg.h"
#include "bsp_nvic.h"
#include "bsp_spi.h"
#include "bsp_timer.h"
#include "bsp_uart.h"

#include "drv_usb_hw.h"
#include "cdc_acm_core.h"

#endif
