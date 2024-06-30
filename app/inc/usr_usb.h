#ifndef __USR_USB_H__
#define __USR_USB_H__

void usb_init(void);
void usb_recv(unsigned char* data, unsigned int len);
void sendcharusb(unsigned char ch);
void printusb(unsigned char *str,int len);

#endif

