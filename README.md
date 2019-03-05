# External Flash Loader for ST-Link Utility and STM32CubeProgrammer

## Overview

This code implements an external flash loader for an Adesto AT25Q641 Flash memory. 
The AT25Q641 is a 64Mbit SPI serial flash.
The code should work with other sizes with just minor changes to the device config.

## External Loader

An external loader is an "extension" to the programming tool, making it possible to read, write and erase an 
external SPI (or I2C or similar) flash/eeprom memory directly from the programmer.
This way, complete filesystems or other constants can be pre-loaded in a flash for use by the application.


## Hardware dependency

The code is obviously quite dependant on the hardware configuration, pinouts e.t.c.
This implementation was made for a board with an STM32F767VGT6 CPU, using the QSPI lines.
The GPIO assingment could easily be changed to match a different pinout or GPIO mapping.


## This code

The code is based on various other external loaders, but have been stripped of most HAL code as ST's HAL code is terribly  bloated and unpredictable.
Still, a few of the HAL headers are needed for constants, which causes the code to compile a part of the HAL, but the resulting code is not included in the final image.
I might clean this up a bit more in the future.


/Jesper