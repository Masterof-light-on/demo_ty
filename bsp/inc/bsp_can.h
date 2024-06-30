#ifndef __BSP_CAN_H__
#define __BSP_CAN_H__

void canx_init(void);
void can_SetFilter32IdList(unsigned char FilterNum, unsigned int FilterId, unsigned int FilterMask);
void can_SetFilter16IdList(unsigned char FilterNum, unsigned short FilterId1, unsigned short FilterId2, unsigned short FilterMask1, unsigned short FilterMask2);
void can_SetFilter32IdMask(unsigned char FilterNum, unsigned int FilterId, unsigned int FilterMask);
void can_SetFilter16IdMask(unsigned char FilterNum, unsigned short FilterId1, unsigned short FilterMask1, unsigned short FilterId2, unsigned short FilterMask2);
void can_SetFilterActivation(unsigned int can_periph, ControlStatus state);
void can_SetRecvFilter(void);
int can_SendData(unsigned int can_periph, unsigned int ID, unsigned int IDE, unsigned char *buf, unsigned char len);

#endif
