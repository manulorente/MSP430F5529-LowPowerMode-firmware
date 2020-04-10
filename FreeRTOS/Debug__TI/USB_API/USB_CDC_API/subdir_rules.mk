################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Each subdirectory must supply rules for building sources it contributes
USB_API/USB_CDC_API/%.obj: ../USB_API/USB_CDC_API/%.c $(GEN_OPTS) | $(GEN_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: MSP430 Compiler'
	"C:/ti/ccsv8/tools/compiler/ti-cgt-msp430_18.1.4.LTS/bin/cl430" -vmspx --data_model=restricted --use_hw_mpy=F5 --include_path="C:/ti/ccsv8/ccs_base/msp430/include" --include_path="C:/Users/Manuel/workspace_v8/ADC_FreeRTOS-MSP430-newlib_v1_0" --include_path="C:/ti/ccsv8/tools/compiler/ti-cgt-msp430_18.1.4.LTS/include" --include_path="C:/Users/Manuel/workspace_v8/ADC_FreeRTOS-MSP430-newlib_v1_0" --include_path="C:/Users/Manuel/workspace_v8/ADC_FreeRTOS-MSP430-newlib_v1_0/FreeRTOS/Source/include" --include_path="C:/Users/Manuel/workspace_v8/ADC_FreeRTOS-MSP430-newlib_v1_0/MSP430F5xx_6xx" --include_path="C:/Users/Manuel/workspace_v8/ADC_FreeRTOS-MSP430-newlib_v1_0/USB_config" --include_path="C:/Users/Manuel/workspace_v8/ADC_FreeRTOS-MSP430-newlib_v1_0/FreeRTOS/Source/portable/CCS/MSP430X" --advice:power=all --define=__MSP430F5529__ --define=CCS_MSP430X --define=UART_BUFFERED --define=WANT_FREERTOS_SUPPORT --define=WANT_CMDLINE_HISTORY -g --printf_support=full --diag_warning=225 --diag_wrap=off --display_error_number --silicon_errata=CPU21 --silicon_errata=CPU22 --silicon_errata=CPU23 --silicon_errata=CPU40 --preproc_with_compile --preproc_dependency="USB_API/USB_CDC_API/$(basename $(<F)).d_raw" --obj_directory="USB_API/USB_CDC_API" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '


