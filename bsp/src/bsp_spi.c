#include "includes.h"

void spi_flash_init(void)
{
	spi_parameter_struct spi_init_struct;
	
	rcu_periph_clock_enable(RCU_GPIOA);
    rcu_periph_clock_enable(RCU_SPI0);
	
	 /* SPI0 GPIO config */
    gpio_af_set(GPIOA, GPIO_AF_5, GPIO_PIN_5 | GPIO_PIN_6 |GPIO_PIN_7);
    gpio_mode_set(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_5 | GPIO_PIN_6 |GPIO_PIN_7);
    gpio_output_options_set(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_5 | GPIO_PIN_6 |GPIO_PIN_7);
	
    /* set SPI0_NSS as GPIO*/
    gpio_mode_set(GPIOA,GPIO_MODE_OUTPUT,GPIO_PUPD_NONE,GPIO_PIN_4);
    gpio_output_options_set(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_4);

	SPI_FLASH_CS_HIGH();
	SPI_FLASH_WP_DISENABLE();
	//mode 0 | 3
	spi_init_struct.trans_mode           = SPI_TRANSMODE_FULLDUPLEX;
    spi_init_struct.device_mode          = SPI_MASTER;
    spi_init_struct.frame_size           = SPI_FRAMESIZE_8BIT;
    spi_init_struct.clock_polarity_phase = SPI_CK_PL_HIGH_PH_2EDGE;
    spi_init_struct.nss                  = SPI_NSS_SOFT;
    spi_init_struct.prescale             = SPI_PSC_32;
    spi_init_struct.endian               = SPI_ENDIAN_MSB;
    spi_init(SPI0, &spi_init_struct);

	spi_enable(SPI0);
}

unsigned char spi_flash_sendbyte(unsigned char TxData)
{
	int iTimeOut = 0;
	
	while(RESET == spi_i2s_flag_get(SPI0, SPI_FLAG_TBE))
	{
		iTimeOut++;
		if(iTimeOut > 20000)
		{
			return 0;
		}
	}
    spi_i2s_data_transmit(SPI0, TxData);
	iTimeOut = 0;
    while(RESET == spi_i2s_flag_get(SPI0, SPI_FLAG_RBNE))
	{
		iTimeOut++;
		if(iTimeOut > 20000)
		{
			return 0;
		}
	}
	return spi_i2s_data_receive(SPI0);
}

