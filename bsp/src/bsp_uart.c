#include "includes.h"

void uart0_init(unsigned int baud)
{
	rcu_periph_clock_enable(RCU_GPIOA);

    rcu_periph_clock_enable(RCU_USART0);

    /* connect port to USARTx_Tx */
    gpio_af_set(GPIOA, GPIO_AF_7, GPIO_PIN_9);

    /* connect port to USARTx_Rx */
    gpio_af_set(GPIOA, GPIO_AF_7, GPIO_PIN_10);

    /* configure USART Tx as alternate function push-pull */
    gpio_mode_set(GPIOA, GPIO_MODE_AF, GPIO_PUPD_PULLUP, GPIO_PIN_9);
    gpio_output_options_set(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_9);

    /* configure USART Rx as alternate function push-pull */
    gpio_mode_set(GPIOA, GPIO_MODE_AF, GPIO_PUPD_PULLUP, GPIO_PIN_10);
    gpio_output_options_set(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_10);

    /* USART configure */
    usart_deinit(USART0);
    usart_baudrate_set(USART0, baud);
    usart_receive_config(USART0, USART_RECEIVE_ENABLE);
    usart_transmit_config(USART0, USART_TRANSMIT_ENABLE);
    usart_enable(USART0);

	uart0_dma();
}

static uart0_tx uart0_dma_tx;
static uart0_rx uart0_dma_rx;

void uart0_dma(void)
{
	dma_single_data_parameter_struct dma_init_struct;

	rcu_periph_clock_enable(RCU_DMA1);
	
    dma_single_data_para_struct_init(&dma_init_struct);
    dma_deinit(DMA1, DMA_CH7);
    dma_init_struct.direction = DMA_MEMORY_TO_PERIPH;
    dma_init_struct.memory0_addr = (unsigned int)uart0_dma_tx.data;
    dma_init_struct.memory_inc = DMA_MEMORY_INCREASE_ENABLE;
    dma_init_struct.periph_memory_width = DMA_PERIPH_WIDTH_8BIT;
    dma_init_struct.number = 0;
    dma_init_struct.periph_addr = (unsigned int)&USART_DATA(USART0);
    dma_init_struct.periph_inc = DMA_PERIPH_INCREASE_DISABLE;
    dma_init_struct.priority = DMA_PRIORITY_HIGH;
    dma_single_data_mode_init(DMA1, DMA_CH7, &dma_init_struct);
    
    dma_circulation_disable(DMA1, DMA_CH7);
    dma_channel_subperipheral_select(DMA1, DMA_CH7, DMA_SUBPERI4);
    dma_interrupt_enable(DMA1, DMA_CH7, DMA_CHXCTL_FTFIE);
    dma_channel_enable(DMA1, DMA_CH7);
	usart_dma_transmit_config(USART0, USART_DENT_ENABLE);

	dma_deinit(DMA1, DMA_CH2);
    dma_init_struct.direction = DMA_PERIPH_TO_MEMORY;
    dma_init_struct.memory0_addr = (unsigned int)uart0_dma_rx.data;
    dma_init_struct.memory_inc = DMA_MEMORY_INCREASE_ENABLE;
    dma_init_struct.number = UART0_MAX_RX;
    dma_init_struct.periph_addr = (unsigned int)&USART_DATA(USART0);
    dma_init_struct.periph_inc = DMA_PERIPH_INCREASE_DISABLE;
    dma_init_struct.periph_memory_width = DMA_PERIPH_WIDTH_8BIT;
    dma_init_struct.priority = DMA_PRIORITY_HIGH;
    dma_single_data_mode_init(DMA1, DMA_CH2, &dma_init_struct);
    
    dma_circulation_disable(DMA1, DMA_CH2);
    dma_channel_subperipheral_select(DMA1, DMA_CH2, DMA_SUBPERI4);
    dma_channel_enable(DMA1, DMA_CH2);
	usart_dma_receive_config(USART0, USART_DENR_ENABLE);
	usart_interrupt_enable(USART0, USART_INT_IDLE);
}

void uart0_dma_send(unsigned char *buf, unsigned short len)
{
	while(dma_transfer_number_get(DMA1, DMA_CH7));

	memcpy_t(uart0_dma_tx.data, buf, len);

	dma_channel_disable(DMA1, DMA_CH7);
	dma_transfer_number_config(DMA1, DMA_CH7, len);
	dma_memory_address_config(DMA1, DMA_CH7, DMA_MEMORY_0, (unsigned int)uart0_dma_tx.data);
	dma_channel_enable(DMA1, DMA_CH7);
}

void sendchar0(unsigned char ch)
{
	uart0_dma_send(&ch, 1);
}

void print0(unsigned char *str,int len)
{
	uart0_dma_send(str, len);
}

void DMA1_Channel7_IRQHandler(void)
{
    if(dma_interrupt_flag_get(DMA1, DMA_CH7, DMA_INT_FLAG_FTF))
	{
        dma_interrupt_flag_clear(DMA1, DMA_CH7, DMA_INT_FLAG_FTF);
        dma_channel_disable(DMA1, DMA_CH7);
    }
}

void USART0_IRQHandler(void)
{
	unsigned int len;
	
    if(RESET != usart_interrupt_flag_get(USART0, USART_INT_FLAG_IDLE))
	{
		USART_STAT0(USART0);
		USART_DATA(USART0);
		
        len = UART0_MAX_RX - dma_transfer_number_get(DMA1, DMA_CH2);

		dma_channel_disable(DMA1, DMA_CH2);
		dma_interrupt_flag_clear(DMA1, DMA_CH2, DMA_INT_FLAG_FTF);
		dma_transfer_number_config(DMA1, DMA_CH2, UART0_MAX_RX);
		dma_channel_enable(DMA1, DMA_CH2);

		dev_recv(COM0, uart0_dma_rx.data, len);
    }
}

void uart1_init(unsigned int baud)
{
	rcu_periph_clock_enable(RCU_GPIOA);

    rcu_periph_clock_enable(RCU_USART1);

    /* connect port to USARTx_Tx */
    gpio_af_set(GPIOA, GPIO_AF_7, GPIO_PIN_2);

    /* connect port to USARTx_Rx */
    gpio_af_set(GPIOA, GPIO_AF_7, GPIO_PIN_3);

    /* configure USART Tx as alternate function push-pull */
    gpio_mode_set(GPIOA, GPIO_MODE_AF, GPIO_PUPD_PULLUP, GPIO_PIN_2);
    gpio_output_options_set(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_2);

    /* configure USART Rx as alternate function push-pull */
    gpio_mode_set(GPIOA, GPIO_MODE_AF, GPIO_PUPD_PULLUP, GPIO_PIN_3);
    gpio_output_options_set(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_3);

    /* USART configure */
    usart_deinit(USART1);
    usart_baudrate_set(USART1, baud);
    usart_receive_config(USART1, USART_RECEIVE_ENABLE);
    usart_transmit_config(USART1, USART_TRANSMIT_ENABLE);
    usart_enable(USART1);

	uart1_dma();
}

static uart1_tx uart1_dma_tx;
static uart1_rx uart1_dma_rx;

void uart1_dma(void)
{
	dma_single_data_parameter_struct dma_init_struct;

	rcu_periph_clock_enable(RCU_DMA0);
	
    dma_single_data_para_struct_init(&dma_init_struct);
    dma_deinit(DMA0, DMA_CH6);
    dma_init_struct.direction = DMA_MEMORY_TO_PERIPH;
    dma_init_struct.memory0_addr = (unsigned int)uart1_dma_tx.data;
    dma_init_struct.memory_inc = DMA_MEMORY_INCREASE_ENABLE;
    dma_init_struct.periph_memory_width = DMA_PERIPH_WIDTH_8BIT;
    dma_init_struct.number = 0;
    dma_init_struct.periph_addr = (unsigned int)&USART_DATA(USART1);
    dma_init_struct.periph_inc = DMA_PERIPH_INCREASE_DISABLE;
    dma_init_struct.priority = DMA_PRIORITY_HIGH;
    dma_single_data_mode_init(DMA0, DMA_CH6, &dma_init_struct);
    
    dma_circulation_disable(DMA0, DMA_CH6);
    dma_channel_subperipheral_select(DMA0, DMA_CH6, DMA_SUBPERI4);
    dma_interrupt_enable(DMA0, DMA_CH6, DMA_CHXCTL_FTFIE);
    dma_channel_enable(DMA0, DMA_CH6);
	usart_dma_transmit_config(USART1, USART_DENT_ENABLE);

	dma_deinit(DMA0, DMA_CH5);
    dma_init_struct.direction = DMA_PERIPH_TO_MEMORY;
    dma_init_struct.memory0_addr = (unsigned int)uart1_dma_rx.data;
    dma_init_struct.memory_inc = DMA_MEMORY_INCREASE_ENABLE;
    dma_init_struct.number = UART1_MAX_RX;
    dma_init_struct.periph_addr = (unsigned int)&USART_DATA(USART1);
    dma_init_struct.periph_inc = DMA_PERIPH_INCREASE_DISABLE;
    dma_init_struct.periph_memory_width = DMA_PERIPH_WIDTH_8BIT;
    dma_init_struct.priority = DMA_PRIORITY_HIGH;
    dma_single_data_mode_init(DMA0, DMA_CH5, &dma_init_struct);
    
    dma_circulation_disable(DMA0, DMA_CH5);
    dma_channel_subperipheral_select(DMA0, DMA_CH5, DMA_SUBPERI4);
    dma_channel_enable(DMA0, DMA_CH5);
	usart_dma_receive_config(USART1, USART_DENR_ENABLE);
	usart_interrupt_enable(USART1, USART_INT_IDLE);
}

void uart1_dma_send(unsigned char *buf, unsigned short len)
{
	while(dma_transfer_number_get(DMA0, DMA_CH6));

	memcpy_t(uart1_dma_tx.data, buf, len);

	dma_channel_disable(DMA0, DMA_CH6);
	dma_transfer_number_config(DMA0, DMA_CH6, len);
	dma_memory_address_config(DMA0, DMA_CH6, DMA_MEMORY_0, (unsigned int)uart1_dma_tx.data);
	dma_channel_enable(DMA0, DMA_CH6);
}

void sendchar1(unsigned char ch)
{
	uart1_dma_send(&ch, 1);
}

void print1(unsigned char *str,int len)
{
	uart1_dma_send(str, len);
}

void DMA0_Channel6_IRQHandler(void)
{
    if(dma_interrupt_flag_get(DMA0, DMA_CH6, DMA_INT_FLAG_FTF))
	{
        dma_interrupt_flag_clear(DMA0, DMA_CH6, DMA_INT_FLAG_FTF);
        dma_channel_disable(DMA0, DMA_CH6);
    }
}

void USART1_IRQHandler(void)
{
	unsigned int len;
	
    if(RESET != usart_interrupt_flag_get(USART1, USART_INT_FLAG_IDLE))
	{
		USART_STAT0(USART1);
		USART_DATA(USART1);
		
        len = UART1_MAX_RX - dma_transfer_number_get(DMA0, DMA_CH5);

		dma_channel_disable(DMA0, DMA_CH5);
		dma_interrupt_flag_clear(DMA0, DMA_CH5, DMA_INT_FLAG_FTF);
		dma_transfer_number_config(DMA0, DMA_CH5, UART1_MAX_RX);
		dma_channel_enable(DMA0, DMA_CH5);

		gps_recv(uart1_dma_rx.data, len);
    }
}

void uart2_init(unsigned int baud)
{
	rcu_periph_clock_enable(RCU_GPIOB);

    rcu_periph_clock_enable(RCU_USART2);

    /* connect port to USARTx_Tx */
    gpio_af_set(GPIOB, GPIO_AF_7, GPIO_PIN_10);

    /* connect port to USARTx_Rx */
    gpio_af_set(GPIOB, GPIO_AF_7, GPIO_PIN_11);

    /* configure USART Tx as alternate function push-pull */
    gpio_mode_set(GPIOB, GPIO_MODE_AF, GPIO_PUPD_PULLUP, GPIO_PIN_10);
    gpio_output_options_set(GPIOB, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_10);

    /* configure USART Rx as alternate function push-pull */
    gpio_mode_set(GPIOB, GPIO_MODE_AF, GPIO_PUPD_PULLUP, GPIO_PIN_11);
    gpio_output_options_set(GPIOB, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_11);

    /* USART configure */
    usart_deinit(USART2);
    usart_baudrate_set(USART2, baud);
    usart_receive_config(USART2, USART_RECEIVE_ENABLE);
    usart_transmit_config(USART2, USART_TRANSMIT_ENABLE);
    usart_enable(USART2);

	uart2_dma();
}

static uart2_tx uart2_dma_tx;
static uart2_rx uart2_dma_rx;

void uart2_dma(void)
{
	dma_single_data_parameter_struct dma_init_struct;

	rcu_periph_clock_enable(RCU_DMA0);
	
    dma_single_data_para_struct_init(&dma_init_struct);
    dma_deinit(DMA0, DMA_CH3);
    dma_init_struct.direction = DMA_MEMORY_TO_PERIPH;
    dma_init_struct.memory0_addr = (unsigned int)uart2_dma_tx.data;
    dma_init_struct.memory_inc = DMA_MEMORY_INCREASE_ENABLE;
    dma_init_struct.periph_memory_width = DMA_PERIPH_WIDTH_8BIT;
    dma_init_struct.number = 0;
    dma_init_struct.periph_addr = (unsigned int)&USART_DATA(USART2);
    dma_init_struct.periph_inc = DMA_PERIPH_INCREASE_DISABLE;
    dma_init_struct.priority = DMA_PRIORITY_HIGH;
    dma_single_data_mode_init(DMA0, DMA_CH3, &dma_init_struct);
    
    dma_circulation_disable(DMA0, DMA_CH3);
    dma_channel_subperipheral_select(DMA0, DMA_CH3, DMA_SUBPERI4);
    dma_interrupt_enable(DMA0, DMA_CH3, DMA_CHXCTL_FTFIE);
    dma_channel_enable(DMA0, DMA_CH3);
	usart_dma_transmit_config(USART2, USART_DENT_ENABLE);

	dma_deinit(DMA0, DMA_CH1);
    dma_init_struct.direction = DMA_PERIPH_TO_MEMORY;
    dma_init_struct.memory0_addr = (unsigned int)uart2_dma_rx.data;
    dma_init_struct.memory_inc = DMA_MEMORY_INCREASE_ENABLE;
    dma_init_struct.number = UART2_MAX_RX;
    dma_init_struct.periph_addr = (unsigned int)&USART_DATA(USART2);
    dma_init_struct.periph_inc = DMA_PERIPH_INCREASE_DISABLE;
    dma_init_struct.periph_memory_width = DMA_PERIPH_WIDTH_8BIT;
    dma_init_struct.priority = DMA_PRIORITY_HIGH;
    dma_single_data_mode_init(DMA0, DMA_CH1, &dma_init_struct);
    
    dma_circulation_disable(DMA0, DMA_CH1);
    dma_channel_subperipheral_select(DMA0, DMA_CH1, DMA_SUBPERI4);
    dma_channel_enable(DMA0, DMA_CH1);
	usart_dma_receive_config(USART2, USART_DENR_ENABLE);
	usart_interrupt_enable(USART2, USART_INT_IDLE);
}

void uart2_dma_send(unsigned char *buf, unsigned short len)
{
	while(dma_transfer_number_get(DMA0, DMA_CH3));

	memcpy_t(uart2_dma_tx.data, buf, len);

	dma_channel_disable(DMA0, DMA_CH3);
	dma_transfer_number_config(DMA0, DMA_CH3, len);
	dma_memory_address_config(DMA0, DMA_CH3, DMA_MEMORY_0, (unsigned int)uart2_dma_tx.data);
	dma_channel_enable(DMA0, DMA_CH3);
}

void sendchar2(unsigned char ch)
{
	uart2_dma_send(&ch, 1);
}

void print2(unsigned char *str,int len)
{
	uart2_dma_send(str, len);
}

void DMA0_Channel3_IRQHandler(void)
{
    if(dma_interrupt_flag_get(DMA0, DMA_CH3, DMA_INT_FLAG_FTF))
	{
        dma_interrupt_flag_clear(DMA0, DMA_CH3, DMA_INT_FLAG_FTF);
        dma_channel_disable(DMA0, DMA_CH3);
    }
}

void USART2_IRQHandler(void)
{
	unsigned int len;
	
    if(RESET != usart_interrupt_flag_get(USART2, USART_INT_FLAG_IDLE))
	{
		USART_STAT0(USART2);
		USART_DATA(USART2);
		
        len = UART2_MAX_RX - dma_transfer_number_get(DMA0, DMA_CH1);

		dma_channel_disable(DMA0, DMA_CH1);
		dma_interrupt_flag_clear(DMA0, DMA_CH1, DMA_INT_FLAG_FTF);
		dma_transfer_number_config(DMA0, DMA_CH1, UART2_MAX_RX);
		dma_channel_enable(DMA0, DMA_CH1);

		modem_recv(uart2_dma_rx.data, len);
    }
}
//bt
void uart3_init(unsigned int baud)
{
	rcu_periph_clock_enable(RCU_GPIOC);

    rcu_periph_clock_enable(RCU_UART3);

    /* connect port to USARTx_Tx */
    gpio_af_set(GPIOC, GPIO_AF_8, GPIO_PIN_10);

    /* connect port to USARTx_Rx */
    gpio_af_set(GPIOC, GPIO_AF_8, GPIO_PIN_11);

    /* configure USART Tx as alternate function push-pull */
    gpio_mode_set(GPIOC, GPIO_MODE_AF, GPIO_PUPD_PULLUP, GPIO_PIN_10);
    gpio_output_options_set(GPIOC, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_10);

    /* configure USART Rx as alternate function push-pull */
    gpio_mode_set(GPIOC, GPIO_MODE_AF, GPIO_PUPD_PULLUP, GPIO_PIN_11);
    gpio_output_options_set(GPIOC, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_11);

    /* USART configure */
    usart_deinit(UART3);
    usart_baudrate_set(UART3, baud);
    usart_receive_config(UART3, USART_RECEIVE_ENABLE);
    usart_transmit_config(UART3, USART_TRANSMIT_ENABLE);
    usart_enable(UART3);

	uart3_dma();
}

static uart3_tx uart3_dma_tx;
static uart3_rx uart3_dma_rx;

void uart3_dma(void)
{
	dma_single_data_parameter_struct dma_init_struct;

	rcu_periph_clock_enable(RCU_DMA0);
	
    dma_single_data_para_struct_init(&dma_init_struct);
    dma_deinit(DMA0, DMA_CH4);
    dma_init_struct.direction = DMA_MEMORY_TO_PERIPH;
    dma_init_struct.memory0_addr = (unsigned int)uart3_dma_tx.data;
    dma_init_struct.memory_inc = DMA_MEMORY_INCREASE_ENABLE;
    dma_init_struct.periph_memory_width = DMA_PERIPH_WIDTH_8BIT;
    dma_init_struct.number = 0;
    dma_init_struct.periph_addr = (unsigned int)&USART_DATA(UART3);
    dma_init_struct.periph_inc = DMA_PERIPH_INCREASE_DISABLE;
    dma_init_struct.priority = DMA_PRIORITY_HIGH;
    dma_single_data_mode_init(DMA0, DMA_CH4, &dma_init_struct);
    
    dma_circulation_disable(DMA0, DMA_CH4);
    dma_channel_subperipheral_select(DMA0, DMA_CH4, DMA_SUBPERI4);
    dma_interrupt_enable(DMA0, DMA_CH4, DMA_CHXCTL_FTFIE);
    dma_channel_enable(DMA0, DMA_CH4);
	usart_dma_transmit_config(UART3, USART_DENT_ENABLE);

	dma_deinit(DMA0, DMA_CH2);
    dma_init_struct.direction = DMA_PERIPH_TO_MEMORY;
    dma_init_struct.memory0_addr = (unsigned int)uart3_dma_rx.data;
    dma_init_struct.memory_inc = DMA_MEMORY_INCREASE_ENABLE;
    dma_init_struct.number = UART3_MAX_RX;
    dma_init_struct.periph_addr = (unsigned int)&USART_DATA(UART3);
    dma_init_struct.periph_inc = DMA_PERIPH_INCREASE_DISABLE;
    dma_init_struct.periph_memory_width = DMA_PERIPH_WIDTH_8BIT;
    dma_init_struct.priority = DMA_PRIORITY_HIGH;
    dma_single_data_mode_init(DMA0, DMA_CH2, &dma_init_struct);
    
    dma_circulation_disable(DMA0, DMA_CH2);
    dma_channel_subperipheral_select(DMA0, DMA_CH2, DMA_SUBPERI4);
    dma_channel_enable(DMA0, DMA_CH2);
	usart_dma_receive_config(UART3, USART_DENR_ENABLE);
	usart_interrupt_enable(UART3, USART_INT_IDLE);
}

void uart3_dma_send(unsigned char *buf, unsigned short len)
{
	while(dma_transfer_number_get(DMA0, DMA_CH4));

	memcpy_t(uart3_dma_tx.data, buf, len);

	dma_channel_disable(DMA0, DMA_CH4);
	dma_transfer_number_config(DMA0, DMA_CH4, len);
	dma_memory_address_config(DMA0, DMA_CH4, DMA_MEMORY_0, (unsigned int)uart3_dma_tx.data);
	dma_channel_enable(DMA0, DMA_CH4);
}

void sendchar3(unsigned char ch)
{
	uart3_dma_send(&ch, 1);
}

void print3(unsigned char *str,int len)
{
	uart3_dma_send(str, len);
}

void DMA0_Channel4_IRQHandler(void)
{
    if(dma_interrupt_flag_get(DMA0, DMA_CH4, DMA_INT_FLAG_FTF))
	{
        dma_interrupt_flag_clear(DMA0, DMA_CH4, DMA_INT_FLAG_FTF);
        dma_channel_disable(DMA0, DMA_CH4);
    }
}

void UART3_IRQHandler(void)
{
	unsigned int len;
	
    if(RESET != usart_interrupt_flag_get(UART3, USART_INT_FLAG_IDLE))
	{
		USART_STAT0(UART3);
		USART_DATA(UART3);
		
        len = UART3_MAX_RX - dma_transfer_number_get(DMA0, DMA_CH2);

		dma_channel_disable(DMA0, DMA_CH2);
		dma_interrupt_flag_clear(DMA0, DMA_CH2, DMA_INT_FLAG_FTF);
		dma_transfer_number_config(DMA0, DMA_CH2, UART3_MAX_RX);
		dma_channel_enable(DMA0, DMA_CH2);

		dev_recv(COM3, uart3_dma_rx.data, len);
    }
}

//232
#define UART4FUN 0
#define RX_A485()	gpio_bit_set(GPIOA,GPIO_PIN_15)//gpio_bit_reset
#define TX_A485()   gpio_bit_reset(GPIOA,GPIO_PIN_15)//gpio_bit_set
void uart4_init(unsigned int baud)
{
	rcu_periph_clock_enable(RCU_GPIOC);
	rcu_periph_clock_enable(RCU_GPIOD);

    rcu_periph_clock_enable(RCU_UART4);

    /* connect port to USARTx_Tx */
    gpio_af_set(GPIOC, GPIO_AF_8, GPIO_PIN_12);

    /* connect port to USARTx_Rx */
    gpio_af_set(GPIOD, GPIO_AF_8, GPIO_PIN_2);

		/* connect port to 485 DR */
	gpio_mode_set(GPIOA,GPIO_MODE_OUTPUT,GPIO_PUPD_PULLUP,GPIO_PIN_15);
    gpio_output_options_set(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_15);
	
    /* configure USART Tx as alternate function push-pull */
    gpio_mode_set(GPIOC, GPIO_MODE_AF, GPIO_PUPD_PULLUP, GPIO_PIN_12);
    gpio_output_options_set(GPIOC, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_12);

    /* configure USART Rx as alternate function push-pull */
    gpio_mode_set(GPIOD, GPIO_MODE_AF, GPIO_PUPD_PULLUP, GPIO_PIN_2);
    gpio_output_options_set(GPIOD, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_2);

    /* USART configure */
    usart_deinit(UART4);
    usart_baudrate_set(UART4, baud);
    usart_receive_config(UART4, USART_RECEIVE_ENABLE);
    usart_transmit_config(UART4, USART_TRANSMIT_ENABLE);
    usart_enable(UART4);

	uart4_dma();
	#if UART4FUN
	RX_A485();
	#endif
}

static uart4_tx uart4_dma_tx;
static uart4_rx uart4_dma_rx;

void uart4_dma(void)
{
	dma_single_data_parameter_struct dma_init_struct;

	rcu_periph_clock_enable(RCU_DMA0);
	
    dma_single_data_para_struct_init(&dma_init_struct);
    dma_deinit(DMA0, DMA_CH7);
    dma_init_struct.direction = DMA_MEMORY_TO_PERIPH;
    dma_init_struct.memory0_addr = (unsigned int)uart4_dma_tx.data;
    dma_init_struct.memory_inc = DMA_MEMORY_INCREASE_ENABLE;
    dma_init_struct.periph_memory_width = DMA_PERIPH_WIDTH_8BIT;
    dma_init_struct.number = 0;
    dma_init_struct.periph_addr = (unsigned int)&USART_DATA(UART4);
    dma_init_struct.periph_inc = DMA_PERIPH_INCREASE_DISABLE;
    dma_init_struct.priority = DMA_PRIORITY_HIGH;
    dma_single_data_mode_init(DMA0, DMA_CH7, &dma_init_struct);
    
    dma_circulation_disable(DMA0, DMA_CH7);
    dma_channel_subperipheral_select(DMA0, DMA_CH7, DMA_SUBPERI4);
    dma_interrupt_enable(DMA0, DMA_CH7, DMA_CHXCTL_FTFIE);
    dma_channel_enable(DMA0, DMA_CH7);
	usart_dma_transmit_config(UART4, USART_DENT_ENABLE);

	dma_deinit(DMA0, DMA_CH0);
    dma_init_struct.direction = DMA_PERIPH_TO_MEMORY;
    dma_init_struct.memory0_addr = (unsigned int)uart4_dma_rx.data;
    dma_init_struct.memory_inc = DMA_MEMORY_INCREASE_ENABLE;
    dma_init_struct.number = UART4_MAX_RX;
    dma_init_struct.periph_addr = (unsigned int)&USART_DATA(UART4);
    dma_init_struct.periph_inc = DMA_PERIPH_INCREASE_DISABLE;
    dma_init_struct.periph_memory_width = DMA_PERIPH_WIDTH_8BIT;
    dma_init_struct.priority = DMA_PRIORITY_HIGH;
    dma_single_data_mode_init(DMA0, DMA_CH0, &dma_init_struct);
    
    #if UART4FUN
		dma_circulation_enable(DMA0, DMA_CH0);//485
	#else
		dma_circulation_disable(DMA0, DMA_CH0);//232
	#endif
    dma_channel_subperipheral_select(DMA0, DMA_CH0, DMA_SUBPERI4);
    dma_channel_enable(DMA0, DMA_CH0);
	usart_dma_receive_config(UART4, USART_DENR_ENABLE);
	usart_interrupt_enable(UART4, USART_INT_IDLE);
}

void uart4_dma_send(unsigned char *buf, unsigned short len)
{
	while(dma_transfer_number_get(DMA0, DMA_CH7));

	#if UART4FUN
		TX_A485();//发送模式置1
	#endif
	
	memcpy_t(uart4_dma_tx.data, buf, len);

	dma_channel_disable(DMA0, DMA_CH7);
	dma_transfer_number_config(DMA0, DMA_CH7, len);
	dma_memory_address_config(DMA0, DMA_CH7, DMA_MEMORY_0, (unsigned int)uart4_dma_tx.data);
	dma_channel_enable(DMA0, DMA_CH7);
}

void sendchar4(unsigned char ch)
{
	uart4_dma_send(&ch, 1);
}

void print4(unsigned char *str,int len)
{
	uart4_dma_send(str, len);
}

void DMA0_Channel7_IRQHandler(void)
{
    if(dma_interrupt_flag_get(DMA0, DMA_CH7, DMA_INT_FLAG_FTF))
	{
        dma_interrupt_flag_clear(DMA0, DMA_CH7, DMA_INT_FLAG_FTF);
        dma_channel_disable(DMA0, DMA_CH7);
		#if UART4FUN
		usart_interrupt_flag_clear(UART4,USART_INT_FLAG_TC);//清USART_IT_TC标志
		usart_interrupt_enable(UART4,USART_INT_TC);//禁止发送中断
		#endif
    }
}

void UART4_IRQHandler(void)
{
	unsigned int len;
	
    if(RESET != usart_interrupt_flag_get(UART4, USART_INT_FLAG_IDLE))
	{
		USART_STAT0(UART4);
		USART_DATA(UART4);
		
        len = UART4_MAX_RX - dma_transfer_number_get(DMA0, DMA_CH0);

		dma_channel_disable(DMA0, DMA_CH0);
		dma_interrupt_flag_clear(DMA0, DMA_CH0, DMA_INT_FLAG_FTF);
		dma_transfer_number_config(DMA0, DMA_CH0, UART4_MAX_RX);
		dma_channel_enable(DMA0, DMA_CH0);

		dev_recv(COM4, uart4_dma_rx.data, len);
    }
	#if UART4FUN
	if(RESET != usart_interrupt_flag_get(UART4, USART_INT_FLAG_TC) )
	{
		RX_A485();//接收模式置0

		usart_interrupt_flag_clear(UART4,USART_INT_FLAG_TC);//清USART_IT_TC标志

		usart_interrupt_disable(UART4,USART_INT_TC);//禁止发送中断
	}
	#endif
}
