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
#include <ctype.h>
#include <string.h>
#include "printf.h"

extern void dbg_serial_send(char);			//!< extern ptr to "our" output function
#define myputchar(x) dbg_serial_send(x)		//!< map the output function in the code below to "our" output


/** Variable printf.
 * Replacement for the regular vprintf, but with some features and quirks.
 * Floats and Double are not supported.
 * @param buf		Pointer to a buffer for sprintf, NULL for printf
 * @param format	Format string in RAM or PROGMEM
 * @param ap		Variable parameter list
 * @return			Always 0
 */
int _p_vprintf(char *buf, char const *format, va_list ap)
{
  uint8_t 	scratch[16];
  uint8_t 	format_flag;
  uint32_t 	u_val=0;
  uint8_t	base;
  uint8_t	*ptr;
  uint8_t	width;
  uint8_t 	fill;
  uint8_t 	neg;
  uint8_t 	long_modifier;

  while(1)
  {
  
	long_modifier = 0;
  	width = 0;
  	neg = 0;
  	fill = ' ';
    while ((format_flag = *format++) != '%')
	{  
      if (!format_flag)
	  {
		  va_end (ap); 
		  return (0);
	  }
      if (buf)
      {
      	*buf = format_flag; buf++; *buf=0;
      }
      else
      	myputchar(format_flag);
    }


	// check for zero pad
	format_flag = *format - '0';
	if (format_flag == 0)	// zero pad
	{
		fill = '0';
		format++;
	}
	
	// check for width spec
	while (1)
	{
		format_flag = *format - '0';
		if ((format_flag <= 9))
		{
			width *= 10;
			width += format_flag;
			format++;
		}
		else
			break;
	}
	
	// check 'l' modifier
	format_flag = *format;
	if (format_flag == 'l')		// long modifier
	{
		long_modifier = 1;
		format++;	
	}
	
    switch (format_flag = *format++)
	{
		case 'c':
		  format_flag = va_arg(ap,int);

		  // no break
		  
		default:
		  if (buf)
		  {
		    *buf = format_flag; 
		    buf++;
		    *buf=0;
		  }
		  else 
	        myputchar(format_flag);
		  continue;
		  
		case 'S':
		case 's':
			ptr = (uint8_t*) va_arg(ap, char *);
			if (buf) {
				while (--width && *ptr)
					*buf++ = *ptr++;

				*buf = 0;
			} else {
				while (--width && *ptr)
					myputchar(*ptr++);
			}
			continue;

		case 'o':
			base = 8;
			if (buf) {
				*buf = '0';
				buf++;
				*buf = 0;
			} else
				myputchar('0');
			goto CONVERSION_LOOP;

		case 'd':
			if ((int) u_val < 0)
			{
				u_val = -u_val;
				neg = 1;
			}

			// no break

		case 'u':
			base = 10;
			goto CONVERSION_LOOP;
		  
		case 'x':
			base = 16;

CONVERSION_LOOP:
		if (long_modifier)
			u_val = va_arg(ap,long);
		else
			u_val = va_arg(ap,int);

		ptr = scratch + 16;
		*--ptr = 0;
		do
		{
			char ch = u_val % base + '0';
			if (ch > '9')
			  ch += 'a' - '9' - 1;
			*--ptr = ch;
			u_val /= base;

			if (width)
				if(--width == 0) break;
		
		} while (u_val);

		if (neg)
			*--ptr = '-';

		while (width--)
			*--ptr = fill;


		if (buf)
		{
			while (*ptr)
				*buf++ = *ptr++;
			*buf = 0;
		}
		else
			while (*ptr)
				myputchar(*ptr++);
	}
  }
  return 0;
}

/** Printf replacement.
 * Small size replacement for printf.
 * Floats and Double are not supported.
 * @param format	Format string in RAM or PROGMEM
 * @param ...		Variable parameter list
 * @return			Always 0
 */
int _p_printf(char const *format, ...)
{
  	va_list ap;
  	va_start (ap, format);
  	return _p_vprintf(NULL,format,ap);
}

/** Sprintf replacement.
 * Small size replacement for sprintf.
 * Floats and Double are not supported.
 * @param buf		Pointer to a buffer for sprintf, NULL for printf
 * @param format	Format string in RAM or PROGMEM
 * @param ...		Variable parameter list
 * @return			Always 0
 */
int _p_sprintf(char *buf, char const *format, ...)
{
  	va_list ap;
  	va_start (ap, format);
  	return _p_vprintf(buf,format,ap);
}
