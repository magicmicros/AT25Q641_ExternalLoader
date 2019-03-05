/**
 *
 * \file
 *
 * Base Interface Functions.
 *
 * AT25Q641 External Flashloader for STM32 with QSPI.
 *
 * Author: Jesper Hansen, 2019
 *
 */

#include <loader_main.h>
#include <string.h>
#include "flash.h"

#include "dbg_serial.h"
#include "printf.h"


/**
  * @brief  System initialization.
  * @param  None
  * @retval  1      : Operation succeeded
  * @retval  0      : Operation failed
  */
int Init (void)
{ 
	/* Enable I-Cache */
	SCB_EnableICache();

	/* Enable D-Cache */
//  SCB_EnableDCache();

	SystemClock_Config();

	// stop system tick IRQ
	SysTick->CTRL &= ~SysTick_CTRL_TICKINT_Msk;

	// enable power to all GPIO
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();
	__HAL_RCC_GPIOC_CLK_ENABLE();
	__HAL_RCC_GPIOD_CLK_ENABLE();
	__HAL_RCC_GPIOE_CLK_ENABLE();

	dbg_serial_init();

	flash_init();

	return 1;
}


KeepInCompilation int Read(uint32_t Address, uint32_t Size, uint8_t* buffer)
{
	Address &= 0x0FFFFFFF;

	return !flash_read(Address, buffer, Size);
}



/**
  * @brief   Program memory.
  * @param   Address: page address
  * @param   Size   : size of data
  * @param   buffer : pointer to data buffer
  * @retval  1      : Operation succeeded
  * @retval  0      : Operation failed
  */
KeepInCompilation int Write (uint32_t Address, uint32_t Size, uint8_t* buffer)
{
	Address &= 0x0FFFFFFF;

	return !flash_write(Address, buffer, Size);
}


/**
  * @brief   Sector erase.
  * @param   EraseStartAddress :  erase start address
  * @param   EraseEndAddress   :  erase end address
  * @retval  None
  */
KeepInCompilation int SectorErase (uint32_t EraseStartAddress, uint32_t EraseEndAddress)
{
  uint32_t BlockAddr;

  EraseStartAddress &= 0x0FFFFFFF;
  EraseEndAddress &= 0x0FFFFFFF;

  EraseStartAddress = EraseStartAddress -  EraseStartAddress % QSPI_SECTOR_SIZE;
	
  while (EraseEndAddress >= EraseStartAddress)
  {
    BlockAddr = EraseStartAddress & 0x0FFFFFFF;
    if (flash_erase(BlockAddr, BLOCKSIZE_4K) == 1)
    	return 0;
    EraseStartAddress += QSPI_SECTOR_SIZE;
  }
  
  return 1;	
}


/**
  * Description :
  * Calculates checksum value of the memory zone
  * Inputs    :
  *      StartAddress  : Flash start address
  *      Size          : Size (in WORD)  
  *      InitVal       : Initial CRC value
  * outputs   :
  *     R0             : Checksum value
  * Note: Optional for all types of device
  */
uint32_t CheckSum(uint32_t StartAddress, uint32_t Size, uint32_t InitVal)
{
  uint8_t missalignementAddress = StartAddress%4;
  uint8_t missalignementSize = Size ;
  int cnt;
  uint32_t Val;
	
  StartAddress-=StartAddress%4;
  Size += (Size%4==0)?0:4-(Size%4);
  
  for(cnt=0; cnt<Size ; cnt+=4)
  {
    Val = *(uint32_t*)StartAddress;
    if(missalignementAddress)
    {
      switch (missalignementAddress)
      {
        case 1:
          InitVal += (uint8_t) (Val>>8 & 0xff);
          InitVal += (uint8_t) (Val>>16 & 0xff);
          InitVal += (uint8_t) (Val>>24 & 0xff);
          missalignementAddress-=1;
          break;
        case 2:
          InitVal += (uint8_t) (Val>>16 & 0xff);
          InitVal += (uint8_t) (Val>>24 & 0xff);
          missalignementAddress-=2;
          break;
        case 3:   
          InitVal += (uint8_t) (Val>>24 & 0xff);
          missalignementAddress-=3;
          break;
      }  
    }
    else if((Size-missalignementSize)%4 && (Size-cnt) <=4)
    {
      switch (Size-missalignementSize)
      {
        case 1:
          InitVal += (uint8_t) Val;
          InitVal += (uint8_t) (Val>>8 & 0xff);
          InitVal += (uint8_t) (Val>>16 & 0xff);
          missalignementSize-=1;
          break;
        case 2:
          InitVal += (uint8_t) Val;
          InitVal += (uint8_t) (Val>>8 & 0xff);
          missalignementSize-=2;
          break;
        case 3:   
          InitVal += (uint8_t) Val;
          missalignementSize-=3;
          break;
      } 
    }
    else
    {
      InitVal += (uint8_t) Val;
      InitVal += (uint8_t) (Val>>8 & 0xff);
      InitVal += (uint8_t) (Val>>16 & 0xff);
      InitVal += (uint8_t) (Val>>24 & 0xff);
    }
    StartAddress+=4;
  }
  
  return (InitVal);
}


/**
  * Description :
  * Verify flash memory with RAM buffer and calculates checksum value of
  * the programmed memory
  * Inputs    :
  *      FlashAddr     : Flash address
  *      RAMBufferAddr : RAM buffer address
  *      Size          : Size (in WORD)  
  *      InitVal       : Initial CRC value
  * outputs   :
  *     R0             : Operation failed (address of failure)
  *     R1             : Checksum value
  * Note: Optional for all types of device
  */
KeepInCompilation uint64_t Verify (uint32_t MemoryAddr, uint32_t RAMBufferAddr, uint32_t Size, uint32_t missalignement)
{
#if 0
  uint32_t VerifiedData = 0, InitVal = 0;
  uint64_t checksum;
  Size*=4;
  QSPI_MemoryMappedTypeDef sMemMappedCfg;
  QSPIHandle.Init.ClockPrescaler     = 0;
  QSPIHandle.Init.FifoThreshold      = 4;
  QSPIHandle.Init.SampleShifting     = QSPI_SAMPLE_SHIFTING_NONE;
  QSPIHandle.Init.FlashSize          = 0x1F;
  QSPIHandle.Init.ChipSelectHighTime = QSPI_CS_HIGH_TIME_1_CYCLE;
  QSPIHandle.Init.ClockMode          = QSPI_CLOCK_MODE_0;  
  
  /* Configure Volatile Configuration register (with new dummy cycles) */
  QSPIHandle.State = HAL_QSPI_STATE_READY;
  
  //QSPI_DummyCyclesCfg(&QSPIHandle);


  sCommand.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
  sCommand.AddressSize       = QSPI_ADDRESS_32_BITS;
  sCommand.AddressMode       = QSPI_ADDRESS_4_LINES;
  sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
  sCommand.AlternateBytesSize= QSPI_ALTERNATE_BYTES_8_BITS;
  sCommand.DataMode          = QSPI_DATA_4_LINES;
  sCommand.DdrMode           = QSPI_DDR_MODE_DISABLE;
  sCommand.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
  sCommand.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;
  sCommand.DummyCycles       = DUMMY_CLOCK_CYCLES_READ_QUAD;
  sCommand.Instruction       = QUAD_INOUT_FAST_READ_CMD;

  sMemMappedCfg.TimeOutActivation = QSPI_TIMEOUT_COUNTER_DISABLE; 
  QSPIHandle.State = HAL_QSPI_STATE_READY;
  HAL_QSPI_MemoryMapped(&QSPIHandle, &sCommand, &sMemMappedCfg);


  checksum = CheckSum((uint32_t)MemoryAddr + (missalignement & 0xf), Size - ((missalignement >> 16) & 0xF), InitVal);
  while (Size>VerifiedData)
  {
    if ( *(uint8_t*)MemoryAddr++ != *((uint8_t*)RAMBufferAddr + VerifiedData))
      return ((checksum<<32) + (MemoryAddr + VerifiedData));  
   
    VerifiedData++;  
  }
        
  return (checksum<<32);
#endif

  return 0;

}


/**
  * @brief  System Clock Configuration
  *         The system Clock is configured as follow :
  *            System Clock source            = PLL (HSE = 25MHz)
  *            SYSCLK(Hz)                     = 216 MHz
  *            HCLK(Hz)                       = 216 MHz
  *            AHB Prescaler                  = 1
  *            APB1 Prescaler                 = 4	-> 54 MHz
  *            APB2 Prescaler                 = 2	-> 108 MHz
  *            HSE Frequency(Hz)              = 25 MHz
  *            PLL_M                          = 25
  *            PLL_N                          = 432
  *            PLL_P                          = 2
  *            PLL_Q                          = 9
  *            PLL_R                          = 7
  *            PLLSAI_N                       = 384
  *            PLLSAI_P                       = 8
  *            VDD(V)                         = 3.3
  *            Main regulator output voltage  = Scale1 mode
  *            Flash Latency(WS)              = 7
  * @param  None
  * @retval None
  */
void SystemClock_Config(void)
{
	  RCC_ClkInitTypeDef RCC_ClkInitStruct;
	  RCC_OscInitTypeDef RCC_OscInitStruct;

	  /* Enable Power Control clock */
	  __HAL_RCC_PWR_CLK_ENABLE();

	  /* The voltage scaling allows optimizing the power consumption when the device is
	     clocked below the maximum system frequency, to update the voltage scaling value
	     regarding system frequency refer to product datasheet.  */
	  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

	  /* Enable HSE Oscillator and activate PLL with HSE as source */
	  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
	  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
	  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
	  RCC_OscInitStruct.HSIState = RCC_HSI_OFF;
	  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
	  RCC_OscInitStruct.PLL.PLLM = 25;
	  RCC_OscInitStruct.PLL.PLLN = 432;
	  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
	  RCC_OscInitStruct.PLL.PLLQ = 9;
	  RCC_OscInitStruct.PLL.PLLR = 7;		// was commented out, enabled to see if it affects USB

	  HAL_RCC_OscConfig(&RCC_OscInitStruct);

	  /* Activate the OverDrive to reach the 216 MHz Frequency */
	  HAL_PWREx_EnableOverDrive();

	  /* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2 clocks dividers */
	  RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
	  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;	// 216 MHz
	  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;		// 54 MHz
	  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;		// 108 MHz

	  HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_7);
}


