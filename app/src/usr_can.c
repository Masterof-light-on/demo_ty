#include "includes.h"

//CAN1
can_receive_message_struct Can1RxQueue[CAN_MAX_QUEUE];
RINGBUFF_T Can1RINGBUFF;

//CAN2
can_receive_message_struct Can2RxQueue[CAN_MAX_QUEUE];
RINGBUFF_T Can2RINGBUFF;

CAN_INFO g_obd;
static unsigned char CanTimeout;

void can_ringbuffer_Init(void)
{
	RingBuffer_Init(&Can1RINGBUFF, Can1RxQueue, sizeof(can_receive_message_struct), NELEMENTS(Can1RxQueue));
	RingBuffer_Init(&Can2RINGBUFF, Can2RxQueue, sizeof(can_receive_message_struct), NELEMENTS(Can2RxQueue));

	memset_t(&g_obd, 0xFF, sizeof(g_obd));
}

void can_STB_Init(void)
{

}

void can_recv(unsigned char c, can_receive_message_struct *RxMessage)
{
	if(c == CAN_CH1)
	{
		if(RingBuffer_Insert(&Can1RINGBUFF, (can_receive_message_struct *)RxMessage))
		{
			OS_SendMessageFromISR(xDevQueueId, MSG_DEV_CAN_REQ, CAN_CH1, 0);
		}
	}
	if(c == CAN_CH2)
	{
		if(RingBuffer_Insert(&Can2RINGBUFF, (can_receive_message_struct *)RxMessage))
		{
			OS_SendMessageFromISR(xDevQueueId, MSG_DEV_CAN_REQ, CAN_CH2, 0);
		}
	}
}

void can_recv_proc(unsigned char c)
{
	can_receive_message_struct outMessage;
	if(c == CAN_CH1)
	{
		RingBuffer_Pop(&Can1RINGBUFF, (can_receive_message_struct *)&outMessage);
		//测试标志
		gc_can1ok   = 1;
		gc_can1time = 0;
		
		LOG_EXT("[can1] %08x,%04x ", outMessage.rx_efid,outMessage.rx_sfid);
		LOGX_EXT(outMessage.rx_data, 8);
		LOG_EXT("\r\n");

		can_engine_check();
		can_stddata_proc(outMessage.rx_sfid, outMessage.rx_data);
		can_extdata_proc(outMessage.rx_efid, outMessage.rx_data);
	}
	if(c == CAN_CH2)
	{
		RingBuffer_Pop(&Can2RINGBUFF, (can_receive_message_struct *)&outMessage);

		//测试标志
		gc_can2ok   = 1;
		gc_can2time = 0;
		
		LOG_EXT("[can2] %08x,%04x ", outMessage.rx_efid,outMessage.rx_sfid);
		LOGX_EXT(outMessage.rx_data, 8);
		LOG_EXT("\r\n");

		can_engine_check();
		can_stddata_proc(outMessage.rx_sfid, outMessage.rx_data);
		can_extdata_proc(outMessage.rx_efid, outMessage.rx_data);
	}
}

void can_engine_check(void)
{
	if(((g_obd.VSS > 0)&&(g_obd.VSS != 0xFFFF))||((g_obd.RPM > 0)&&(g_obd.RPM != 0xFFFF)))
	{
		CanTimeout = 10;
		sys_state_obd(1);
	}
}

void can_stddata_proc(unsigned int id, unsigned char *data)
{
	switch(id)
	{
		case 0x71:
			break;
		default:
			break;
	}
}

void can_extdata_proc(unsigned int id, unsigned char *data)
{
	switch(id)
	{
		case 0x15FF043A:
			break;
		default:
			break;
	}
}

void can_timeout(void)
{
	if(CanTimeout)
	{
		CanTimeout --;
		if(CanTimeout == 0)
		{
			memset_t(&g_obd, 0xFF, sizeof(g_obd));
			sys_state_obd(0);
		}
	}
}

unsigned char can_getTimeout(void)
{
	return CanTimeout;
}

/******************************************************************************/


