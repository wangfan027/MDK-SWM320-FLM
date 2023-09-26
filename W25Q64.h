

#ifndef __W25Q64_H__
#define __W25Q64_H__

#include <stdint.h>

#define ADDR_W25Q64_START		0x90000000

void W25Q64_Init(void);
uint16_t W25Q64_ReadChipID(void);
uint16_t W25Q64_ReadStatus(void);

uint8_t W25Q64_ReadSR(void);
void W25Q64_WriteSR(uint8_t _ucByte);
void W25Q64_WaitNobusy(void);
void W25Q64_ReadSomeBytes(uint8_t *ucpBuffer, uint32_t _ulReadAddr, uint16_t _usNByte);
void W25Q64_FastReadByte(uint8_t *ucpBuffer, uint32_t _ulReadAddr, uint16_t _usNByte);
void W25Q64_WritePage(uint8_t *ucpBuffer, uint32_t _ulWriteAddr, uint16_t _usNByte);
void W25Q64_WriteNoCheck(uint8_t *ucpBuffer, uint32_t _ulWriteAddr, uint16_t _usNByte);
void W25Q64_WriteSomeBytes(uint8_t *ucpBuffer, uint32_t _ulWriteAddr, uint16_t _usNByte);
void W25Q64_ErasePage(uint32_t _ulPageAddr);
void W25Q64_EraseSector4K(uint32_t _ulSectorAddr);
void W25Q64_EraseBlock(uint32_t _ulBlockAddr);
void W25Q64_EraseChip(void);
void W25Q64_PowerDown(void);
void W25Q64_WakeUp(void);
uint16_t W25Q64_ReadDeviceID(void);
uint32_t W25Q64_ReadJEDECID(void);


#define W25Q64_STATUS_WIP_Pos		0		// Write In Progress
#define W25Q64_STATUS_WIP_Msk		(0x01 << W25Q64_STATUS_WIP_Pos)

#define W25Q64_STATUS_WEL_Pos		1		// Write Enable Latch
#define W25Q64_STATUS_WEL_Msk		(0x01 << W25Q64_STATUS_WEL_Pos)

#define W25Q64_STATUS_SBP_Pos		2		// Software Block Protect
#define W25Q64_STATUS_SBP_Msk		(0x1F << W25Q64_STATUS_SBP_Pos)

#define W25Q64_STATUS_SRP_Pos		7		// Status Register Protect
#define W25Q64_STATUS_SRP_Msk		(0x03 << W25Q64_STATUS_SRP_Pos)

#define W25Q64_STATUS_SUS_Pos		15		// Erase/Program Suspend
#define W25Q64_STATUS_SUS_Msk		(0x01 << W25Q64_STATUS_SUS_Pos)


#define  W25Q64_WRITE_ENABLE_CMD 		0x06
#define  W25Q64_WRITE_DISABLE_CMD		0x04
#define  W25Q64_READ_SR_CMD					0x05
#define  W25Q64_WRITE_SR_CMD				0x01
#define  W25Q64_READ_DATA						0x03
#define  W25Q64_FASTREAD_DATA				0x0b
#define  W25Q64_WRITE_PAGE					0x02
#define  W25Q64_ERASE_PAGE      		0x81
#define  W25Q64_ERASE_SECTOR4K      0x20
#define	 W25Q64_ERASE_BLOCK					0xd8
#define	 W25Q64_ERASE_CHIP					0xc7
#define  W25Q64_POWER_DOWN					0xb9
#define  W25Q64_RELEASE_POWER_DOWN  0xab
#define  W25Q64_READ_DEVICE_ID      0x90
#define  W25Q64_READ_JEDEC_ID      	0x9f
#define  W25Q64_ID									0x164017

#define 	FLASH_SIZE	 	0x00800000	// 8M字节
#define		PAGE_SIZE			256	// 256 bytes
#define 	SECTOR_SIZE		512	 // 4-Kbyte
#define		BLOCK_SIZE		32	// 64-Kbyte	

#endif

