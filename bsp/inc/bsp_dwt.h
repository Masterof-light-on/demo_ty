#ifndef __BSP_DWT_H__
#define __BSP_DWT_H__

/*
*********************************************************************************************************
*                                             ¼Ä´æÆ÷
*********************************************************************************************************
*/
#define  DWT_CYCCNT  *(volatile unsigned int *)0xE0001004
#define  DWT_CR      *(volatile unsigned int *)0xE0001000
#define  DEM_CR      *(volatile unsigned int *)0xE000EDFC
#define  DBGMCU_CR   *(volatile unsigned int *)0xE0042004

#define  DEM_CR_TRCENA               (1 << 24)
#define  DWT_CR_CYCCNTENA            (1 <<  0)

#define delayus(uSec)	DWT_DELAY_uS(uSec)
#define delay_us(uSec)	DWT_DELAY_uS(uSec)
#define delayms(mSec)   DWT_DELAY_uS(mSec*1000)  

void DWT_Init(void);
void DWT_DELAY_uS(unsigned int _ulDelayTime);

#endif

