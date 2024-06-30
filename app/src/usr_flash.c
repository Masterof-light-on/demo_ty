#include "includes.h"

//SPI_FLASHдʹ��
void SPI_FLASH_WriteEnable(void)
{
	SPI_FLASH_CS_LOW();
	spi_flash_sendbyte(W25X_WriteEnable);
	SPI_FLASH_CS_HIGH();
}
//��ȡSPI_FLASH��״̬�Ĵ���
//BIT7  6   5   4   3   2   1   0
//SPR   RV  TB BP2 BP1 BP0 WEL BUSY
//SPR:Ĭ��0,״̬�Ĵ�������λ,���WPʹ��
//TB,BP2,BP1,BP0:FLASH����д��������
//WEL:дʹ������
//BUSY:æ���λ(1,æ;0,����)
//Ĭ��:0x00
//�ȴ�����
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
//��ȡflash����
void SPI_FLASH_BufferRead(unsigned int ReadAddr,unsigned char* pBuffer,unsigned short NumByteToRead)
{
	SPI_FLASH_CS_LOW();
	spi_flash_sendbyte(W25X_ReadData);
	spi_flash_sendbyte((unsigned char)(ReadAddr>>16));//����24bit��ַ 
	spi_flash_sendbyte((unsigned char)(ReadAddr>>8));
	spi_flash_sendbyte((unsigned char)(ReadAddr));
	while(NumByteToRead--)
	{
		*pBuffer = spi_flash_sendbyte(0xFF);
		pBuffer++;
	}
	SPI_FLASH_CS_HIGH();
}
//дflash,ÿҳ����д256�ֽ�
void SPI_FLASH_PageWrite(unsigned int WriteAddr,unsigned char* pBuffer,unsigned short NumByteToWrite)
{
	SPI_FLASH_WriteEnable();
	SPI_FLASH_CS_LOW();
	spi_flash_sendbyte(W25X_PageProgram);
	spi_flash_sendbyte((unsigned char)(WriteAddr>>16));//����24bit��ַ 
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
	WriteAddr:��ʼд��ĵ�ַ(24bit)
	pBuffer:���ݴ洢��
	NumByteToWrite:Ҫд����ֽ���
*/
//��ָ����ַ��ʼд��ָ�����ȵ�����,�����Զ���ҳ����
void SPI_Flash_Write_NoCheck(unsigned int WriteAddr,unsigned char* pBuffer,unsigned short NumByteToWrite)   
{ 			 		 
	unsigned short PageRemain = 0;	   
	PageRemain = 256 - WriteAddr%256; //��ҳʣ����ֽ���		 	    
	if(NumByteToWrite <= PageRemain)
		PageRemain = NumByteToWrite;//������256���ֽ�
	while(1)
	{	   
		SPI_FLASH_PageWrite(WriteAddr,pBuffer,PageRemain);
		if(NumByteToWrite == PageRemain)
			break;//д�������
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
	WriteAddr:��ʼд��ĵ�ַ(24bit)
	pBuffer:���ݴ洢��
	NumByteToWrite:Ҫд����ֽ���
*/
//�ú�������������
void SPI_FLASH_BufferWrite(unsigned int WriteAddr,unsigned char* pBuffer,unsigned short NumByteToWrite)   
{
	SPI_FLASH_SectorErase(WriteAddr>>12, WriteAddr>>12);
	SPI_Flash_Write_NoCheck(WriteAddr,pBuffer,NumByteToWrite);
} 
//������ַ 0~1023 for w25q32
//����һ������4kb
//����һ������������ʱ��:150ms
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
	
	//��ֹ����������������������
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
	//��Ƭ���
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
	//������������
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
//��ȡоƬID W25q32��ID:0XEF15
unsigned short SPI_Flash_ReadID(void)
{
	unsigned short Temp = 0;
	
	SPI_FLASH_CS_LOW();				    
	spi_flash_sendbyte(W25X_ManufactDeviceID);//���Ͷ�ȡID����	    
	spi_flash_sendbyte(0x00); 	    
	spi_flash_sendbyte(0x00); 	    
	spi_flash_sendbyte(0x00); 	 			   
	Temp |= spi_flash_sendbyte(0xFF)<<8;  
	Temp |= spi_flash_sendbyte(0xFF);	 
	SPI_FLASH_CS_HIGH();
	
	return Temp;
}

//��̱��溯��
void flash_mileage_erase(void)
{
	NorFlash_Lock(xFlashMutex);//����
	SPI_FLASH_WP_ENABLE();
	SPI_FLASH_SectorErase((PAGE_MILEINFO_START)>>12, (PAGE_MILEINFO_END)>>12);
	SPI_FLASH_WP_DISENABLE();
	NorFlash_UnLock(xFlashMutex);//����
}
void flash_mileage_write(unsigned int addr,unsigned short size,unsigned char *buf)
{
	NorFlash_Lock(xFlashMutex);//����
	SPI_FLASH_WP_ENABLE();
	SPI_Flash_Write_NoCheck(addr,buf,size);
	SPI_FLASH_WP_DISENABLE();
	NorFlash_UnLock(xFlashMutex);//����
}
void flash_mileage_read(unsigned int addr,unsigned short size,unsigned char *buf)
{
	NorFlash_Lock(xFlashMutex);//����
	SPI_FLASH_BufferRead(addr,buf,size);
	NorFlash_UnLock(xFlashMutex);//����
}
//gps��Чֵ���溯��
void flash_gpsvalid_erase(void)
{
	NorFlash_Lock(xFlashMutex);//����
	SPI_FLASH_WP_ENABLE();
	SPI_FLASH_SectorErase((PAGE_GPSDATA_START)>>12, (PAGE_GPSDATA_END)>>12);
	SPI_FLASH_WP_DISENABLE();
	NorFlash_UnLock(xFlashMutex);//����
}
void flash_gpsvalid_write(unsigned int addr,unsigned short size,unsigned char *buf)
{
	NorFlash_Lock(xFlashMutex);//����
	SPI_FLASH_WP_ENABLE();
	SPI_Flash_Write_NoCheck(addr,buf,size);
	SPI_FLASH_WP_DISENABLE();
	NorFlash_UnLock(xFlashMutex);//����
}
void flash_gpsvalid_read(unsigned int addr,unsigned short size,unsigned char *buf)
{
	NorFlash_Lock(xFlashMutex);//����
	SPI_FLASH_BufferRead(addr,buf,size);
	NorFlash_UnLock(xFlashMutex);//����
}
//ä���α걣�溯��
void flash_blindInIndex_erase(unsigned int startaddr, unsigned int endaddr)
{
	NorFlash_Lock(xFlashMutex);//����
	SPI_FLASH_WP_ENABLE();
	SPI_FLASH_SectorErase((startaddr)>>12, (endaddr)>>12);
	SPI_FLASH_WP_DISENABLE();
	NorFlash_UnLock(xFlashMutex);//����
}
void flash_blindOutIndex_erase(unsigned int startaddr, unsigned int endaddr)
{
	NorFlash_Lock(xFlashMutex);//����
	SPI_FLASH_WP_ENABLE();
	SPI_FLASH_SectorErase((startaddr)>>12, (endaddr)>>12);
	SPI_FLASH_WP_DISENABLE();
	NorFlash_UnLock(xFlashMutex);//����
}
void flash_blindIndex_write(unsigned int addr,unsigned short size,unsigned char *buf)
{
	NorFlash_Lock(xFlashMutex);//����
	SPI_FLASH_WP_ENABLE();
	SPI_Flash_Write_NoCheck(addr,buf,size);
	SPI_FLASH_WP_DISENABLE();
	NorFlash_UnLock(xFlashMutex);//����
}
void flash_blindIndex_read(unsigned int addr,unsigned short size,unsigned char *buf)
{
	NorFlash_Lock(xFlashMutex);//����
	SPI_FLASH_BufferRead(addr,buf,size);
	NorFlash_UnLock(xFlashMutex);//����
}
//ä�����ݱ���
void flash_blinddata_write(unsigned int addr,unsigned short size,unsigned char *buf)
{
	NorFlash_Lock(xFlashMutex);//����
	SPI_FLASH_WP_ENABLE();
	if((addr % 4096) == 0)
	{
		SPI_FLASH_SectorErase((addr)>>12, (addr)>>12);
	}
	SPI_Flash_Write_NoCheck(addr,buf,size);
	SPI_FLASH_WP_DISENABLE();
	NorFlash_UnLock(xFlashMutex);//����
}
void flash_blinddata_read(unsigned int addr,unsigned short size,unsigned char *buf)
{
	NorFlash_Lock(xFlashMutex);//����
	SPI_FLASH_BufferRead(addr,buf,size);
	NorFlash_UnLock(xFlashMutex);//����
}
//Զ������
void flash_fota_backup_erase(void)
{
	NorFlash_Lock(xFlashMutex);//����
	SPI_FLASH_WP_ENABLE();
	SPI_FLASH_SectorErase((PAGE_APPBACKUP_START)>>12, (PAGE_APPBACKUP_END)>>12);
	SPI_FLASH_WP_DISENABLE();
	NorFlash_UnLock(xFlashMutex);//����
}
void flash_fota_erase(void)
{
	NorFlash_Lock(xFlashMutex);//����
	SPI_FLASH_WP_ENABLE();
	SPI_FLASH_SectorErase((PAGE_UPDATE_START)>>12, (PAGE_UPDATE_END)>>12);
	SPI_FLASH_WP_DISENABLE();
	NorFlash_UnLock(xFlashMutex);//����
}
void flash_fota_write(unsigned int addr,unsigned short size,unsigned char *buf)
{
	NorFlash_Lock(xFlashMutex);//����
	SPI_FLASH_WP_ENABLE();
	SPI_Flash_Write_NoCheck(addr,buf,size);
	SPI_FLASH_WP_DISENABLE();
	NorFlash_UnLock(xFlashMutex);//����
}

void flash_Write(unsigned int addr,unsigned short size,unsigned char *buf)
{
	NorFlash_Lock(xFlashMutex);//����
	SPI_FLASH_WP_ENABLE();
	SPI_FLASH_BufferWrite(addr,buf,size);
	SPI_FLASH_WP_DISENABLE();
	NorFlash_UnLock(xFlashMutex);//����
}
void flash_Read(unsigned int addr,unsigned short size,unsigned char *buf)
{
	NorFlash_Lock(xFlashMutex);//����
	SPI_FLASH_BufferRead(addr,buf,size);
	NorFlash_UnLock(xFlashMutex);//����
}

