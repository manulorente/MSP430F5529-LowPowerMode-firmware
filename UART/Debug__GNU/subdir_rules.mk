################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.c $(GEN_OPTS) | $(GEN_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: GNU Compiler'
	"C:/ti/ccsv8/tools/compiler/msp430-gcc-7.3.1.24_win32/bin/msp430-elf-gcc.exe" -c -mmcu=msp430f5529 -I"C:/ti/ccsv8/ccs_base/msp430/include_gcc" -I"C:/ti/ccsv8/tools/compiler/msp430-gcc-7.3.1.24_win32/msp430-elf/include" -I"C:/Master/MICRO/ejercicios/EF34_EC34/usci1_uart_G34/driverlib/MSP430F5xx_6xx" -Os -g -gdwarf-3 -gstrict-dwarf -Wall -mlarge -mhwmult=f5series -mcode-region=none -mdata-region=none -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)"  $(GEN_OPTS__FLAG) -o"$@" "$<"
	@echo 'Finished building: "$<"'
	@echo ' '


