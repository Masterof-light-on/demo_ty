#include "includes.h"
  
void DWT_Init(void)  
{  
	DEM_CR         |= (unsigned int)DEM_CR_TRCENA;   /* Enable Cortex-M3's DWT CYCCNT reg.  */
	DWT_CYCCNT      = (unsigned int)0u;
	DWT_CR         |= (unsigned int)DWT_CR_CYCCNTENA;
}
  
// 微妙延时 
void DWT_DELAY_uS(unsigned int _ulDelayTime)  
{  
	unsigned int tCnt, tDelayCnt;
	unsigned int tStart;
		
	tStart = DWT_CYCCNT;                                     /* 刚进入时的计数器值 */
	tCnt = 0;
	tDelayCnt = _ulDelayTime * (SystemCoreClock / 1000000);	 /* 需要的节拍数 */ 		      

	while(tCnt < tDelayCnt)
	{
		tCnt = DWT_CYCCNT - tStart; /* 求减过程中，如果发生第一次32位计数器重新计数，依然可以正确计算 */	
	}  
} 

