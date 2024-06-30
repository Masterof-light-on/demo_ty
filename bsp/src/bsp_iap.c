#include "includes.h"

uint32_t fmc_sector_get(uint32_t address)
{
    uint32_t sector = 0;
    if((address < ADDR_FMC_SECTOR_1) && (address >= ADDR_FMC_SECTOR_0)){
        sector = CTL_SECTOR_NUMBER_0;
    }else if((address < ADDR_FMC_SECTOR_2) && (address >= ADDR_FMC_SECTOR_1)){
        sector = CTL_SECTOR_NUMBER_1;
    }else if((address < ADDR_FMC_SECTOR_3) && (address >= ADDR_FMC_SECTOR_2)){
        sector = CTL_SECTOR_NUMBER_2;
    }else if((address < ADDR_FMC_SECTOR_4) && (address >= ADDR_FMC_SECTOR_3)){
        sector = CTL_SECTOR_NUMBER_3;  
    }else if((address < ADDR_FMC_SECTOR_5) && (address >= ADDR_FMC_SECTOR_4)){
        sector = CTL_SECTOR_NUMBER_4;
    }else if((address < ADDR_FMC_SECTOR_6) && (address >= ADDR_FMC_SECTOR_5)){
        sector = CTL_SECTOR_NUMBER_5;
    }else if((address < ADDR_FMC_SECTOR_7) && (address >= ADDR_FMC_SECTOR_6)){
        sector = CTL_SECTOR_NUMBER_6;
    }else if((address < ADDR_FMC_SECTOR_8) && (address >= ADDR_FMC_SECTOR_7)){
        sector = CTL_SECTOR_NUMBER_7;  
    }else if((address < ADDR_FMC_SECTOR_9) && (address >= ADDR_FMC_SECTOR_8)){
        sector = CTL_SECTOR_NUMBER_8;
    }else if((address < ADDR_FMC_SECTOR_10) && (address >= ADDR_FMC_SECTOR_9)){
        sector = CTL_SECTOR_NUMBER_9;
    }else if((address < ADDR_FMC_SECTOR_11) && (address >= ADDR_FMC_SECTOR_10)){
        sector = CTL_SECTOR_NUMBER_10;
    }else{
        sector = CTL_SECTOR_NUMBER_11;
    }
    return sector;
}


uint32_t sector_number(uint32_t sector_num)
{
    if(11 >= sector_num){
        return CTL_SN(sector_num);
    }else if(23 >= sector_num){
        return CTL_SN(sector_num + 4);
    }else /*if(27 >= sector_num)*/{
        return CTL_SN(sector_num - 12);
    }
}

void fmc_erase_sector_by_number(uint32_t sector_num)
{
    /* unlock the flash program erase controller */
    fmc_unlock(); 
    /* clear pending flags */
    fmc_flag_clear(FMC_FLAG_END | FMC_FLAG_OPERR | FMC_FLAG_WPERR | FMC_FLAG_PGMERR | FMC_FLAG_PGSERR);
    /* wait the erase operation complete*/
    if(FMC_READY != fmc_sector_erase(sector_number(sector_num))){
		PRINTF("[fmc] fmc_erase_sector_by_number err\r\n");
        return;
    }
    /* lock the flash program erase controller */
    fmc_lock();
}

void fmc_write_data(uint32_t address, uint16_t length, uint8_t* data_8)
{
	uint16_t i;
	
    /* unlock the flash program erase controller */
    fmc_unlock();
    /* clear pending flags */
    fmc_flag_clear(FMC_FLAG_END | FMC_FLAG_OPERR | FMC_FLAG_WPERR | FMC_FLAG_PGMERR | FMC_FLAG_PGSERR);
    /* wait the erase operation complete*/
    if(FMC_READY != fmc_sector_erase(fmc_sector_get(address))){
		PRINTF("[fmc] fmc_sector_erase err\r\n");
        return;
    }

    /* write data_8 to the corresponding address */
    for(i=0; i<length; i++){
        if(FMC_READY == fmc_byte_program(address, data_8[i])){
            address++;
        }else{
        	PRINTF("[fmc] fmc_write_data err\r\n");
            break;
        }
    }
    /* lock the flash program erase controller */
    fmc_lock();
}

void fmc_write_data_noerase(uint32_t address, uint16_t length, uint8_t* data_8)
{
    uint16_t i;
    
    /* unlock the flash program erase controller */
    fmc_unlock();
    /* clear pending flags */
    fmc_flag_clear(FMC_FLAG_END | FMC_FLAG_OPERR | FMC_FLAG_WPERR | FMC_FLAG_PGMERR | FMC_FLAG_PGSERR);

    /* write data_8 to the corresponding address */
    for(i=0; i<length; i++){
        if(FMC_READY == fmc_byte_program(address, data_8[i])){
            address++;
        }else{
        	PRINTF("[fmc] fmc_write_data_noerase err\r\n");
            break;
        }
    }
    /* lock the flash program erase controller */
    fmc_lock();
}

void fmc_read_data(uint32_t address, uint16_t length, uint8_t* data_8)
{
    uint16_t i;
	
    for(i=0; i<length; i++){
        data_8[i] = *(__IO uint8_t*)address;
        address++;
    }
}

