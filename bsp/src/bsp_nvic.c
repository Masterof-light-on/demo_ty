#include "includes.h"

void nvic_init(void)
{
	nvic_irq_enable(TIMER1_IRQn, 5, 0);
	
	nvic_irq_enable(USART0_IRQn, 6, 0);
	nvic_irq_enable(DMA1_Channel7_IRQn, 6, 0);
	
	nvic_irq_enable(USART1_IRQn, 6, 0);
	nvic_irq_enable(DMA0_Channel6_IRQn, 6, 0);

	nvic_irq_enable(USART2_IRQn, 6, 0);
	nvic_irq_enable(DMA0_Channel3_IRQn, 6, 0);
	
	nvic_irq_enable(UART3_IRQn, 6, 0);
	nvic_irq_enable(DMA0_Channel4_IRQn, 6, 0);

	nvic_irq_enable(UART4_IRQn, 6, 0);
	nvic_irq_enable(DMA0_Channel7_IRQn, 6, 0);
}

