#include "includes.h"

void canx_init(void)
{
	can_parameter_struct can_parameter = {0};
	
	rcu_periph_clock_enable(RCU_CAN0);
    rcu_periph_clock_enable(RCU_CAN1);

	rcu_periph_clock_enable(RCU_GPIOB);
    rcu_periph_clock_enable(RCU_GPIOD);
	//CAN0
	gpio_output_options_set(GPIOD, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_0);
    gpio_mode_set(GPIOD, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_0);
    gpio_af_set(GPIOD, GPIO_AF_9, GPIO_PIN_0);
    
    gpio_output_options_set(GPIOD, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_1);
    gpio_mode_set(GPIOD, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_1);
    gpio_af_set(GPIOD, GPIO_AF_9, GPIO_PIN_1);
	//CAN1
	gpio_output_options_set(GPIOB, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_5);
    gpio_mode_set(GPIOB, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_5);
    gpio_af_set(GPIOB, GPIO_AF_9, GPIO_PIN_5);
    
    gpio_output_options_set(GPIOB, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_6);
    gpio_mode_set(GPIOB, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_6);
    gpio_af_set(GPIOB, GPIO_AF_9, GPIO_PIN_6);

	can_deinit(CAN0);
	can_parameter.time_triggered = DISABLE;
    can_parameter.auto_bus_off_recovery = ENABLE;
    can_parameter.auto_wake_up = DISABLE;
    can_parameter.no_auto_retrans = DISABLE;
    can_parameter.rec_fifo_overwrite = DISABLE;
    can_parameter.trans_fifo_order = DISABLE;
    can_parameter.working_mode = CAN_NORMAL_MODE;
    can_parameter.resync_jump_width = CAN_BT_SJW_1TQ;
	if(sys_GetCan1Baud() == 125)
	{
		can_parameter.time_segment_1 = CAN_BT_BS1_6TQ;
	    can_parameter.time_segment_2 = CAN_BT_BS2_1TQ;
		can_parameter.prescaler = 42;//125kb
	}
	else if(sys_GetCan1Baud() == 250)
	{
	    can_parameter.time_segment_1 = CAN_BT_BS1_6TQ;
	    can_parameter.time_segment_2 = CAN_BT_BS2_1TQ;
		can_parameter.prescaler = 21;//250kb
	}
	else
	{
		can_parameter.time_segment_1 = CAN_BT_BS1_5TQ;
	    can_parameter.time_segment_2 = CAN_BT_BS2_1TQ;
		can_parameter.prescaler = 12;//500kb
	}

	can_init(CAN0, &can_parameter);
	can_interrupt_enable(CAN0, CAN_INT_RFNE0);

	can_deinit(CAN1);
	can_parameter.time_triggered = DISABLE;
    can_parameter.auto_bus_off_recovery = ENABLE;
    can_parameter.auto_wake_up = DISABLE;
    can_parameter.no_auto_retrans = DISABLE;
    can_parameter.rec_fifo_overwrite = DISABLE;
    can_parameter.trans_fifo_order = DISABLE;
    can_parameter.working_mode = CAN_NORMAL_MODE;
    can_parameter.resync_jump_width = CAN_BT_SJW_1TQ;
    if(sys_GetCan2Baud() == 125)
	{
		can_parameter.time_segment_1 = CAN_BT_BS1_6TQ;
	    can_parameter.time_segment_2 = CAN_BT_BS2_1TQ;
		can_parameter.prescaler = 42;//125kb
	}
	else if(sys_GetCan2Baud() == 250)
	{
	    can_parameter.time_segment_1 = CAN_BT_BS1_6TQ;
	    can_parameter.time_segment_2 = CAN_BT_BS2_1TQ;
		can_parameter.prescaler = 21;//250kb
	}
	else
	{
		can_parameter.time_segment_1 = CAN_BT_BS1_5TQ;
	    can_parameter.time_segment_2 = CAN_BT_BS2_1TQ;
		can_parameter.prescaler = 12;//500kb
	}

	can_init(CAN1, &can_parameter);
	can_interrupt_enable(CAN1, CAN_INT_RFNE1);

	can_SetRecvFilter();
}

void can_SetFilter32IdList(unsigned char FilterNum, unsigned int FilterId, unsigned int FilterMask)
{
    can_filter_parameter_struct can_filter;
    
	can_filter.filter_number = FilterNum;
	can_filter.filter_mode = CAN_FILTERMODE_LIST;
	can_filter.filter_bits = CAN_FILTERBITS_32BIT;
	if(FilterId > 0x7FF)
	{
		can_filter.filter_list_high = ((FilterId<<3)&0xFFFF0000)>>16;
		can_filter.filter_list_low = ((FilterId<<3)|CAN_FF_EXTENDED|CAN_FT_DATA)&0xFFFF;
	}
	else
	{
		can_filter.filter_list_high = (FilterId<<5);
		can_filter.filter_list_low = (0|CAN_FF_STANDARD|CAN_FT_DATA);
	}
	if(FilterMask > 0x7FF)
	{
		can_filter.filter_mask_high = ((FilterMask<<3)&0xFFFF0000)>>16;
		can_filter.filter_mask_low = ((FilterMask<<3)|CAN_FF_EXTENDED|CAN_FT_DATA)&0xFFFF;
	}
	else
	{
		can_filter.filter_mask_high = (FilterMask<<5);
		can_filter.filter_mask_low = (0|CAN_FF_STANDARD|CAN_FT_DATA);
	}
	if(FilterNum < 14)
	{
		can_filter.filter_fifo_number = CAN_FIFO0;
	}
	else
	{
		can_filter.filter_fifo_number = CAN_FIFO1;
	}
	can_filter.filter_enable = ENABLE;
	
	can_filter_init(&can_filter);
}

void can_SetFilter16IdList(unsigned char FilterNum, unsigned short FilterId1, unsigned short FilterId2, unsigned short FilterMask1, unsigned short FilterMask2)
{
    can_filter_parameter_struct can_filter;

	if((FilterId1 > 0x7FF)||(FilterId2 > 0x7FF)||(FilterMask1 > 0x7FF)||(FilterMask2 > 0x7FF))
	{
		return;
	}
    
	can_filter.filter_number = FilterNum;
	can_filter.filter_mode = CAN_FILTERMODE_LIST;
	can_filter.filter_bits = CAN_FILTERBITS_16BIT;
	can_filter.filter_list_high = (FilterId1<<5);
	can_filter.filter_list_low = (FilterId2<<5);
	can_filter.filter_mask_high = (FilterMask1<<5);
	can_filter.filter_mask_low = (FilterMask2<<5);
	
	if(FilterNum < 14)
	{
		can_filter.filter_fifo_number = CAN_FIFO0;
	}
	else
	{
		can_filter.filter_fifo_number = CAN_FIFO1;
	}
	can_filter.filter_enable = ENABLE;
	
	can_filter_init(&can_filter);
}

void can_SetFilter32IdMask(unsigned char FilterNum, unsigned int FilterId, unsigned int FilterMask)
{
    can_filter_parameter_struct can_filter;
    
	can_filter.filter_number = FilterNum;
	can_filter.filter_mode = CAN_FILTERMODE_MASK;
	can_filter.filter_bits = CAN_FILTERBITS_32BIT;
	if(FilterId > 0x7FF)
	{
		can_filter.filter_list_high = ((FilterId<<3)&0xFFFF0000)>>16;
		can_filter.filter_list_low = ((FilterId<<3)|CAN_FF_EXTENDED|CAN_FT_DATA)&0xFFFF;
	}
	else
	{
		can_filter.filter_list_high = (FilterId<<5);
		can_filter.filter_list_low = (0|CAN_FF_STANDARD|CAN_FT_DATA);
	}
	if(FilterId > 0x7FF)
	{
		can_filter.filter_mask_high = ((FilterMask<<3)&0xFFFF0000)>>16;
		can_filter.filter_mask_low = ((FilterMask<<3)|0x02)&0xFFFF;
	}
	else
	{
		can_filter.filter_mask_high = (FilterMask<<5);
		can_filter.filter_mask_low = (0|0x02);
	}
	if(FilterNum < 14)
	{
		can_filter.filter_fifo_number = CAN_FIFO0;
	}
	else
	{
		can_filter.filter_fifo_number = CAN_FIFO1;
	}
	can_filter.filter_enable = ENABLE;
	
	can_filter_init(&can_filter);
}

void can_SetFilter16IdMask(unsigned char FilterNum, unsigned short FilterId1, unsigned short FilterMask1, unsigned short FilterId2, unsigned short FilterMask2)
{
    can_filter_parameter_struct can_filter;

	if((FilterId1 > 0x7FF)||(FilterId2 > 0x7FF)||(FilterMask1 > 0x7FF)||(FilterMask2 > 0x7FF))
	{
		return;
	}
    
	can_filter.filter_number = FilterNum;
	can_filter.filter_mode = CAN_FILTERMODE_MASK;
	can_filter.filter_bits = CAN_FILTERBITS_16BIT;
	can_filter.filter_list_low = (FilterId1<<5);
	can_filter.filter_mask_low = (FilterMask1<<5)|0x10;
	can_filter.filter_list_high = (FilterId2<<5);
	can_filter.filter_mask_high = (FilterMask2<<5)|0x10;
	
	if(FilterNum < 14)
	{
		can_filter.filter_fifo_number = CAN_FIFO0;
	}
	else
	{
		can_filter.filter_fifo_number = CAN_FIFO1;
	}
	can_filter.filter_enable = ENABLE;
	
	can_filter_init(&can_filter);
}

void can_SetFilterActivation(unsigned int can_periph, ControlStatus state)
{
	can_filter_parameter_struct can_filter;
	
	can_filter.filter_enable = state;
	can_filter_init(&can_filter);

	if(can_periph == CAN0)
	{
		can_fifo_release(can_periph, CAN_FIFO0);
	}
	else
	{
		can_fifo_release(can_periph, CAN_FIFO1);
	}
}

void can_SetRecvFilter(void)
{
	can_SetFilterActivation(CAN0, DISABLE);
	can_SetFilterActivation(CAN1, DISABLE);
	
	can_SetFilter16IdList(0, 0x71, 0x72, 0x73, 0x89);/* 过滤器组0-13 */
	can_SetFilter32IdList(1, 0x15FF043A, 0x05FF013A);
	can_SetFilter32IdList(2, 0x09FF023A, 0x18FEF100);
	
	can_SetFilter16IdList(14, 0x71, 0x72, 0x73, 0x89);/* 过滤器组14-27 */
	can_SetFilter32IdList(15, 0x15FF043A, 0x05FF013A);
	can_SetFilter32IdList(16, 0x09FF023A, 0x18FEF100);
}

//IDE: CAN_FF_STANDARD,CAN_FF_EXTENDED
int can_SendData(unsigned int can_periph, unsigned int ID, unsigned int IDE, unsigned char *buf, unsigned char len)
{
	can_trasnmit_message_struct transmit_message;
	unsigned char i,transmit_mailbox;
	unsigned int timeout = 0xFFFF;

	transmit_message.tx_sfid = ID;
    transmit_message.tx_efid = ID;
    transmit_message.tx_ft = CAN_FT_DATA;
    transmit_message.tx_ff = IDE;
    transmit_message.tx_dlen = len;
	for(i=0; i<len; i++)
	{
		transmit_message.tx_data[i] = buf[i];
	}

	transmit_mailbox = can_message_transmit(can_periph, &transmit_message);
    while((CAN_TRANSMIT_OK != can_transmit_states(can_periph, transmit_mailbox)) && (0 != timeout))
	{
        timeout--;
    }
	if(timeout == 0)
	{
		return -1;
	}

	return 0;
}

void CAN0_RX0_IRQHandler(void)
{
	can_receive_message_struct receive_message;
	
    can_message_receive(CAN0, CAN_FIFO0, &receive_message);
    
    can_recv(CAN_CH1, &receive_message);
}

void CAN1_RX1_IRQHandler(void)
{
    can_receive_message_struct receive_message;
	
    can_message_receive(CAN1, CAN_FIFO1, &receive_message);
    
	can_recv(CAN_CH2, &receive_message);
}

