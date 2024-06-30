#include "includes.h"
  
void DWT_Init(void)  
{  
	DEM_CR         |= (unsigned int)DEM_CR_TRCENA;   /* Enable Cortex-M3's DWT CYCCNT reg.  */
	DWT_CYCCNT      = (unsigned int)0u;
	DWT_CR         |= (unsigned int)DWT_CR_CYCCNTENA;
}
  
// ΢����ʱ 
void DWT_DELAY_uS(unsigned int _ulDelayTime)  
{  
	unsigned int tCnt, tDelayCnt;
	unsigned int tStart;
		
	tStart = DWT_CYCCNT;                                     /* �ս���ʱ�ļ�����ֵ */
	tCnt = 0;
	tDelayCnt = _ulDelayTime * (SystemCoreClock / 1000000);	 /* ��Ҫ�Ľ����� */ 		      

	while(tCnt < tDelayCnt)
	{
		tCnt = DWT_CYCCNT - tStart; /* ��������У����������һ��32λ���������¼�������Ȼ������ȷ���� */	
	}  
} 

