################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Each subdirectory must supply rules for building sources it contributes
driverlib/MSP430F5xx_6xx/%.o: ../driverlib/MSP430F5xx_6xx/%.c $(GEN_OPTS) | $(GEN_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: GNU Compiler'
	"C:/ti/ccsv8/tools/compiler/msp430-gcc-7.3.1.24_win32/bin/msp430-elf-gcc.exe" -c -mmcu=msp430f5529 -D__MSP430F5529__ -I"C:/Master/MICRO/ejercicios/EF34_EC34/usci1_uart_G34/driverlib/MSP430F5xx_6xx" -I"C:/ti/ccsv8/tools/compiler/msp430-gcc-7.3.1.24_win32/msp430-elf/include" -I"C:/Master/MICRO/ejercicios/EF34_EC34/usci1_uart_G34" -I"C:/ti/ccsv8/ccs_base/msp430/include_gcc" -Os -g -gstrict-dwarf -Wall -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)"  $(GEN_OPTS__FLAG) -o"$@" "$<"
	@echo 'Finished building: "$<"'
	@echo ' '


