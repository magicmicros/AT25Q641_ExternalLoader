/**
 *
 * \file
 *
 * Debug Serial Channel functions.
 *
 * AT25Q641 External Flashloader for STM32 with QSPI.
 *
 * Author: Jesper Hansen, 2019
 *
 */

#include <loader_main.h>
#include "dbg_serial.h"

/**
 * Send a character over the serial line.
 * \param	[in]	c	Character to send
 */
void dbg_serial_send( char c )
{
	while ((DBG_USART->ISR & USART_ISR_TXE) != USART_ISR_TXE);
	DBG_USART->TDR = (uint8_t) c;
}

/**
 * Initialize Serial Port.
 */
void dbg_serial_init (void)
{
	GPIO_InitTypeDef gpio_def;
	RCC_PeriphCLKInitTypeDef RCC_PeriphClkInit;

	__HAL_RCC_USART1_CLK_ENABLE();
	__HAL_RCC_GPIOA_CLK_ENABLE();

	// Select SysClk
	RCC_PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART1;
	RCC_PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_SYSCLK;
	HAL_RCCEx_PeriphCLKConfig(&RCC_PeriphClkInit);

	// Configure pins
	gpio_def.Pin = GPIO_PIN_9 | GPIO_PIN_10;		// TXD and RXD
	gpio_def.Mode = GPIO_MODE_AF_PP;
	gpio_def.Pull = GPIO_NOPULL;
	gpio_def.Speed = GPIO_SPEED_MEDIUM;
	gpio_def.Alternate = GPIO_AF7_USART1;
	HAL_GPIO_Init(GPIOA, &gpio_def);

	USART1->CR1 = USART_CR1_TE;
	USART1->CR2 = 0;
	USART1->CR3 = 0;

	// BRR Configuration
	USART1->BRR = (uint16_t) ((HAL_RCC_GetSysClockFreq() + (115200 / 2)) / 115200);
	USART1->CR2 &= ~(USART_CR2_LINEN | USART_CR2_CLKEN);
	USART1->CR3 &= ~(USART_CR3_SCEN | USART_CR3_HDSEL | USART_CR3_IREN);

	// Enable USART
	USART1->CR1 |= USART_CR1_UE;
}


//@}
