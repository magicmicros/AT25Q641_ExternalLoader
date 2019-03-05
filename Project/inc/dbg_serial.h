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

#ifndef DBG_SERIAL_H_
#define DBG_SERIAL_H_

#define DBG_USART	USART1

/**
 * @addtogroup dbg_serial
 */
//@{
//@}

void dbg_serial_init(void);
void dbg_serial_send(char c);

#endif /* DBG_SERIAL_H_ */
