#ifndef __BSP_IAP_H__
#define __BSP_IAP_H__

/* base address of the FMC sectors */
#define ADDR_FMC_SECTOR_0     ((uint32_t)0x08000000) /*!< base address of sector 0, 16 kbytes */
#define ADDR_FMC_SECTOR_1     ((uint32_t)0x08004000) /*!< base address of sector 1, 16 kbytes */
#define ADDR_FMC_SECTOR_2     ((uint32_t)0x08008000) /*!< base address of sector 2, 16 kbytes */
#define ADDR_FMC_SECTOR_3     ((uint32_t)0x0800C000) /*!< base address of sector 3, 16 kbytes */
#define ADDR_FMC_SECTOR_4     ((uint32_t)0x08010000) /*!< base address of sector 4, 64 kbytes */
#define ADDR_FMC_SECTOR_5     ((uint32_t)0x08020000) /*!< base address of sector 5, 128 kbytes */
#define ADDR_FMC_SECTOR_6     ((uint32_t)0x08040000) /*!< base address of sector 6, 128 kbytes */
#define ADDR_FMC_SECTOR_7     ((uint32_t)0x08060000) /*!< base address of sector 7, 128 kbytes */
#define ADDR_FMC_SECTOR_8     ((uint32_t)0x08080000) /*!< base address of sector 8, 128 kbytes */
#define ADDR_FMC_SECTOR_9     ((uint32_t)0x080A0000) /*!< base address of sector 9, 128 kbytes */
#define ADDR_FMC_SECTOR_10    ((uint32_t)0x080C0000) /*!< base address of sector 10, 128 kbytes */
#define ADDR_FMC_SECTOR_11    ((uint32_t)0x080E0000) /*!< base address of sector 11, 128 kbytes */

void fmc_erase_sector_by_number(uint32_t sector_num);
void fmc_write_data(uint32_t address, uint16_t length, uint8_t* data_8);
void fmc_write_data_noerase(uint32_t address, uint16_t length, uint8_t* data_8);
void fmc_read_data(uint32_t address, uint16_t length, uint8_t* data_8);

#endif

