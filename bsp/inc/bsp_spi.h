#ifndef __BSP_SPI_H__
#define __BSP_SPI_H__

#define SPI_FLASH_CS_HIGH()          gpio_bit_set(GPIOA,GPIO_PIN_4)
#define SPI_FLASH_CS_LOW()           gpio_bit_reset(GPIOA,GPIO_PIN_4)

#define SPI_FLASH_WP_ENABLE()    	 //gpio_bit_set(GPIOB,GPIO_PIN_10)
#define SPI_FLASH_WP_DISENABLE() 	 //gpio_bit_reset(GPIOB,GPIO_PIN_10)


void spi_flash_init(void);
unsigned char spi_flash_sendbyte(unsigned char TxData);

#endif

