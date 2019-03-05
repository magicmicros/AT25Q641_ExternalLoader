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

#include "printf.h"
#include "flash.h"

/**
 * @brief  Delays for amount of micro seconds
 * @param  micros: Number of microseconds for delay
 * @retval None
 */
void usleep(__IO uint32_t micros)
{
	/*
	 * The DWT doesn't seem to work properly in this setup
	 *
    uint32_t start = DWT->CYCCNT;
    // Go to number of cycles for system
    micros *= (HAL_RCC_GetHCLKFreq() / 1000000);
    // Delay till end
    while ((DWT->CYCCNT - start) < micros);
    */
	while (micros--)
		for (volatile int i=0;i<20;i++)
		{
		}
}


//
// returns 0 or 1 according to status register flag state
//
static int inline get_sr_flag_state(uint32_t flag)
{
	return (QUADSPI->SR & flag) != 0;
}

static int inline wait_flag(uint32_t flag, int state, uint32_t ms)
{
	ms *= 100;

	// Wait until flag is in expected state
	while ( get_sr_flag_state(flag) != state)
	{
		usleep(10);		// wait 100us
		ms--;
		if (ms == 0)	// if timeout
			return 1;	// error
	}
	return 0;			// ok
}


static int send_single_command(uint8_t cmd)
{
	// wait for not busy
	if (wait_flag(QSPI_FLAG_BUSY, RESET, 1000))
	{
		return 1;
	}

    QUADSPI->CCR  = QSPI_INSTRUCTION_1_LINE | cmd | 0;

	// When there is no data phase, the transfer start as soon as the configuration is done
	// so wait until TC flag is set to go back in idle state
	if (wait_flag(QSPI_FLAG_TC, SET, 1000) == 0)
	{
		QUADSPI->FCR = QSPI_FLAG_TC;	// clear flag
		return 0;
	}

	return 1;
}



static int read_status_register(void)
{
    uint32_t addr_reg = QUADSPI->AR;
    __IO uint32_t *data_reg = &QUADSPI->DR;
    uint32_t length = 1;
    uint8_t buffer[4];
    uint8_t *pdata = buffer;

    QUADSPI->DLR = 0;
    QUADSPI->CCR  = QSPI_INSTRUCTION_1_LINE | QSPI_DATA_1_LINE | READ_STATUS_REG_CMD | QUADSPI_CCR_FMODE_0;

	// Start the transfer by re-writing the address in AR register
	QUADSPI->AR = addr_reg;

	while (length)
	{
		if (wait_flag((QSPI_FLAG_FT | QSPI_FLAG_TC), SET, 1000) != 0)
		{
			return -1;
		}
		*pdata++ = *(__IO uint8_t *)data_reg;
		length--;
	}

	if (wait_flag(QSPI_FLAG_TC, SET, 1000) == 0)
		QUADSPI->FCR = QSPI_FLAG_TC;	// clear flag
	else
	{
		return -1;
	}

	return buffer[0];
}

static int wait_busy_clear(uint32_t timeout)
{
	timeout *= 100;
	int n;
	while (1)
	{
		if ((n = read_status_register()) == -1)
			return 1;
		if ((n & FLASH_DEV_SR_BUSY) == 0)
			break;
		usleep(10);
		if (--timeout == 0)
			return 1;
	}
	return 0;
}


/**
  * @brief  This function send a Write Enable and wait it is effective.
  * @param  hqspi: QSPI handle
  * @retval None
  */
static uint8_t write_enable(void /*QSPI_HandleTypeDef *hqspi*/)
{
	int n;
	uint32_t timeout = 100000;

	// send command
	if (send_single_command(WRITE_ENABLE_CMD) != 0)
		return 1;

	// now wait for the WEL bit to be set
	while ((n = read_status_register()) != -1)
	{
		if (n & FLASH_DEV_SR_WEL)
			return 0;
		usleep(10);
		if (--timeout == 0)
		{
			return 1;
		}
	}

	return 1;
}




/**
  * @brief  This function reset the QSPI memory.
  * @param  hqspi: QSPI handle
  * @retval None
  */
static uint8_t reset_memory(void)
{
	// send command
	if (send_single_command(RESET_ENABLE_CMD) != 0)
	{
		return 1;
	}

	// send command
	if (send_single_command(RESET_MEMORY_CMD) != 0)
	{
		return 2;
	}

	if (wait_busy_clear(1000))
	{
		return 3;
	}

	return 0;
}


/********************************************************************************************/
/********************************************************************************************/
/********************************************************************************************/

static void flash_deinit(void)
{
  /* De-Configure QSPI pins */
	HAL_GPIO_DeInit(QSPI_CLK_GPIO_PORT, QSPI_CLK_PIN);

	HAL_GPIO_DeInit(QSPI_BK1_CS_GPIO_PORT, QSPI_BK1_CS_PIN);
	HAL_GPIO_DeInit(QSPI_BK1_D0_GPIO_PORT, QSPI_BK1_D0_PIN);
	HAL_GPIO_DeInit(QSPI_BK1_D1_GPIO_PORT, QSPI_BK1_D1_PIN);
	HAL_GPIO_DeInit(QSPI_BK1_D2_GPIO_PORT, QSPI_BK1_D2_PIN);
	HAL_GPIO_DeInit(QSPI_BK1_D3_GPIO_PORT, QSPI_BK1_D3_PIN);

	HAL_GPIO_DeInit(QSPI_BK2_CS_GPIO_PORT, QSPI_BK2_CS_PIN);
	HAL_GPIO_DeInit(QSPI_BK2_D0_GPIO_PORT, QSPI_BK2_D0_PIN);
	HAL_GPIO_DeInit(QSPI_BK2_D1_GPIO_PORT, QSPI_BK2_D1_PIN);
	HAL_GPIO_DeInit(QSPI_BK2_D2_GPIO_PORT, QSPI_BK2_D2_PIN);
	HAL_GPIO_DeInit(QSPI_BK2_D3_GPIO_PORT, QSPI_BK2_D3_PIN);

	/* Reset the QuadSPI memory interface */
	QSPI_FORCE_RESET();
	QSPI_RELEASE_RESET();

	/* Disable the QuadSPI memory interface clock */
	QSPI_CLK_DISABLE();
}



int flash_init(void)
{
	GPIO_InitTypeDef gpio_init_structure;

	flash_deinit();

	/* Enable the QuadSPI memory interface clock */
	QSPI_CLK_ENABLE();

	/* Enable GPIO clocks */
	QSPI_CS_GPIO_CLK_ENABLE();
	QSPI_CLK_GPIO_CLK_ENABLE();
	QSPI_D0_GPIO_CLK_ENABLE();
	QSPI_D1_GPIO_CLK_ENABLE();
	QSPI_D2_GPIO_CLK_ENABLE();
	QSPI_D3_GPIO_CLK_ENABLE();

	/* QSPI CLK GPIO pin configuration  */
	gpio_init_structure.Pin = QSPI_CLK_PIN;
	gpio_init_structure.Mode = GPIO_MODE_AF_PP;
	gpio_init_structure.Pull = GPIO_NOPULL;
	gpio_init_structure.Speed = GPIO_SPEED_HIGH;
	gpio_init_structure.Alternate = GPIO_AF9_QUADSPI;
	HAL_GPIO_Init(QSPI_CLK_GPIO_PORT, &gpio_init_structure);

	// Bank 1
#if 1
	/* QSPI CS GPIO pin configuration  */
	gpio_init_structure.Pin = QSPI_BK1_CS_PIN;
	gpio_init_structure.Alternate = GPIO_AF10_QUADSPI;
	HAL_GPIO_Init(QSPI_BK1_CS_GPIO_PORT, &gpio_init_structure);

	/* QSPI D0 GPIO pin configuration  */
	gpio_init_structure.Pin = QSPI_BK1_D0_PIN;
	gpio_init_structure.Alternate = GPIO_AF9_QUADSPI;
	HAL_GPIO_Init(QSPI_BK1_D0_GPIO_PORT, &gpio_init_structure);

	/* QSPI D1 GPIO pin configuration  */
	gpio_init_structure.Pin = QSPI_BK1_D1_PIN;
	gpio_init_structure.Alternate = GPIO_AF9_QUADSPI;
	HAL_GPIO_Init(QSPI_BK1_D1_GPIO_PORT, &gpio_init_structure);

	/* QSPI D2 GPIO pin configuration  */
	gpio_init_structure.Pin = QSPI_BK1_D2_PIN;
	gpio_init_structure.Alternate = GPIO_AF9_QUADSPI;
	HAL_GPIO_Init(QSPI_BK1_D2_GPIO_PORT, &gpio_init_structure);

	/* QSPI D3 GPIO pin configuration  */
	gpio_init_structure.Pin = QSPI_BK1_D3_PIN;
	gpio_init_structure.Alternate = GPIO_AF9_QUADSPI;
	HAL_GPIO_Init(QSPI_BK1_D3_GPIO_PORT, &gpio_init_structure);
#endif

	// Bank 2

#if 1
	/* QSPI CS GPIO pin configuration  */
	gpio_init_structure.Pin = QSPI_BK2_CS_PIN;
	gpio_init_structure.Alternate = GPIO_AF9_QUADSPI;
	HAL_GPIO_Init(QSPI_BK2_CS_GPIO_PORT, &gpio_init_structure);

	/* QSPI D0 GPIO pin configuration  */
	gpio_init_structure.Pin = QSPI_BK2_D0_PIN;
	gpio_init_structure.Alternate = GPIO_AF10_QUADSPI;
	HAL_GPIO_Init(QSPI_BK2_D0_GPIO_PORT, &gpio_init_structure);

	/* QSPI D1 GPIO pin configuration  */
	gpio_init_structure.Pin = QSPI_BK2_D1_PIN;
	gpio_init_structure.Alternate = GPIO_AF10_QUADSPI;
	HAL_GPIO_Init(QSPI_BK2_D1_GPIO_PORT, &gpio_init_structure);

	/* QSPI D2 GPIO pin configuration  */
	gpio_init_structure.Pin = QSPI_BK2_D2_PIN;
	gpio_init_structure.Alternate = GPIO_AF10_QUADSPI;
	HAL_GPIO_Init(QSPI_BK2_D2_GPIO_PORT, &gpio_init_structure);

	/* QSPI D3 GPIO pin configuration  */
	gpio_init_structure.Pin = QSPI_BK2_D3_PIN;
	gpio_init_structure.Alternate = GPIO_AF10_QUADSPI;
	HAL_GPIO_Init(QSPI_BK2_D3_GPIO_PORT, &gpio_init_structure);
#endif

	/* Configure QSPI FIFO Threshold */
	QUADSPI->CR &= QUADSPI_CR_FTHRES;
	QUADSPI->CR |= (3 << 8);			// set FIFO Threshold to 4

	// wait for not busy
	if (wait_flag(QSPI_FLAG_BUSY, RESET, 1000))
		return 1;

	/* Configure QSPI Clock Prescaler and Sample Shift */
	QUADSPI->CR &= ~(QUADSPI_CR_PRESCALER | QUADSPI_CR_SSHIFT | QUADSPI_CR_FSEL
			| QUADSPI_CR_DFM);
	QUADSPI->CR |= ((2 << 24) |	// set clock prescaler to 2 => 72MHz
			QSPI_SAMPLE_SHIFTING_HALFCYCLE |
			QSPI_FLASH_ID_1 |
			QSPI_DUALFLASH_DISABLE);

	/* Configure QSPI Flash Size, CS High Time and Clock Mode */
	QUADSPI->DCR &=
			~(QUADSPI_DCR_FSIZE | QUADSPI_DCR_CSHT | QUADSPI_DCR_CKMODE);
	QUADSPI->DCR |= (((POSITION_VAL(FLASH_DEV_FLASH_SIZE) - 1) << 16) |
	QSPI_CS_HIGH_TIME_6_CYCLE |
	QSPI_CLOCK_MODE_0);

	/* Enable the QSPI peripheral */
	QUADSPI->CR |= QUADSPI_CR_EN;

	reset_memory();

	return QSPI_OK;
}



int flash_chiperase(void)
{
	if (write_enable() != 0)
		return 1;

	if (send_single_command(BULK_ERASE_CMD) != 0)
		return 2;

	if (wait_busy_clear(FLASH_DEV_BULK_ERASE_MAX_TIME))
		return 3;

	return 0;
}



int flash_read( uint32_t ReadAddr, uint8_t *pData, uint32_t Size)
{
	if (wait_flag(QSPI_FLAG_BUSY, RESET, 1000) != 0)
		return 1;

    /* Configure QSPI: DLR register with the number of data to read or write */
    QUADSPI->DLR = (Size - 1);

    /* Configure QSPI: ABR register with alternate bytes value */
    QUADSPI->ABR = 0;

    /*---- Command with instruction, address and alternate bytes ----*/
    /* Configure QSPI: CCR register with all communications parameters */
    QUADSPI->CCR = (QSPI_DATA_4_LINES | (FLASH_DEV_DUMMY_CYCLES_READ_QUAD << 18) | QSPI_ALTERNATE_BYTES_8_BITS |
    				QSPI_ALTERNATE_BYTES_4_LINES | QSPI_ADDRESS_24_BITS | QSPI_ADDRESS_4_LINES |
					QSPI_INSTRUCTION_1_LINE | QUAD_INOUT_FAST_READ_CMD | QUADSPI_CCR_FMODE_0);

    /* Configure QSPI: AR register with address value */
    QUADSPI->AR = ReadAddr;


    volatile uint32_t addr_reg = QUADSPI->AR;
    __IO uint32_t *data_reg = &QUADSPI->DR;

	// Start the transfer by re-writing the address in AR register
	QUADSPI->AR = addr_reg;

	while (Size)
	{
		if (wait_flag((QSPI_FLAG_FT | QSPI_FLAG_TC), SET, 1000) != 0)
		{
			return 1;
		}
		*pData++ = *(__IO uint8_t *) data_reg;
		Size--;
	}

	if (wait_flag(QSPI_FLAG_TC, SET, 1000) == 0)
		QUADSPI->FCR = QSPI_FLAG_TC;	// clear flag
	else
	{
		return 2;
	}

	return 0;
}



int flash_write( uint32_t WriteAddr, uint8_t *pData, uint32_t Size)
{
	uint32_t end_addr, current_size, current_addr;
    __IO uint32_t *data_reg = &QUADSPI->DR;

	/* Calculation of the size between the write address and the end of the page */
	current_size = FLASH_DEV_PAGE_SIZE - (WriteAddr % FLASH_DEV_PAGE_SIZE);

	/* Check if the size of the data is less than the remaining place in the page */
	if (current_size > Size)
	{
		current_size = Size;
	}

	/* Initialize the adress variables */
	current_addr = WriteAddr;
	end_addr = WriteAddr + Size;

	if (wait_flag(QSPI_FLAG_BUSY, RESET, 1000) != 0)
		return 1;


//	usleep(500);

	/* Perform the write page by page */
	do
	{
		/* Enable write operations */
		if (write_enable() != 0)
		{
			return 1;
		}

	    /* Configure QSPI: DLR register with the number of data to read or write */
	    QUADSPI->DLR =  (current_size - 1);

	    /* Configure QSPI: ABR register with alternate bytes value */
	    QUADSPI->ABR = 0;

	    /*---- Command with instruction, address and alternate bytes ----*/
	    /* Configure QSPI: CCR register with all communications parameters */
	    QUADSPI->CCR = (QSPI_DATA_4_LINES | QSPI_ADDRESS_24_BITS | QSPI_ADDRESS_4_LINES |
						QSPI_INSTRUCTION_1_LINE | QUAD_PAGE_PROG_CMD);

	    /* Configure QSPI: AR register with address value */
	    QUADSPI->AR = current_addr;

		current_addr += current_size;

		while (current_size > 0)
		{
		  while ((QUADSPI->SR & QSPI_FLAG_FT) == 0);	// wait

		  *(__IO uint8_t *)data_reg = *pData++;			// send byte
		  current_size--;								// decrement size
		}

		if (wait_flag(QSPI_FLAG_TC, SET, 1000) == 0)
			QUADSPI->FCR = QSPI_FLAG_TC;	// clear flag
		else
		{
			return 2;
		}

		if (wait_busy_clear(1000))
		{
			return 3;
		}

		current_size = ((current_addr + FLASH_DEV_PAGE_SIZE) > end_addr) ? (end_addr - current_addr) : FLASH_DEV_PAGE_SIZE;
	} while (current_addr < end_addr);

	return 0;
}



int flash_erase(uint32_t address, blocksize_e blocktype)
{
	uint8_t cmd;
	uint32_t erase_time = FLASH_DEV_SUBSECTOR_ERASE_MAX_TIME;

	// Enable write operations
	if (write_enable() != 0)
	{
		return 1;
	}

	switch (blocktype)
	{
	default:
	case BLOCKSIZE_4K:
		cmd	= BLOCK_ERASE_4_CMD;
		erase_time	= FLASH_DEV_SUBSECTOR_ERASE_MAX_TIME;
		break;

	case BLOCKSIZE_32K:
		cmd   		= BLOCK_ERASE_32_CMD;
		erase_time	= FLASH_DEV_SECTOR_ERASE_MAX_TIME;
		break;

	case BLOCKSIZE_64K:
		cmd   		= BLOCK_ERASE_64_CMD;
		erase_time	= FLASH_DEV_SECTOR_ERASE_MAX_TIME;
		break;

	case BLOCKSIZE_ALL:
		return flash_chiperase();
		break;
	}

	if (wait_flag(QSPI_FLAG_BUSY, RESET, 1000) != 0)
	{
		return 2;
	}


    /*---- Command with instruction and address ----*/
    /* Configure QSPI: CCR register with all communications parameters */
    QUADSPI->CCR = QSPI_ADDRESS_24_BITS | QSPI_ADDRESS_1_LINE | QSPI_INSTRUCTION_1_LINE | cmd;

    /* Configure QSPI: AR register with address value */
    QUADSPI->AR = address;

    usleep(10);


	if (wait_busy_clear(erase_time))
	{
		return 3;
	}

	if (wait_flag(QSPI_FLAG_BUSY, RESET, 1000) != 0)
	{
		return 4;
	}
	QUADSPI->FCR = 0x07;	// clear flags

	return 0;
}



