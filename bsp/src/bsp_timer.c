#include "includes.h"

void timer1_init(void)
{
	timer_parameter_struct timer_initpara;

    rcu_periph_clock_enable(RCU_TIMER1);
    rcu_timer_clock_prescaler_config(RCU_TIMER_PSC_MUL4);

    timer_deinit(TIMER1);

	timer_initpara.prescaler         = 168-1;
    timer_initpara.alignedmode       = TIMER_COUNTER_EDGE;
    timer_initpara.counterdirection  = TIMER_COUNTER_UP;
    timer_initpara.period            = 10000-1;
    timer_initpara.clockdivision     = TIMER_CKDIV_DIV1;
    timer_initpara.repetitioncounter = 0;
    timer_init(TIMER1,&timer_initpara);

	timer_interrupt_flag_clear(TIMER1, TIMER_INT_UP);
	timer_interrupt_enable(TIMER1, TIMER_INT_UP);
	timer_enable(TIMER1);
}

void TIMER1_IRQHandler(void)
{
	if(timer_interrupt_flag_get(TIMER1, TIMER_INT_UP))
	{
		timer_interrupt_flag_clear(TIMER1, TIMER_INT_UP);
		Timer_IRQHandlerCB();
	}
}

