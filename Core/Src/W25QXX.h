#ifndef __W25QCSFLASHCS
#define __W25QCSFLASHCS

// 对于W25Q64,SECTOR的大小是4KB

// SPI时序要求，时钟平时是低，上升沿捕获，高位在前，8字节数据长度,
// 1注意：如果出现数据异常，尝试芯片断电一会在上电
#include <stdint.h>

typedef enum DEV_TYPE {W25Q,MX25}DEV_TYPE;//支持两种芯片
// enum DEV_STA1_ {W25QCS_BUSY,W25QCS_IDLE,W25QCS_ERROR,W25QCS_READY,};
typedef struct W25QCS_BASE{
	// enum DEV_STA1_ dev_sta;

	uint16_t W25QCS_SIZE_MB;
	void (* W25QCS_CS_L_enable)(void);
	void (* W25QCS_CS_H)(void);
	// spi时序要求，时钟平时是低，上升沿捕获，高位在前，8字节数据长度,
	uint8_t (*W25QCS_SPI_RW)(uint8_t a_u8);
	uint8_t (*DELAY_MS)(uint32_t a_u8);
	DEV_TYPE dev_type;

	uint16_t good_trans_count_u16;
	uint16_t bad_trans_count_u16;
	uint16_t good_recv_count_u16;     
}W25QCS_BASE;


void W25Q_Init(W25QCS_BASE * base,
					void (* W25QCS_CS_L_enable)(void),
					void (* W25QCS_CS_H)(void),
					// spi时序要求，时钟平时是低，上升沿捕获，高位在前，8字节数据长度,
					uint8_t (*W25QCS_SPI_RW)(uint8_t a_u8),
					uint8_t (*DELAY_MS)(uint32_t a_u8),
					uint32_t size_MB,//可填写 W25Q,MX25
					 DEV_TYPE dev_type
					);
// 1:测试不通过，器件可能有问题
// 0：测试通过
uint8_t W25Q_Test(W25QCS_BASE * base); //用于测试 SPI FLASH 
void W25QCSXX_Read(W25QCS_BASE * base,uint32_t ReadAddr,uint8_t* pBuffer,uint16_t len); //这可以读出整个flash的内容
void W25QCS_Write(W25QCS_BASE * base,uint32_t addr,uint8_t* ptr,uint16_t len);//任意位置都可以写入

// 1：擦除成功
// -1：擦除失败
// 擦除指定地址所在的sector的数据
int8_t  W25QCSXX_Erase_Sector(W25QCS_BASE * base,uint32_t Dst_Addr); 

// 1：擦除成功
// -1：擦除失败
// 由 Sector的序号来擦除flash
int8_t W25QCSXXEraseSectorByIndex(W25QCS_BASE * base,uint16_t SectorIndex);

void W25QCSXX_Erase_Chip(W25QCS_BASE * base);  

// 低功耗下使用
void W25QCSXX_PowerDown(W25QCS_BASE * base);  
void W25QCSXX_WAKEUP(W25QCS_BASE * base);  
void W25QCSXX_Write_NoCheck(W25QCS_BASE * base,uint32_t WriteAddr,uint8_t* pBuffer,uint16_t NumByteToWrite);
void W25QCSXX_ReadUniquidID(W25QCS_BASE * base,uint8_t *ptr,uint8_t len);
#define W25Q_GetSectorNum(base) ((base)->W25QCS_SIZE_MB*1024/4)//得到flash有多少区块

#endif

