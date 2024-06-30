#include "includes.h"

usb_core_driver cdc_acm;

void usb_init(void)
{
	usb_gpio_config();
    usb_rcu_config();

    usbd_init (&cdc_acm,USB_CORE_ENUM_FS,&cdc_desc,&cdc_class);

    /* enable interrupts */
    usb_intr_config();
}

void sendcharusb(unsigned char ch)
{
	if (USBD_CONFIGURED != cdc_acm.dev.cur_status) return;
	cdc_acm_data_send1(&cdc_acm,&ch, 1);
}

void printusb(unsigned char *str,int len)
{
	if (USBD_CONFIGURED != cdc_acm.dev.cur_status) return;
	cdc_acm_data_send1(&cdc_acm,str, len);
}

//void USB_Printf(const char *format, ...)
//{
//	char oledPrinterBuff[128];
//	char *pOledBuff = oledPrinterBuff;
//	va_list pList;	
//	if (USBD_CONFIGURED != cdc_acm.dev.cur_status) return;
//	va_start(pList,format);
//	vsprintf(oledPrinterBuff,format,pList);
//	va_end(pList);
//	printusb((unsigned char *)pOledBuff,strlen_t(pOledBuff));
//}

void usb_recv(unsigned char* data, unsigned int len)
{
	RingBuffer_InsertMult(&USBRINGBUFF, (unsigned char *)data, len);
	OS_SendMessageFromISR(xDevQueueId, MSG_DEV_COM_USB_REQ, USBCOM, 0);
}

