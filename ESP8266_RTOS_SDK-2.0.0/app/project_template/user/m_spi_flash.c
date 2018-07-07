/*
 * ESPRSSIF MIT License
 *
 * Copyright (c) 2015 <ESPRESSIF SYSTEMS (SHANGHAI) PTE LTD>
 *
 *
 *
 */

#include "user_config.h"


//spi flash read function
/**
  * @brief  Read data from Flash.
  *
  * @param  uint32 src_addr  : source address of the flash.
  * @param  uint32 *buf		 : destination address in data.
  * @param  uint32 size      : length of data
  *
  * @return SpiFlashOpResult 
  *		   SPI_FLASH_RESULT_OK,        < SPI Flash operating OK 
  *		    SPI_FLASH_RESULT_ERR,        SPI Flash operating fail 
  *		    SPI_FLASH_RESULT_TIMEOUT     SPI Flash operating time out 
  */
SpiFlashOpResult mSpi_flash_read(uint32 src_addr, uint32 *buf, uint32 size){

	printf("---lx0x3E sec:%02x%02x%02x%02x\r\n", buf[0], buf[1], buf[2], buf[3]);	
	return spi_flash_read(src_addr, buf, sizeof(uint32)*size);
	
}
//spi flash write function
SpiFlashOpResult mSpi_flash_write(uint32 des_addr, uint32 *data, uint32 size){
	printf("---lx write the sector =0x%x",des_addr/SPI_FLASH_SEC_SIZE);
	// TODO: fit in the data
	spi_flash_erase_sector(des_addr/SPI_FLASH_SEC_SIZE);//erase flash sector(N*4K/4096) 
	return spi_flash_write(des_addr, data, sizeof(uint32)*size);
}


