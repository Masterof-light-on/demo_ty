#ifndef __BSP_UART_H__
#define __BSP_UART_H__

#define UART0_MAX_TX		1024
#define UART0_MAX_RX		1024

typedef struct Uart0_Tx          
{
   unsigned char  data[UART0_MAX_TX];
}uart0_tx;

typedef struct Uart0_Rx          
{
   unsigned short len;        
   unsigned char  data[UART0_MAX_RX];
}uart0_rx;

#define UART1_MAX_TX		1024
#define UART1_MAX_RX		1024

typedef struct Uart1_Tx          
{
   unsigned char  data[UART1_MAX_TX];
}uart1_tx;

typedef struct Uart1_Rx          
{
   unsigned short len;        
   unsigned char  data[UART1_MAX_RX];
}uart1_rx;

#define UART2_MAX_TX		1024
#define UART2_MAX_RX		1024

typedef struct Uart2_Tx          
{
   unsigned char  data[UART2_MAX_TX];
}uart2_tx;

typedef struct Uart2_Rx          
{
   unsigned short len;        
   unsigned char  data[UART2_MAX_RX];
}uart2_rx;

#define UART3_MAX_TX		1024
#define UART3_MAX_RX		1024

typedef struct Uart3_Tx          
{
   unsigned char  data[UART3_MAX_TX];
}uart3_tx;

typedef struct Uart3_Rx          
{
   unsigned short len;        
   unsigned char  data[UART3_MAX_RX];
}uart3_rx;

#define UART4_MAX_TX		1024
#define UART4_MAX_RX		1024

typedef struct Uart4_Tx          
{
   unsigned char  data[UART4_MAX_TX];
}uart4_tx;

typedef struct Uart4_Rx          
{
   unsigned short len;        
   unsigned char  data[UART4_MAX_RX];
}uart4_rx;

//232 1
void uart0_init(unsigned int baud);
void uart0_dma(void);
void sendchar0(unsigned char ch);
void print0(unsigned char *str,int len);

//gps
void uart1_init(unsigned int baud);
void uart1_dma(void);
void sendchar1(unsigned char ch);
void print1(unsigned char *str,int len);

//4G
void uart2_init(unsigned int baud);
void uart2_dma(void);
void sendchar2(unsigned char ch);
void print2(unsigned char *str,int len);

//bt
void uart3_init(unsigned int baud);
void uart3_dma(void);
void sendchar3(unsigned char ch);
void print3(unsigned char *str,int len);

//232 2
void uart4_init(unsigned int baud);
void uart4_dma(void);
void sendchar4(unsigned char ch);
void print4(unsigned char *str,int len);

#endif
