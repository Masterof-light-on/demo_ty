#include "includes.h"

//SPI_FLASH写使能
void SPI_FLASH_WriteEnable(void)
{
	SPI_FLASH_CS_LOW();
	spi_flash_sendbyte(W25X_WriteEnable);
	SPI_FLASH_CS_HIGH();
}
//读取SPI_FLASH的状态寄存器
//BIT7  6   5   4   3   2   1   0
//SPR   RV  TB BP2 BP1 BP0 WEL BUSY
//SPR:默认0,状态寄存器保护位,配合WP使用
//TB,BP2,BP1,BP0:FLASH区域写保护设置
//WEL:写使能锁定
//BUSY:忙标记位(1,忙;0,空闲)
//默认:0x00
//等待空闲
void SPI_Flash_Wait_Busy(void)
{
	unsigned char FLASH_Status = 0;
	SPI_FLASH_CS_LOW();
	spi_flash_sendbyte(W25X_ReadStatusReg);
	do
	{
		FLASH_Status = spi_flash_sendbyte(0xFF);
	}while((FLASH_Status & 0x01) == 0x01); //Check Busy
	SPI_FLASH_CS_HIGH();
}
//读取flash内容
void SPI_FLASH_BufferRead(unsigned int ReadAddr,unsigned char* pBuffer,unsigned short NumByteToRead)
{
	SPI_FLASH_CS_LOW();
	spi_flash_sendbyte(W25X_ReadData);
	spi_flash_sendbyte((unsigned char)(ReadAddr>>16));//发送24bit地址 
	spi_flash_sendbyte((unsigned char)(ReadAddr>>8));
	spi_flash_sendbyte((unsigned char)(ReadAddr));
	while(NumByteToRead--)
	{
		*pBuffer = spi_flash_sendbyte(0xFF);
		pBuffer++;
	}
	SPI_FLASH_CS_HIGH();
}
//写flash,每页最大可写256字节
void SPI_FLASH_PageWrite(unsigned int WriteAddr,unsigned char* pBuffer,unsigned short NumByteToWrite)
{
	SPI_FLASH_WriteEnable();
	SPI_FLASH_CS_LOW();
	spi_flash_sendbyte(W25X_PageProgram);
	spi_flash_sendbyte((unsigned char)(WriteAddr>>16));//发送24bit地址 
	spi_flash_sendbyte((unsigned char)(WriteAddr>>8));
	spi_flash_sendbyte((unsigned char)(WriteAddr));
	while(NumByteToWrite--)
	{
		spi_flash_sendbyte(*pBuffer);
		pBuffer++;
	}
	SPI_FLASH_CS_HIGH();
	SPI_Flash_Wait_Busy();
}
/*	
	WriteAddr:开始写入的地址(24bit)
	pBuffer:数据存储区
	NumByteToWrite:要写入的字节数
*/
//在指定地址开始写入指定长度的数据,具有自动换页功能
void SPI_Flash_Write_NoCheck(unsigned int WriteAddr,unsigned char* pBuffer,unsigned short NumByteToWrite)   
{ 			 		 
	unsigned short PageRemain = 0;	   
	PageRemain = 256 - WriteAddr%256; //单页剩余的字节数		 	    
	if(NumByteToWrite <= PageRemain)
		PageRemain = NumByteToWrite;//不大于256个字节
	while(1)
	{	   
		SPI_FLASH_PageWrite(WriteAddr,pBuffer,PageRemain);
		if(NumByteToWrite == PageRemain)
			break;//写入结束了
	 	else //NumByteToWrite>pageremain
		{
			pBuffer += PageRemain;
			WriteAddr += PageRemain;	
			NumByteToWrite -= PageRemain;

			if(NumByteToWrite > 256)
				PageRemain = 256;
			else 
				PageRemain = NumByteToWrite;
		}
	}	    
}
/*	
	WriteAddr:开始写入的地址(24bit)
	pBuffer:数据存储区
	NumByteToWrite:要写入的字节数
*/
//该函数带擦除操作
void SPI_FLASH_BufferWrite(unsigned int WriteAddr,unsigned char* pBuffer,unsigned short NumByteToWrite)   
{
	SPI_FLASH_SectorErase(WriteAddr>>12, WriteAddr>>12);
	SPI_Flash_Write_NoCheck(WriteAddr,pBuffer,NumByteToWrite);
} 
//扇区地址 0~1023 for w25q32
//擦除一个扇区4kb
//擦除一个扇区的最少时间:150ms
void SPI_FLASH_SectorErase(unsigned int secStart, unsigned int secEnd)
{	
	unsigned int ErAddr;
	unsigned int no_SecToEr;
	unsigned int CurSecToEr;
	
	if((secStart > FLASH_Max_Sector)||(secEnd > FLASH_Max_Sector))
	{
		return;
	}
	if(secStart > secEnd)
	{
		return;
	}
	
	//起止扇区号相等则擦除单个扇区
	if(secStart == secEnd)
	{
		ErAddr = secStart<<12;
		SPI_FLASH_WriteEnable();
		SPI_Flash_Wait_Busy();
		SPI_FLASH_CS_LOW();
	    spi_flash_sendbyte(W25X_SectorErase);
	    spi_flash_sendbyte(((ErAddr & 0xFF0000) >> 16)); 
	    spi_flash_sendbyte(((ErAddr & 0xFF00) >> 8));  
	    spi_flash_sendbyte(ErAddr & 0xFF);  
	    SPI_FLASH_CS_HIGH();
		SPI_Flash_Wait_Busy();
		
		return;
	}
	//整片拆除
	if(secEnd - secStart == FLASH_Max_Sector)
	{
		SPI_FLASH_WriteEnable();
		SPI_Flash_Wait_Busy();
		SPI_FLASH_CS_LOW();
	    spi_flash_sendbyte(W25X_ChipErase); 
	    SPI_FLASH_CS_HIGH();
		SPI_Flash_Wait_Busy();
		
		return;
	}
	//计算扇区个数
	no_SecToEr = secEnd - secStart + 1;
	CurSecToEr = secStart;
	while(no_SecToEr >= 1)
	{
		ErAddr = CurSecToEr<<12;
		SPI_FLASH_WriteEnable();
		SPI_Flash_Wait_Busy();
		SPI_FLASH_CS_LOW();
	    spi_flash_sendbyte(W25X_SectorErase);
	    spi_flash_sendbyte(((ErAddr & 0xFF0000) >> 16)); 
	    spi_flash_sendbyte(((ErAddr & 0xFF00) >> 8));  
	    spi_flash_sendbyte(ErAddr & 0xFF);  
	    SPI_FLASH_CS_HIGH();
		SPI_Flash_Wait_Busy();

		CurSecToEr += 1;
		no_SecToEr -= 1;
	}
}
//读取芯片ID W25q32的ID:0XEF15
unsigned short SPI_Flash_ReadID(void)
{
	unsigned short Temp = 0;
	
	SPI_FLASH_CS_LOW();				    
	spi_flash_sendbyte(W25X_ManufactDeviceID);//发送读取ID命令	    
	spi_flash_sendbyte(0x00); 	    
	spi_flash_sendbyte(0x00); 	    
	spi_flash_sendbyte(0x00); 	 			   
	Temp |= spi_flash_sendbyte(0xFF)<<8;  
	Temp |= spi_flash_sendbyte(0xFF);	 
	SPI_FLASH_CS_HIGH();
	
	return Temp;
}

//里程保存函数
void flash_mileage_erase(void)
{
	NorFlash_Lock(xFlashMutex);//加锁
	SPI_FLASH_WP_ENABLE();
	SPI_FLASH_SectorErase((PAGE_MILEINFO_START)>>12, (PAGE_MILEINFO_END)>>12);
	SPI_FLASH_WP_DISENABLE();
	NorFlash_UnLock(xFlashMutex);//解锁
}
void flash_mileage_write(unsigned int addr,unsigned short size,unsigned char *buf)
{
	NorFlash_Lock(xFlashMutex);//加锁
	SPI_FLASH_WP_ENABLE();
	SPI_Flash_Write_NoCheck(addr,buf,size);
	SPI_FLASH_WP_DISENABLE();
	NorFlash_UnLock(xFlashMutex);//解锁
}
void flash_mileage_read(unsigned int addr,unsigned short size,unsigned char *buf)
{
	NorFlash_Lock(xFlashMutex);//加锁
	SPI_FLASH_BufferRead(addr,buf,size);
	NorFlash_UnLock(xFlashMutex);//解锁
}
//gps有效值保存函数
void flash_gpsvalid_erase(void)
{
	NorFlash_Lock(xFlashMutex);//加锁
	SPI_FLASH_WP_ENABLE();
	SPI_FLASH_SectorErase((PAGE_GPSDATA_START)>>12, (PAGE_GPSDATA_END)>>12);
	SPI_FLASH_WP_DISENABLE();
	NorFlash_UnLock(xFlashMutex);//解锁
}
void flash_gpsvalid_write(unsigned int addr,unsigned short size,unsigned char *buf)
{
	NorFlash_Lock(xFlashMutex);//加锁
	SPI_FLASH_WP_ENABLE();
	SPI_Flash_Write_NoCheck(addr,buf,size);
	SPI_FLASH_WP_DISENABLE();
	NorFlash_UnLock(xFlashMutex);//解锁
}
void flash_gpsvalid_read(unsigned int addr,unsigned short size,unsigned char *buf)
{
	NorFlash_Lock(xFlashMutex);//加锁
	SPI_FLASH_BufferRead(addr,buf,size);
	NorFlash_UnLock(xFlashMutex);//解锁
}
//盲区游标保存函数
void flash_blindInIndex_erase(unsigned int startaddr, unsigned int endaddr)
{
	NorFlash_Lock(xFlashMutex);//加锁
	SPI_FLASH_WP_ENABLE();
	SPI_FLASH_SectorErase((startaddr)>>12, (endaddr)>>12);
	SPI_FLASH_WP_DISENABLE();
	NorFlash_UnLock(xFlashMutex);//解锁
}
void flash_blindOutIndex_erase(unsigned int startaddr, unsigned int endaddr)
{
	NorFlash_Lock(xFlashMutex);//加锁
	SPI_FLASH_WP_ENABLE();
	SPI_FLASH_SectorErase((startaddr)>>12, (endaddr)>>12);
	SPI_FLASH_WP_DISENABLE();
	NorFlash_UnLock(xFlashMutex);//解锁
}
void flash_blindIndex_write(unsigned int addr,unsigned short size,unsigned char *buf)
{
	NorFlash_Lock(xFlashMutex);//加锁
	SPI_FLASH_WP_ENABLE();
	SPI_Flash_Write_NoCheck(addr,buf,size);
	SPI_FLASH_WP_DISENABLE();
	NorFlash_UnLock(xFlashMutex);//解锁
}
void flash_blindIndex_read(unsigned int addr,unsigned short size,unsigned char *buf)
{
	NorFlash_Lock(xFlashMutex);//加锁
	SPI_FLASH_BufferRead(addr,buf,size);
	NorFlash_UnLock(xFlashMutex);//解锁
}
//盲区数据保存
void flash_blinddata_write(unsigned int addr,unsigned short size,unsigned char *buf)
{
	NorFlash_Lock(xFlashMutex);//加锁
	SPI_FLASH_WP_ENABLE();
	if((addr % 4096) == 0)
	{
		SPI_FLASH_SectorErase((addr)>>12, (addr)>>12);
	}
	SPI_Flash_Write_NoCheck(addr,buf,size);
	SPI_FLASH_WP_DISENABLE();
	NorFlash_UnLock(xFlashMutex);//解锁
}
void flash_blinddata_read(unsigned int addr,unsigned short size,unsigned char *buf)
{
	NorFlash_Lock(xFlashMutex);//加锁
	SPI_FLASH_BufferRead(addr,buf,size);
	NorFlash_UnLock(xFlashMutex);//解锁
}
//远程升级
void flash_fota_backup_erase(void)
{
	NorFlash_Lock(xFlashMutex);//加锁
	SPI_FLASH_WP_ENABLE();
	SPI_FLASH_SectorErase((PAGE_APPBACKUP_START)>>12, (PAGE_APPBACKUP_END)>>12);
	SPI_FLASH_WP_DISENABLE();
	NorFlash_UnLock(xFlashMutex);//解锁
}
void flash_fota_erase(void)
{
	NorFlash_Lock(xFlashMutex);//加锁
	SPI_FLASH_WP_ENABLE();
	SPI_FLASH_SectorErase((PAGE_UPDATE_START)>>12, (PAGE_UPDATE_END)>>12);
	SPI_FLASH_WP_DISENABLE();
	NorFlash_UnLock(xFlashMutex);//解锁
}
void flash_fota_write(unsigned int addr,unsigned short size,unsigned char *buf)
{
	NorFlash_Lock(xFlashMutex);//加锁
	SPI_FLASH_WP_ENABLE();
	SPI_Flash_Write_NoCheck(addr,buf,size);
	SPI_FLASH_WP_DISENABLE();
	NorFlash_UnLock(xFlashMutex);//解锁
}

void flash_Write(unsigned int addr,unsigned short size,unsigned char *buf)
{
	NorFlash_Lock(xFlashMutex);//加锁
	SPI_FLASH_WP_ENABLE();
	SPI_FLASH_BufferWrite(addr,buf,size);
	SPI_FLASH_WP_DISENABLE();
	NorFlash_UnLock(xFlashMutex);//解锁
}
void flash_Read(unsigned int addr,unsigned short size,unsigned char *buf)
{
	NorFlash_Lock(xFlashMutex);//加锁
	SPI_FLASH_BufferRead(addr,buf,size);
	NorFlash_UnLock(xFlashMutex);//解锁
}

