#ifndef __USR_FALSH_H__
#define __USR_FALSH_H__

#define FLASH_Max_Sector			  1023

#define W25X_WriteEnable		      0x06 
#define W25X_WriteDisable		      0x04 
#define W25X_ReadStatusReg		      0x05 
#define W25X_WriteStatusReg		      0x01 
#define W25X_ReadData			      0x03 
#define W25X_FastReadData		      0x0B 
#define W25X_FastReadDual		      0x3B 
#define W25X_PageProgram		      0x02 
#define W25X_BlockErase			      0xD8 
#define W25X_SectorErase		      0x20 
#define W25X_ChipErase			      0xC7 
#define W25X_PowerDown			      0xB9 
#define W25X_ReleasePowerDown	      0xAB 
#define W25X_DeviceID			      0xAB 
#define W25X_ManufactDeviceID         0x90 
#define W25X_JedecDeviceID		      0x9F

/*外部flash分配*/
#define PAGE_SYSCONFIG			(0<<12)		// 4kb系统参数
#define PAGE_RUNTIME			(1<<12)		// 4kb系统参数
#define PAGE_SNINFO				(2<<12)		// 4kb SN
#define PAGE_RESV				(3<<12)		// 4kb保留

#define PAGE_UPDATE_START		(4<<12)		//200kb升级
#define PAGE_UPDATE_END			(54<<12)

#define PAGE_APPBACKUP_START	(55<<12)	//200kb备份程序
#define PAGE_APPBACKUP_END		(105<<12)

#define PAGE_GPSDATA_START		(106<<12)	//100kb GPS数据
#define PAGE_GPSDATA_END		(130<<12)

#define PAGE_IN_GSMBLIND_START	(131<<12)  	//64k盲区进游标
#define PAGE_IN_GSMBLIND_END	(146<<12)

#define PAGE_OUT_GSMBLIND_START	(147<<12)  	//64k盲区出游标
#define PAGE_OUT_GSMBLIND_END	(162<<12)

#define PAGE_MILEINFO_START		(195<<12)	//64k保存里程值
#define PAGE_MILEINFO_END		(210<<12)

#define PAGE_GSMBLIND_START		(211<<12)	//1250kb盲区
#define PAGE_GSMBLIND_END		(367<<12)

void SPI_FLASH_BufferRead(unsigned int ReadAddr,unsigned char* pBuffer,unsigned short NumByteToRead);
void SPI_FLASH_BufferWrite(unsigned int WriteAddr,unsigned char* pBuffer,unsigned short NumByteToWrite);
void SPI_FLASH_SectorErase(unsigned int secStart, unsigned int secEnd);
unsigned short SPI_Flash_ReadID(void);

void flash_mileage_erase(void);
void flash_mileage_write(unsigned int addr,unsigned short size,unsigned char *buf);
void flash_mileage_read(unsigned int addr,unsigned short size,unsigned char *buf);
void flash_gpsvalid_erase(void);
void flash_gpsvalid_write(unsigned int addr,unsigned short size,unsigned char *buf);
void flash_gpsvalid_read(unsigned int addr,unsigned short size,unsigned char *buf);
void flash_blindInIndex_erase(unsigned int startaddr, unsigned int endaddr);
void flash_blindOutIndex_erase(unsigned int startaddr, unsigned int endaddr);
void flash_blindIndex_write(unsigned int addr,unsigned short size,unsigned char *buf);
void flash_blindIndex_read(unsigned int addr,unsigned short size,unsigned char *buf);
void flash_blinddata_write(unsigned int addr,unsigned short size,unsigned char *buf);
void flash_blinddata_read(unsigned int addr,unsigned short size,unsigned char *buf);

void flash_fota_backup_erase(void);
void flash_fota_erase(void);
void flash_fota_write(unsigned int addr,unsigned short size,unsigned char *buf);

void flash_Write(unsigned int addr,unsigned short size,unsigned char *buf);
void flash_Read(unsigned int addr,unsigned short size,unsigned char *buf);

#endif

