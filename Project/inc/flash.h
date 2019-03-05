/**
 *
 * \file
 *
 * Flash Functions.
 *
 * AT25Q641 External Flashloader for STM32 with QSPI.
 *
 * Author: Jesper Hansen, 2019
 *
 */

#ifndef PROJECT_INC_FLASH_H_
#define PROJECT_INC_FLASH_H_

#include "stm32f7xx_hal.h"
#include "spi_flash_dev.h"

/* QSPI Error codes */
#define QSPI_OK            ((uint8_t)0x00)
#define QSPI_ERROR         ((uint8_t)0x01)
#define QSPI_BUSY          ((uint8_t)0x02)
#define QSPI_NOT_SUPPORTED ((uint8_t)0x04)
#define QSPI_SUSPENDED     ((uint8_t)0x08)


/* Definition for QSPI clock resources */
#define QSPI_CLK_ENABLE()          __HAL_RCC_QSPI_CLK_ENABLE()
#define QSPI_CLK_DISABLE()         __HAL_RCC_QSPI_CLK_DISABLE()
#define QSPI_CS_GPIO_CLK_ENABLE()  __HAL_RCC_GPIOB_CLK_ENABLE()
#define QSPI_CLK_GPIO_CLK_ENABLE() __HAL_RCC_GPIOB_CLK_ENABLE()
#define QSPI_D0_GPIO_CLK_ENABLE()  __HAL_RCC_GPIOD_CLK_ENABLE()
#define QSPI_D1_GPIO_CLK_ENABLE()  __HAL_RCC_GPIOD_CLK_ENABLE()
#define QSPI_D2_GPIO_CLK_ENABLE()  __HAL_RCC_GPIOE_CLK_ENABLE()
#define QSPI_D3_GPIO_CLK_ENABLE()  __HAL_RCC_GPIOD_CLK_ENABLE()

#define QSPI_FORCE_RESET()         __HAL_RCC_QSPI_FORCE_RESET()
#define QSPI_RELEASE_RESET()       __HAL_RCC_QSPI_RELEASE_RESET()

/* Definition for QSPI Pins */
#define QSPI_CLK_PIN             	GPIO_PIN_2	// AF 9
#define QSPI_CLK_GPIO_PORT         	GPIOB

// bank 1
#define QSPI_BK1_CS_PIN				GPIO_PIN_6	// AF 10
#define QSPI_BK1_CS_GPIO_PORT		GPIOB

#define QSPI_BK1_D0_PIN				GPIO_PIN_11	// AF 9
#define QSPI_BK1_D0_GPIO_PORT		GPIOD

#define QSPI_BK1_D1_PIN				GPIO_PIN_12	// AF 9
#define QSPI_BK1_D1_GPIO_PORT		GPIOD

#define QSPI_BK1_D2_PIN				GPIO_PIN_2	// AF 9
#define QSPI_BK1_D2_GPIO_PORT		GPIOE

#define QSPI_BK1_D3_PIN				GPIO_PIN_13	// AF 9
#define QSPI_BK1_D3_GPIO_PORT		GPIOD


// bank 2
#define QSPI_BK2_CS_PIN				GPIO_PIN_11	// AF 9
#define QSPI_BK2_CS_GPIO_PORT		GPIOC

#define QSPI_BK2_D0_PIN				GPIO_PIN_7	// AF 10
#define QSPI_BK2_D0_GPIO_PORT		GPIOE

#define QSPI_BK2_D1_PIN				GPIO_PIN_8	// AF 10
#define QSPI_BK2_D1_GPIO_PORT		GPIOE

#define QSPI_BK2_D2_PIN				GPIO_PIN_9	// AF 10
#define QSPI_BK2_D2_GPIO_PORT		GPIOE

#define QSPI_BK2_D3_PIN				GPIO_PIN_10	// AF 10
#define QSPI_BK2_D3_GPIO_PORT		GPIOE


typedef enum blocksize_e
{
	BLOCKSIZE_4K,
	BLOCKSIZE_32K,
	BLOCKSIZE_64K,
	BLOCKSIZE_ALL
}blocksize_e;

int flash_read( uint32_t address, uint8_t *pdata, uint32_t length);
int flash_write( uint32_t address, uint8_t *pdata, uint32_t length);
int flash_init(void);
int flash_erase(uint32_t address, blocksize_e blocktype);
int flash_chiperase();

#endif /* PROJECT_INC_FLASH_H_ */
