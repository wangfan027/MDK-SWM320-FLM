

#include "SWM320.h"
#include "w25q64.h"

uint8_t SectorBuf[0x1000] __attribute((aligned (4))) ;

#define W25Q64_CSLOW	GPIO_ClrBit(GPIOP, PIN22)
#define W25Q64_CSHIGH	GPIO_SetBit(GPIOP,PIN22)

void W25Q64_InitPort(void)
{
	SPI_InitStructure SPI_initStruct;

	GPIO_Init(GPIOP, PIN22, 1, 0, 0);
	W25Q64_CSHIGH;
	
	PORT_Init(PORTP, PIN23, FUNMUX1_SPI0_SCLK, 0);
	PORT_Init(PORTP, PIN18, FUNMUX0_SPI0_MOSI, 0);
	PORT_Init(PORTP, PIN19, FUNMUX1_SPI0_MISO, 1);
//	PORT_Init(PORTP, PIN22, FUNMUX0_SPI0_SSEL, 0);
	
	SPI_initStruct.clkDiv = SPI_CLKDIV_4;
	SPI_initStruct.FrameFormat = SPI_FORMAT_SPI;
	SPI_initStruct.SampleEdge = SPI_FIRST_EDGE;
	SPI_initStruct.IdleLevel = SPI_LOW_LEVEL;
	SPI_initStruct.WordSize = 8;
	SPI_initStruct.Master = 1;
	SPI_initStruct.RXHFullIEn = 0;
	SPI_initStruct.TXEmptyIEn = 0;
	SPI_initStruct.TXCompleteIEn = 0;
	SPI_Init(SPI0, &SPI_initStruct);
	
	SPI_Open(SPI0);
}

void W25Q64_Init(void)
{
	uint32_t chip_id;
	W25Q64_InitPort();
	chip_id = W25Q64_ReadDeviceID();
	if(chip_id!=W25Q64_ID)
	{
//		printf("This Chip(Manufactor ID: %02X, Device ID: %02X) is not W25Q64\r\n", manufacture_id, device_id);
	}	
}
//used spi0
uint32_t SPI_RW1Byte(uint8_t d)
{
//	SPI_WriteWithWait(SPI0,d);
//	return SPI_Read(SPI0);
	return SPI_ReadWrite(SPI0,d);
}

//--------------------------------------------------------------------------------------------------------
//	函 数 名: W25Q64_WriteEnable
//	功能说明: 写使能,置位 WEL 位 WEL 位(WEL-->1)
//--------------------------------------------------------------------------------------------------------
void W25Q64_WriteEnable(void)
{
	W25Q64_CSLOW;
	SPI_RW1Byte(W25Q64_WRITE_ENABLE_CMD);//开启写使能
	W25Q64_CSHIGH;
}

//--------------------------------------------------------------------------------------------------------
//	函 数 名: W25Q64_WriteDisable
//	功能说明: 写失能,复位 WEL 位(WEL-->0)
//--------------------------------------------------------------------------------------------------------
void W25Q64_WriteDisable(void)
{
	W25Q64_CSLOW;
	SPI_RW1Byte(W25Q64_WRITE_DISABLE_CMD);
	W25Q64_CSHIGH;
}

//--------------------------------------------------------------------------------------------------------
//	函 数 名: W25Q64_WriteSR
//	功能说明: 读状态寄存器
//--------------------------------------------------------------------------------------------------------
uint8_t W25Q64_ReadSR(void)
{
	uint8_t sta;
	W25Q64_CSLOW;
	SPI_RW1Byte(W25Q64_READ_SR_CMD);
	sta = SPI_RW1Byte(0xff);
	W25Q64_CSHIGH;
	return sta;
}

//--------------------------------------------------------------------------------------------------------
//	函 数 名: W25Q64_WriteSR
//	功能说明: 写状态寄存器
//   _ucByte: 写入状态寄存器的数值
//--------------------------------------------------------------------------------------------------------
void W25Q64_WriteSR(uint8_t _ucByte)
{
	W25Q64_WriteEnable();	
	W25Q64_WaitNobusy();
	W25Q64_CSLOW;
	SPI_RW1Byte(W25Q64_WRITE_SR_CMD);
	SPI_RW1Byte(_ucByte);
	W25Q64_CSHIGH;
}

//--------------------------------------------------------------------------------------------------------
//	函 数 名: W25Q64_WaitNobusy
//	功能说明: 检查 FLASH BUSY 位状态
//  备    注: 调用W25Q64_ReadSR(),判断状态寄存器的R0位,执行结束操作清零
//--------------------------------------------------------------------------------------------------------
void W25Q64_WaitNobusy(void)
{
	//W25Q64_READ_SR_CMD 指令的发送,有的FLASH仅需发送一次,FLASH自动回复,有的FLASH无法自动回复,需要循环一直发送等待
	while(((W25Q64_ReadSR()) & 0x01)==0x01);	//等待BUSY位清空
}

//--------------------------------------------------------------------------------------------------------
//	函 数 名: W25Q64_FastReadByte
//	功能说明: flash 都数据(快速读取：Fast read operate at the highest poossible frequency)
//	形    参: 	ucpBuffer：数据存储区首地址
//				_ulReadAddr: 要读出Flash的首地址
//				_usNByte： 要读出的字节数(最大65535B)
//--------------------------------------------------------------------------------------------------------
void W25Q64_ReadSomeBytes(uint8_t *ucpBuffer, uint32_t _ulReadAddr, uint16_t _usNByte)
{
	uint16_t lop;
	uint8_t temp_buff[3] = {0};
	W25Q64_CSLOW;
	temp_buff[0] = (uint8_t)(_ulReadAddr >> 16);
	temp_buff[1] = (uint8_t)(_ulReadAddr >> 8);
	temp_buff[2] = (uint8_t)(_ulReadAddr >> 0);
	SPI_RW1Byte(W25Q64_READ_DATA);
	SPI_RW1Byte(temp_buff[0]);
	SPI_RW1Byte(temp_buff[1]);
	SPI_RW1Byte(temp_buff[2]);

	for(lop=0;lop<_usNByte;lop++)
	{
		ucpBuffer[lop]=SPI_RW1Byte(0xff);
	}
	W25Q64_CSHIGH;
}

//--------------------------------------------------------------------------------------------------------
//	函 数 名: W25Q64_FastReadByte
//	功能说明: flash 都数据(快速读取：Fast read operate at the highest poossible frequency)
//	ucpBuffer：数据存储区首地址
//	_ulReadAddr: 要读出Flash的首地址
//	_usNByte： 要读出的字节数(最大65535B)
//  备    注: 从_ulReadAddr地址,连续读出_usNByte长度的字节
//--------------------------------------------------------------------------------------------------------
void W25Q64_FastReadByte(uint8_t *ucpBuffer, uint32_t _ulReadAddr, uint16_t _usNByte)
{
	uint16_t lop;
	uint8_t temp_buff[3] = {0};
	W25Q64_CSLOW;
	temp_buff[0] = (uint8_t)(_ulReadAddr >> 16);
	temp_buff[1] = (uint8_t)(_ulReadAddr >> 8);
	temp_buff[2] = (uint8_t)(_ulReadAddr >> 0);
	SPI_RW1Byte(W25Q64_FASTREAD_DATA);
	SPI_RW1Byte(temp_buff[0]);
	SPI_RW1Byte(temp_buff[1]);
	SPI_RW1Byte(temp_buff[2]);

	for(lop=0;lop<_usNByte;lop++)
	{
		ucpBuffer[lop]=SPI_RW1Byte(0xff);
	}
	W25Q64_CSHIGH;
}

//--------------------------------------------------------------------------------------------------------
//	函 数 名: W25Q64_WritePage
//	功能说明: flash 写数据(按页写入,一页256字节,写入之前FLASH地址上必须为0xFF)
//	ucpBuffer：数据存储区首地址
//	_ulWriteAddr: 要读写入Flash的首地址
//	_usNByte： 要写入的字节数(最大65535B = 64K 块)
//  备    注: _ulWriteAddr,连续写入_usNByte长度的字节
//--------------------------------------------------------------------------------------------------------
void W25Q64_WritePage(uint8_t *ucpBuffer, uint32_t _ulWriteAddr, uint16_t _usNByte)
{
	uint16_t lop;
	uint8_t temp_buff[3] = {0};

	W25Q64_WriteEnable();	//写使能
	W25Q64_WaitNobusy();	//等待写入结束
	
	W25Q64_CSLOW;
	temp_buff[0] = (uint8_t)(_ulWriteAddr >> 16);
	temp_buff[1] = (uint8_t)(_ulWriteAddr >> 8);
	temp_buff[2] = (uint8_t)(_ulWriteAddr >> 0);	
	
	SPI_RW1Byte(W25Q64_WRITE_PAGE);
	SPI_RW1Byte(temp_buff[0]);
	SPI_RW1Byte(temp_buff[1]);
	SPI_RW1Byte(temp_buff[2]);
	
	for(lop=0;lop<_usNByte;lop++)
	{
		  SPI_RW1Byte(ucpBuffer[lop]);
	}
	
	W25Q64_CSHIGH;
	
	W25Q64_WaitNobusy();	//等待写入结束
}

//--------------------------------------------------------------------------------------------------------
//	函 数 名: W25Q64_WriteNoCheck
//	功能说明: flash 写数据(不带擦除,写入之前必须确保写入部分FLASH的数据全为0xFf,否则写入失败)
//	ucpBuffer：数据存储区首地址
//	_ulWriteAddr: 要读写入Flash的首地址
//	_usNByte： 要写入的字节数(最大65535B = 64K 块)
//--------------------------------------------------------------------------------------------------------
void W25Q64_WriteNoCheck(uint8_t *ucpBuffer, uint32_t _ulWriteAddr, uint16_t _usNByte)
{
	uint16_t PageByte = 256 - _ulWriteAddr % 256;//单页剩余可写字节数
	if(_usNByte <= PageByte)	//不大于256字节
	{
		PageByte = _usNByte;
	}	
	while(1)
	{
		W25Q64_WritePage(ucpBuffer, _ulWriteAddr, PageByte);
		if(_usNByte == PageByte)	//写入结束
			break;
		else
		{
			ucpBuffer += PageByte;	//下一页写入的数据
			_ulWriteAddr += PageByte;	//下一页写入的地址
			_usNByte -= PageByte;	//待写入的字节数递减
			if(_usNByte > 256)
			{
				PageByte = 256;
			}
			else
			{
				PageByte = _usNByte;
			}
		}
	}
}

//--------------------------------------------------------------------------------------------------------
//	函 数 名: W25Q64_WriteSomeBytes
//	功能说明: flash 写数据
//	ucpBuffer：数据存储区首地址
//	_ulWriteAddr: 要读写入Flash的首地址
//	_usNByte： 要写入的字节数(最大65535B = 64K 块)
//  备    注: _ulWriteAddr,连续写入_usNByte长度的字节，程序带FLASH数据检查写入
//--------------------------------------------------------------------------------------------------------
void W25Q64_WriteSomeBytes(uint8_t *ucpBuffer, uint32_t _ulWriteAddr, uint16_t _usNByte)
{
	uint32_t ulSecPos = 0;				//得到扇区位置
	uint16_t usSecOff = 0;				//扇区偏移
	uint16_t usSecRemain = 0;		//剩余扇区
	uint32_t i = 0;
	ulSecPos = _ulWriteAddr / 4096;//地址所在扇区(0--511)
	usSecOff = _ulWriteAddr % 4096;//扇区内地址偏移
	usSecRemain = 4096 - usSecOff;//扇区除去偏移，还剩多少字节

	if(_usNByte <= usSecRemain)	//写入数据大小 < 剩余扇区空间大小
	{
		usSecRemain = _usNByte;
	}

	while(1)
	{
		W25Q64_ReadSomeBytes(SectorBuf, ulSecPos*4096, 4096);//读出整个扇区的内容
		for (i = 0; i < usSecRemain; i++)	//校验数据
		{
			if (SectorBuf[usSecOff + i] != 0xFF)//储存数据不为0xFF，需要擦除
				break;
		}
		
		if(i < usSecRemain)	//需要擦除
		{
			W25Q64_EraseSector4K(ulSecPos);	//擦除这个扇区
			for(i = 0; i < usSecRemain; i++)	//保存写入的数据
			{
				SectorBuf[usSecOff + i] = ucpBuffer[i];
			}
			W25Q64_WriteNoCheck(SectorBuf, ulSecPos*4096, 4096);	//写入整个扇区(扇区=老数据+新写入数据)
		}
		else
		{
			W25Q64_WriteNoCheck(ucpBuffer, _ulWriteAddr, usSecRemain);//不需要擦除,直接写入扇区
		}
		if(_usNByte == usSecRemain)	//写入结束
		{
			W25Q64_WriteDisable();
			break;
		}
		else
		{
			ulSecPos++;		//扇区地址增加1
			usSecOff = 0;		//扇区偏移归零
			ucpBuffer += usSecRemain;	//指针偏移
			_ulWriteAddr += usSecRemain;	//写地址偏移
			_usNByte -= usSecRemain;	//待写入的字节递减

			if(_usNByte > 4096)
			{
				usSecRemain = 4096;	//待写入一扇区(4096字节大小)
			}
			else
			{
				usSecRemain = _usNByte;		//待写入少于一扇区的数据
			}
		}	
	}
}

//--------------------------------------------------------------------------------------------------------
//	函 数 名: W25Q64_ErasePage
//	功能说明: flash erase page
//--------------------------------------------------------------------------------------------------------
void W25Q64_ErasePage(uint32_t _ulPageAddr)
{
	W25Q64_WriteEnable();
	W25Q64_WaitNobusy();
	
	W25Q64_CSLOW;	
	SPI_RW1Byte(W25Q64_ERASE_PAGE);	//页擦除指令
	SPI_RW1Byte((uint8_t)(_ulPageAddr>>16));	//写入24位地址
	SPI_RW1Byte((uint8_t)(_ulPageAddr>>8));
	SPI_RW1Byte((uint8_t)(_ulPageAddr>>0));
	W25Q64_CSHIGH;
	
	W25Q64_WaitNobusy();	//等待写入结束
}

//--------------------------------------------------------------------------------------------------------
//	函 数 名: W25Q64_EraseSector
//	功能说明: flash erase sector
//  备    注: 1扇区 = 4K Bytes
//--------------------------------------------------------------------------------------------------------
void W25Q64_EraseSector4K(uint32_t _ulSectorAddr)
{
	uint8_t temp_buff[3] = {0};
		
	W25Q64_WriteEnable();
	W25Q64_WaitNobusy();
	
	W25Q64_CSLOW;
	temp_buff[0] = (uint8_t)(_ulSectorAddr >> 16);
	temp_buff[1] = (uint8_t)(_ulSectorAddr >> 8);
	temp_buff[2] = (uint8_t)(_ulSectorAddr >> 0);	
	
	SPI_RW1Byte(W25Q64_ERASE_SECTOR4K);
	SPI_RW1Byte(temp_buff[0]);
	SPI_RW1Byte(temp_buff[1]);
	SPI_RW1Byte(temp_buff[2]);
	W25Q64_CSHIGH;
	
	W25Q64_WaitNobusy();	//等待写入结束
}

//--------------------------------------------------------------------------------------------------------
//	函 数 名: W25Q64_EraseBlock
//	功能说明: flash erase block 
//--------------------------------------------------------------------------------------------------------
void W25Q64_EraseBlock(uint32_t _ulBlockAddr)
{
//	_ulBlockAddr *= 65536;	//块地址,一块64K
	W25Q64_WriteEnable();
	W25Q64_WaitNobusy();
	
	W25Q64_CSLOW;
	
	SPI_RW1Byte(W25Q64_ERASE_BLOCK);
	SPI_RW1Byte((uint8_t)(_ulBlockAddr>>16));
	SPI_RW1Byte((uint8_t)(_ulBlockAddr>>8));
	SPI_RW1Byte((uint8_t)(_ulBlockAddr>>0));
	
	W25Q64_CSHIGH;	
	W25Q64_WaitNobusy();	//等待写入结束
}

//--------------------------------------------------------------------------------------------------------
//	函 数 名: W25Q64_EraseChip
//	功能说明: flash erase chip , it makes flash  recovery FF
//--------------------------------------------------------------------------------------------------------
void W25Q64_EraseChip(void)
{
	W25Q64_WriteEnable();	//flash芯片写使能
	W25Q64_WaitNobusy();	//等待写操作完成
	
	W25Q64_CSLOW;
	
	SPI_RW1Byte(W25Q64_ERASE_CHIP);
	
	W25Q64_CSHIGH;
	
	W25Q64_WaitNobusy();	//等待写入结束

}

//--------------------------------------------------------------------------------------------------------
//	函 数 名: W25Q64_PowerDown
//	功能说明: flash into power down mode 
//--------------------------------------------------------------------------------------------------------
void W25Q64_PowerDown(void)
{
	W25Q64_CSLOW;
	SPI_RW1Byte(W25Q64_POWER_DOWN); 
	W25Q64_CSHIGH;
}

//--------------------------------------------------------------------------------------------------------
//	函 数 名: W25Q64_WakeUp
//	功能说明: wake up flash from power down mode or hign performance mode
//--------------------------------------------------------------------------------------------------------
void W25Q64_WakeUp(void)
{
	W25Q64_CSLOW;
	SPI_RW1Byte(W25Q64_RELEASE_POWER_DOWN);
	W25Q64_CSHIGH;
}

//--------------------------------------------------------------------------------------------------------
//	函 数 名: W25Q64_ReadDeviceID
//	功能说明: 读取FLASH ID(manufacturer ID-1Byte + Device ID-2Byte:type+density)
//	形    参: 无
//	返 回 值: ulJedId：FLASH ID 3字节
//--------------------------------------------------------------------------------------------------------
uint16_t W25Q64_ReadDeviceID(void)
{
	uint16_t usFlashId = 0;
	uint8_t temp_buff[3] = {0};
	W25Q64_CSLOW;
	SPI_RW1Byte(W25Q64_READ_DEVICE_ID);	//90h
	temp_buff[0]=SPI_RW1Byte(0xff);	//写入24位地址；假地址
	temp_buff[1]=SPI_RW1Byte(0xff);
	
	usFlashId = (uint16_t)(temp_buff[0] << 8) | (temp_buff[1] << 0);

	W25Q64_CSHIGH;
	
	return usFlashId;
}
 
//--------------------------------------------------------------------------------------------------------
//	函 数 名: W25Q64_ReadJEDECID
//	功能说明: 读取FLASH ID(manufacturer ID-1Byte + Device ID-2Byte:type+density)
//	形    参: 无
//	返 回 值: ulJedId：FLASH ID 3字节
//--------------------------------------------------------------------------------------------------------
uint32_t W25Q64_ReadJEDECID(void)
{
	uint8_t command = W25Q64_READ_JEDEC_ID;
	uint32_t flash_jed_id = 0;
	uint8_t recv_buff[3] = {0};
	
	W25Q64_CSLOW;
	SPI_RW1Byte(W25Q64_READ_JEDEC_ID);	//9fh
	recv_buff[0]=SPI_RW1Byte(0xff);
	recv_buff[1]=SPI_RW1Byte(0xff);
	recv_buff[2]=SPI_RW1Byte(0xff);
	
	flash_jed_id = (recv_buff[0] << 16) | (recv_buff[1] << 8) | (recv_buff[2] << 0);
	
	W25Q64_CSHIGH;
	return flash_jed_id;
}
