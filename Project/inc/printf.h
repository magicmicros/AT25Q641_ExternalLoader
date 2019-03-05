/**
 *
 * \file
 *
 * Small printf replacement functions.
 *
 * AT25Q641 External Flashloader for STM32 with QSPI.
 *
 * Author: Jesper Hansen, 2019
 *
 */

#include <stdarg.h>
#include <inttypes.h>

extern int _p_printf(char const *fmt0, ...);
extern int _p_sprintf(char *buf, char const *format, ...);

#define printf(format, args...)    _p_printf(format, ## args)					//!< macro to map printf to our _p_printf function
#define sprintf(buffer, format, args...)   _p_sprintf(buffer, format, ## args)	//!< macro to map sprintf to our _p_sprintf function

