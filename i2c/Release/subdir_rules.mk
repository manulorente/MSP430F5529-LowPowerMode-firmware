################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.c $(GEN_OPTS) | $(GEN_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: GNU Compiler'
	"C:/ti/ccsv8/tools/compiler/msp430-gcc-7.3.2.154_win32/bin/msp430-elf-gcc.exe" -c -mmcu=msp430X -D__MSP430F5529__ -I"C:/ti/ccsv8/tools/compiler/msp430-gcc-7.3.2.154_win32/msp430-elf/include" -I"C:/ti/ccsv8/ccs_base/msp430/include_gcc" -O2 -Os -Wall -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)"  $(GEN_OPTS__FLAG) -o"$@" "$<"
	@echo 'Finished building: "$<"'
	@echo ' '


