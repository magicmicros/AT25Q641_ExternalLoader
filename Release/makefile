################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include Project/src/subdir.mk
-include Libraries/STM32F7xx_HAL_Driver/Src/Legacy/subdir.mk
-include Libraries/STM32F7xx_HAL_Driver/Src/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: AT25Q641_ExternalLoader.elf

# Tool invocations
AT25Q641_ExternalLoader.elf: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: Cross GCC Linker'
	arm-none-eabi-gcc -T "/Users/jesper/Documents/workspace/AT25Q641_ExternalLoader/Project/stm32f767vgt6_ram.ld"  --specs=nano.specs  -mcpu=cortex-m7  -mthumb -fno-exceptions -fno-exceptions  -Wl,-Map=linker.map -Wl,-cref  -Wl,--gc-sections -Wl,-Map=AT25Q641_ExternalLoader.map -o "AT25Q641_ExternalLoader.elf" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '
	$(MAKE) --no-print-directory post-build

# Other Targets
clean:
	-$(RM) $(EXECUTABLES)$(OBJS)$(C_DEPS) AT25Q641_ExternalLoader.elf
	-@echo ' '

post-build:
	-arm-none-eabi-size "AT25Q641_ExternalLoader.elf"; arm-none-eabi-objdump -D "AT25Q641_ExternalLoader.elf" > "AT25Q641_ExternalLoader.lss"; cp "AT25Q641_ExternalLoader.elf" "AT25Q641_ExternalLoader.stldr"
	-@echo ' '

.PHONY: all clean dependents post-build

-include ../makefile.targets
