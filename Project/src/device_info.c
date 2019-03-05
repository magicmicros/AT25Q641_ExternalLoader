/**
 *
 * \file
 *
 * Device Information.
 *
 * AT25Q641 External Flashloader for STM32 with QSPI.
 *
 * Author: Jesper Hansen, 2019
 *
 */

#include <device_info.h>


/* This structure containes information used by ST-LINK Utility to program and erase the device */
#if defined (__ICCARM__)
__root struct StorageInfo const StorageInfo  =  {
#else
__attribute__ ((section(".Dev_info"))) struct StorageInfo const StorageInfo  =  {
#endif
   "AT25QF641_STM32F767VGT6", 	 					        // Device Name + EVAL Board name
   NOR_FLASH,                   					        // Device Type
   0x90000000,                						        // Device Start Address
   0x0800000,                 						        // Device Size in Bytes (8MBytes)
   0x100,                    						        // Programming Page Size (256Bytes)
   0xFF,                       						        // Initial Content of Erased Memory
// Specify Size and Address of Sectors (view example below)
   {{0x0000800, 0x000001000},     				 		   	// Sector Num : 2048 ,Sector Size: 4KBytes
   {0x00000000, 0x00000000}}								// End of list
}; 
