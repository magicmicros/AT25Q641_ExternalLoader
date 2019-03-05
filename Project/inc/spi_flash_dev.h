/**
 *
 * \file
 *
 * AT25Q641 constants and commands.
 *
 * AT25Q641 External Flashloader for STM32 with QSPI.
 *
 * Author: Jesper Hansen, 2019
 *
 */

#ifndef __SPI_FLASH_DEV_H
#define __SPI_FLASH_DEV_H


#define FLASH_DEV_FLASH_SIZE        		0x800000 	// 64 MBits => 8MBytes
#define FLASH_DEV_SECTOR_SIZE               0x10000   	// 128 sectors of 64KBytes
#define FLASH_DEV_SUBSECTOR_SIZE            0x1000    	// 4096 subsectors of 4kBytes
#define FLASH_DEV_PAGE_SIZE                 0x100     	// 65536 pages of 256 bytes

#define FLASH_DEV_DUMMY_CYCLES_READ_FAST    8
#define FLASH_DEV_DUMMY_CYCLES_READ_QUAD    4

#define FLASH_DEV_BULK_ERASE_MAX_TIME      	250000
#define FLASH_DEV_SECTOR_ERASE_MAX_TIME    	3000
#define FLASH_DEV_SUBSECTOR_ERASE_MAX_TIME  800



/* Reset Operations */
#define RESET_ENABLE_CMD                    0x66
#define RESET_MEMORY_CMD 		            0x99

/* Identification Operations */
#define READ_MANUF_ID                       0x90
#define READ_JEDEC_ID                       0x9F
#define READ_SERIAL_FLASH_DISCO_PARAM_CMD   0x5A
#define READ_UNIQUE_ID                      0x4B

/* Read Operations */
#define READ_CMD                            0x03
#define FAST_READ_CMD                       0x0B
#define QUAD_OUT_FAST_READ_CMD              0x6B
#define QUAD_INOUT_FAST_READ_CMD            0xEB

/* Write Operations */
#define WRITE_ENABLE_CMD                    0x06
#define WRITE_DISABLE_CMD                   0x04

/* Register Operations */
#define READ_STATUS_REG_CMD                 0x05
#define READ_STATUS_REG2_CMD                0x35
#define WRITE_STATUS_REG_CMD                0x01
#define WRITE_STATUS_REG2_CMD          		0x31

/* Program Operations */
#define PAGE_PROG_CMD						0x02
#define QUAD_PAGE_PROG_CMD					0x33		// NOTE: some may have this as 0x32

/* Erase Operations */
#define BLOCK_ERASE_4_CMD              	 	0x20		// block erase 4kB
#define BLOCK_ERASE_32_CMD                  0x52		// block erase 32kB
#define BLOCK_ERASE_64_CMD                  0xD8		// block erase 64kB
#define BULK_ERASE_CMD                      0xC7		// full chip erase

#define PROG_ERASE_SUSPEND_CMD             	0x75
#define PROG_ERASE_RESUME_CMD          		0x7A


/** 
  * @brief  Registers
  */ 
/* Status Register */
#define FLASH_DEV_SR_BUSY    				((uint8_t)0x01)    /*!< Write in progress */
#define FLASH_DEV_SR_WEL                    ((uint8_t)0x02)    /*!< Write enable latch */
#define FLASH_DEV_SR_BL0                    ((uint8_t)0x04)    /*!< Block Protect 0 */
#define FLASH_DEV_SR_BL1                    ((uint8_t)0x08)    /*!< Block Protect 1 */
#define FLASH_DEV_SR_BL2                    ((uint8_t)0x10)    /*!< Block Protect 2 */
#define FLASH_DEV_SR_TB		                ((uint8_t)0x20)    /*!< Protected memory area defined by BLOCKPR starts from top or bottom */
#define FLASH_DEV_SR_SEC		            ((uint8_t)0x40)    /*!< Sector Protect */
#define FLASH_DEV_SR_SRP0					((uint8_t)0x80)    /*!< Status register write enable/disable */

/* Status Register 2 */
#define FLASH_DEV_SR2_SRP1                	((uint8_t)0x01)    /*!< Status register 2 write enable/disable */
#define FLASH_DEV_SR2_QE            		((uint8_t)0x02)    /*!< Quad Enable */
#define FLASH_DEV_SR2_CMP	    	       	((uint8_t)0x40)    /*!< Complement Protect */
#define FLASH_DEV_SR2_SUS               	((uint8_t)0x80)    /*!< Suspend Status */



#endif /* __SPI_FLASH_DEV_H */
