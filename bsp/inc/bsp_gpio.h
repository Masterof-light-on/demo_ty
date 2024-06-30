#ifndef __BSP_GPIO_H__
#define __BSP_GPIO_H__

#define PA0   100
#define PA1   101
#define PA2   102
#define PA3   103
#define PA4   104
#define PA5   105
#define PA6   106
#define PA7   107
#define PA8   108
#define PA9   109
#define PA10  110
#define PA11  111
#define PA12  112
#define PA13  113
#define PA14  114
#define PA15  115

#define PB0   200
#define PB1   201
#define PB2   202
#define PB3   203
#define PB4   204
#define PB5   205
#define PB6   206
#define PB7   207
#define PB8   208
#define PB9   209
#define PB10  210
#define PB11  211
#define PB12  212
#define PB13  213
#define PB14  214
#define PB15  215

#define PC0   300
#define PC1   301
#define PC2   302
#define PC3   303
#define PC4   304
#define PC5   305
#define PC6   306
#define PC7   307
#define PC8   308
#define PC9   309
#define PC10  310
#define PC11  311
#define PC12  312
#define PC13  313
#define PC14  314
#define PC15  315

#define PD0   400
#define PD1   401
#define PD2   402
#define PD3   403
#define PD4   404
#define PD5   405
#define PD6   406
#define PD7   407
#define PD8   408
#define PD9   409
#define PD10  410
#define PD11  411
#define PD12  412
#define PD13  413
#define PD14  414
#define PD15  415
			  
#define PE0   500
#define PE1   501
#define PE2   502
#define PE3   503
#define PE4   504
#define PE5   505
#define PE6   506
#define PE7   507
#define PE8   508
#define PE9   509
#define PE10  510
#define PE11  511
#define PE12  512
#define PE13  513
#define PE14  514
#define PE15  515

#define PF0   600
#define PF1   601
#define PF2   602
#define PF3   603
#define PF4   604
#define PF5   605
#define PF6   606
#define PF7   607
#define PF8   608
#define PF9   609
#define PF10  610
#define PF11  611
#define PF12  612
#define PF13  613
#define PF14  614
#define PF15  615

#define PG0   700
#define PG1   701
#define PG2   702
#define PG3   703
#define PG4   704
#define PG5   705
#define PG6   706
#define PG7   707
#define PG8   708
#define PG9   709
#define PG10  710
#define PG11  711
#define PG12  712
#define PG13  713
#define PG14  714
#define PG15  715

void gpio_dir(unsigned short pin,unsigned char dir);
void gpio_set(unsigned short pin,unsigned char val);
unsigned char gpio_val(unsigned short pin);
unsigned char gpio_valo(unsigned short pin);
#endif

