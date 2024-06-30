#ifndef __USR_IBUTTON_H__
#define __USR_IBUTTON_H__

#define TM_Read_ROM   0x33
#define TM_Search_ROM 0xF0
#define TM_Match_ROM  0x55
#define TM_Skip_ROM   0xCC

#define TM_PORT   GPIO_PIN_1

#define ibottom_low		gpio_bit_reset(GPIOD, TM_PORT)//PORT_ResetBits(PortA,Pin09)
#define ibottom_high	gpio_bit_set(GPIOD, TM_PORT)//PORT_SetBits(PortA,Pin09)
#define ibottom_input	gpio_input_bit_get(GPIOD, TM_PORT)//PORT_GetBit(PortA,Pin09)


typedef enum
{
	Bit_RESET = 0,
    Bit_SET
}BitAction;
typedef struct
{
    unsigned char Family_Code;
    unsigned char Serial_Number[6];
    unsigned char crc;

}IBottom_Typedef;

extern unsigned char Byte_buf[8];

void TM_GPIO_Configuration(void);
void data_output_set(void);
void data_input_set(void);
void TMSN_ReadROM(void);
unsigned char ibottom_init(void);
void TM_Send_Byte(unsigned char dat);
unsigned char TM_Read_Byte(void);
unsigned char Data_Crc(unsigned char indata,unsigned char crc);
void rfid_timeout(void);
#endif

