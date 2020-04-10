/*
    FreeRTOS V8.1.2 - Copyright (C) 2014 Real Time Engineers Ltd.
    All rights reserved

    VISIT http://www.FreeRTOS.org TO ENSURE YOU ARE USING THE LATEST VERSION.

 ***************************************************************************
 *                                                                       *
 *    FreeRTOS provides completely free yet professionally developed,    *
 *    robust, strictly quality controlled, supported, and cross          *
 *    platform software that has become a de facto standard.             *
 *                                                                       *
 *    Help yourself get started quickly and support the FreeRTOS         *
 *    project by purchasing a FreeRTOS tutorial book, reference          *
 *    manual, or both from: http://www.FreeRTOS.org/Documentation        *
 *                                                                       *
 *    Thank you!                                                         *
 *                                                                       *
 ***************************************************************************

    This file is part of the FreeRTOS distribution.

    FreeRTOS is free software; you can redistribute it and/or modify it under
    the terms of the GNU General Public License (version 2) as published by the
    Free Software Foundation >>!AND MODIFIED BY!<< the FreeRTOS exception.

    >>!   NOTE: The modification to the GPL is included to allow you to     !<<
    >>!   distribute a combined work that includes FreeRTOS without being   !<<
    >>!   obliged to provide the source code for proprietary components     !<<
    >>!   outside of the FreeRTOS kernel.                                   !<<

    FreeRTOS is distributed in the hope that it will be useful, but WITHOUT ANY
    WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
    FOR A PARTICULAR PURPOSE.  Full license text is available from the following
    link: http://www.freertos.org/a00114.html

    1 tab == 4 spaces!

 ***************************************************************************
 *                                                                       *
 *    Having a problem?  Start by reading the FAQ "My application does   *
 *    not run, what could be wrong?"                                     *
 *                                                                       *
 *    http://www.FreeRTOS.org/FAQHelp.html                               *
 *                                                                       *
 ***************************************************************************

    http://www.FreeRTOS.org - Documentation, books, training, latest versions,
    license and Real Time Engineers Ltd. contact details.

    http://www.FreeRTOS.org/plus - A selection of FreeRTOS ecosystem products,
    including FreeRTOS+Trace - an indispensable productivity tool, a DOS
    compatible FAT file system, and our tiny thread aware UDP/IP stack.

    http://www.OpenRTOS.com - Real Time Engineers ltd license FreeRTOS to High
    Integrity Systems to sell under the OpenRTOS brand.  Low cost OpenRTOS
    licenses offer ticketed support, indemnification and middleware.

    http://www.SafeRTOS.com - High Integrity Systems also provide a safety
    engineered and independently SIL3 certified version for use in safety and
    mission critical applications that require provable dependability.

    1 tab == 4 spaces!
 */



/* Standard includes. */
#include <MSP430F5xx_6xx/driverlib.h>
#include <stdio.h>
#include <stdlib.h>

/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "semphr.h"
#include "queue.h"
#include "event_groups.h"

/* Hardware includes. */
#include "msp430.h"
#include "hal.h"

#include "serial.h"
#include "analog.h"


/* USB includes */
#include "USB_config/descriptors.h"
#include "USB_API/USB_Common/device.h"
#include "USB_API/USB_Common/usb.h"                 // USB-specific functions
#include "USB_API/USB_CDC_API/UsbCdc.h"
#include "USB_app/usbConstructs.h"

/* Definiciones del protocolo de aplicacion*/
#include "protocol.h"

/* BaudRate usado por la tarea del puerto serie */
#define mainCOM_TEST_BAUD_RATE			( 9600 )


/*-----------------------------------------------------------*/

/* declaracion de funciones */

/*
 * Funcion que configura el hardware.
 */
static void prvSetupHardware( void );


/*
 * Declaración de tareas
 */

extern void vUARTTask( void *pvParameters );	//Esta tarea está implementada en commands.c
static void prvUSBCDCTask( void *pvParameters );
static void ReadADCTask( void *pvParameters );
/*-----------------------------------------------------------*/

//Globales
volatile uint8_t button1Pressed=0;
volatile uint8_t button2Pressed=0;


static SemaphoreHandle_t mutex_send;

/*-----------------------------------------------------------*/


//Funcion main
void main( void )
{
    /* antes que nada, configuramos el micro y los perifericos */
    prvSetupHardware();

    //Inicializa la biblioteca serial.c
    xSerialPortInitMinimal(mainCOM_TEST_BAUD_RATE , 32 );

    USB_setup(TRUE, TRUE); // Init USB & events; if a host is present, connect

    //Inicializa el CDC
    cdcInit();

    /* luego creamos las tareas */
    if((xTaskCreate(vUARTTask, "Cmd", 256,NULL,tskIDLE_PRIORITY + 1, NULL) != pdTRUE))
    {
        while(1);
    }

    /*Crea la tarea que recibe los paquetes por el USB (puerto serie emulado) */
    if (xTaskCreate( prvUSBCDCTask, "CDC", 256, NULL, tskIDLE_PRIORITY+2, NULL )!=pdPASS)
    {
        while (1);
    }
    /*Crea la tarea que permite realizar la lectura de las conversiones*/
    if((xTaskCreate(ReadADCTask, "ADC", 256,NULL,tskIDLE_PRIORITY + 1, NULL) != pdTRUE))
    {
        while(1);
    }


    //Se crea un mutex
    mutex_send = xSemaphoreCreateMutex();

    if( mutex_send == NULL )
    {
        while(1);
    }



    //Configuración inicial del ADC
    AnalogInit();
    /* Arranca el scheduler */
    vTaskStartScheduler();	//Ejecuta el bucle del RTOS. De aquí no se retorna...


    /* Si llegamos a este punto es que algo raro ha pasado. Ponemos un bucle infinito para depurar */
    taskDISABLE_INTERRUPTS();
    for( ;; );
    __no_operation();
}
/*-----------------------------------------------------------*/



/*-----------------------------------------------------------*/
// Tarea que lee los datos del ADC y los envía al PC
// Los datos que se envian están sin calibrar
// se mandan en crudo

static void ReadADCTask( void *pvParameters )
{
    conversionData data;
    TipoPaquete header;
    // MLA: variables auxiliares para la manipulacion de datos
    uint16_t aux_Chan0Ref, aux_BattRef, aux_TempRef;
    uint8_t Vref;

    for( ;; )
    {
        AnalogRead(&data, portMAX_DELAY);

        // MLA: Realizo lectura de registro compartido REFCTL0 para ver Vref del ADC

        if (REFCTL0 & REFVSEL_2)        // MLA: 2.5V
        {
            Vref = 2;
        }
        else if (REFCTL0 & REFVSEL_1)   // MLA: 2V
        {
            Vref = 1;
        }
        else                            // MLA: 1.5V
        {
            Vref = 0;
        }

        aux_TempRef = data.TempRef;
        aux_Chan0Ref = data.Chan0Ref;
        aux_BattRef = data.BattRef;

        //MLA: Calibramos datos antes de enviar al PC, además pasamos como parámetro la referencia de tension
        data.TempRef = AnalogTempCompensate(aux_TempRef, Vref);
        data.Chan0Ref = AnalogValueCompensate(aux_Chan0Ref, Vref);
        data.BattRef = AnalogValueCompensate(aux_BattRef, Vref);

        header.parametro.analogData=data;
        header.comando=COMANDO_ADC_DATA;
        xSemaphoreTake( mutex_send,portMAX_DELAY);
        cdcSendData((uint8_t*)&header, sizeof(header), CDC0_INTFNUM, portMAX_DELAY);
        xSemaphoreGive( mutex_send );
    }
}

/*-----------------------------------------------------------*/






//Tarea que atiende los paquetes que llegan por el USB-CDC
//Gestión de los comandos
static void prvUSBCDCTask( void *pvParameters )
{
    TipoPaquete header;
    uint16_t count;
    for( ;; )
    {
        //Espera a recibir (cdcReceiveData es bloqueante)
        count = cdcReceiveData((uint8_t*)&header,sizeof(header), CDC0_INTFNUM);

        //Analiza lo recibido
        switch (header.comando)
        {
        case COMANDO_PING:
            //paquete ping, responde con el mismo comando
            xSemaphoreTake( mutex_send,portMAX_DELAY);
            cdcSendData((uint8_t*)&header, sizeof(header), CDC0_INTFNUM, portMAX_DELAY);
            xSemaphoreGive( mutex_send );
            break;
        case COMANDO_LED:
            //paquete LED. cambiamos el estado de los leds
            if (header.parametro.bits.bit0)
            {
                GPIO_setOutputHighOnPin(LED1_PORT,LED1_PIN);
            }
            else
            {
                GPIO_setOutputLowOnPin(LED1_PORT,LED1_PIN);

            }

            if (header.parametro.bits.bit1)
            {
                GPIO_setOutputHighOnPin(LED2_PORT,LED2_PIN);
            }
            else
            {
                GPIO_setOutputLowOnPin(LED2_PORT,LED2_PIN);
            }
            break;
        case COMANDO_SONDEA_BOTONES:
            header.parametro.bits.bit0=GPIO_getInputPinValue(BUTTON1_PORT, BUTTON1_PIN); //GPIO_getInputPinValue devuelve TRUE o FALSE, no un bitfield:
            header.parametro.bits.bit1=GPIO_getInputPinValue(BUTTON2_PORT, BUTTON2_PIN); //GPIO_getInputPinValue devuelve TRUE o FALSE, no un bitfield
            xSemaphoreTake( mutex_send,portMAX_DELAY);
            cdcSendData((uint8_t*)&header, sizeof(header), CDC0_INTFNUM, portMAX_DELAY);
            xSemaphoreGive( mutex_send );
            break;
        case COMANDO_ADC_START:
            AnalogStart(header.parametro.adcconf.ciclos, header.parametro.adcconf.mode);

            break;
        case COMANDO_ADC_STOP:
            AnalogStop();
            AnalogConfigADC(header.parametro.adcconf.mode);
            break;
        case COMANDO_LEERCAL_ADC:
        {
            struct s_TLV_ADC_Cal_Data *adccal;
            uint8_t longitud;

            //lee los datos de calibracion de temperatura, ganancia, offset y referencia.
            TLV_getInfo(TLV_TAG_ADCCAL,
                        0,
                        &longitud,
                        (uint16_t **)&adccal
            );
            header.parametro.tlv_adc=*adccal;
            header.comando=COMANDO_LEERCAL_ADC;
            xSemaphoreTake( mutex_send,portMAX_DELAY);

            cdcSendData((uint8_t*)&header, sizeof(header), CDC0_INTFNUM, portMAX_DELAY);
            xSemaphoreGive( mutex_send );


        }

        break;
        case COMANDO_LEERCAL_REF:
        {

            struct s_TLV_REF_Cal_Data *refcal;
            uint8_t longitud;

            //lee los datos de calibracion de temperatura, ganancia, offset y referencia.

            TLV_getInfo(TLV_TAG_REFCAL,
                        0,
                        &longitud,
                        (uint16_t **)&refcal
            );

            header.parametro.tlv_ref=*refcal;
            header.comando=COMANDO_LEERCAL_REF;
            xSemaphoreTake( mutex_send,portMAX_DELAY);
            cdcSendData((uint8_t*)&header, sizeof(header), CDC0_INTFNUM, portMAX_DELAY);
            xSemaphoreGive( mutex_send );


        }

        break;
        case COMANDO_SETREF:
        {

            //Antes de cambiar la referencia es necesario apagar el ADC
            AnalogStop();

            while(REF_BUSY == Ref_isRefGenBusy(REF_BASE));

            Ref_disableReferenceVoltage(REF_BASE);

            switch (header.parametro.adcconf.index_ref)
            {
            case 0:

                //Enciende la referencia de tension...
                Ref_setReferenceVoltage(REF_BASE,REF_VREF1_5V);
                break;
            case 1:

                //Enciende la referencia de tension...
                Ref_setReferenceVoltage(REF_BASE,REF_VREF2_0V);
                break;
            case 2:

                //Enciende la referencia de tension...
                Ref_setReferenceVoltage(REF_BASE,REF_VREF2_5V);
                break;

            }
            //Internal Reference ON
            Ref_enableReferenceVoltage(REF_BASE);

            //Configura el ADC y dispara la conversión
            AnalogConfigADC(header.parametro.adcconf.mode);
            AnalogStart(header.parametro.adcconf.ciclos, header.parametro.adcconf.mode);
        }
        break;

        default:
            //Comando hasta ahora no implementado. Respondo con COMANDO_RECHAZADO
            header.parametro.value=header.comando;
            header.comando=COMANDO_RECHAZADO;
            xSemaphoreTake( mutex_send,portMAX_DELAY);
            cdcSendData((uint8_t*)&header, sizeof(header), CDC0_INTFNUM, portMAX_DELAY);
            xSemaphoreGive( mutex_send );
            break;
        }

    }
}

/*-----------------------------------------------------------*/
//Esta es la funcion que realiza la configuración inicial del hardware.
static void prvSetupHardware( void )
{
    taskDISABLE_INTERRUPTS();

    /* Disable the watchdog. */

    WDT_A_hold(WDT_A_BASE); // Stop watchdog timer

    // Minumum Vcore setting required for the USB API is PMM_CORE_LEVEL_2 .

#ifndef DRIVERLIB_LEGACY_MODE
    PMM_setVCore(PMM_CORE_LEVEL_3);
#else

    PMM_setVCore(PMM_BASE, PMM_CORE_LEVEL_2);
#endif

    initPorts();           // Config GPIOS for low-power (output low)


    GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P5,GPIO_PIN4|GPIO_PIN5); //Conecta el cristal XT1LF...

    initClocks(configCPU_CLOCK_HZ);   // Config clocks. MCLK=SMCLK=FLL=configCPU_CLOCK_HZ; ACLK=XT1LF=32kHz

    initButtons();         // Init the two buttons
}
/*-----------------------------------------------------------*/



/* esta función configura el tick del RTOS, utilizando para ello el TimerA0, que no debe utilizarse para otras cosas.*/
/* The MSP430X port uses this callback function to configure its tick interrupt.
This allows the application to choose the tick interrupt source.
configTICK_VECTOR must also be set in FreeRTOSConfig.h to the correct
interrupt vector for the chosen tick interrupt source.  This implementation of
vApplicationSetupTimerInterrupt() generates the tick from timer A0, so in this
case configTICK_VECTOR is set to TIMER0_A0_VECTOR. */
void vApplicationSetupTimerInterrupt( void )
{
    const unsigned short usACLK_Frequency_Hz = 32768;

    /* Ensure the timer is stopped. */
    TA0CTL = 0;

    /* Run the timer from the ACLK. */
    TA0CTL = TASSEL_1;

    /* Clear everything to start with. */
    TA0CTL |= TACLR;

    /* Set the compare match value according to the tick rate we want. */
    TA0CCR0 = usACLK_Frequency_Hz / configTICK_RATE_HZ;

    /* Enable the interrupts. */
    TA0CCTL0 = CCIE;

    /* Start up clean. */
    TA0CTL |= TACLR;

    /* Up mode. */
    TA0CTL |= MC_1;
}
/*-----------------------------------------------------------*/

//Esta es la función "gancho" a la tarea Idle. Se puede usar para entrar en bajo consumo
void vApplicationIdleHook( void )
{
    /* Called on each iteration of the idle task.  In this case the idle task
	just enters a low(ish) power mode. */
    __bis_SR_register( LPM0_bits + GIE );
}
///*-----------------------------------------------------------*/


//Función gancho al desbordamiento de pila. El RTOS llama a a esta función si detecta dicho desbordamiento.
void vApplicationStackOverflowHook( TaskHandle_t pxTask, char *pcTaskName )
{
    ( void ) pxTask;
    ( void ) pcTaskName;

    /* Run time stack overflow checking is performed if
	configconfigCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2.  This hook
	function is called if a stack overflow is detected. */
    taskDISABLE_INTERRUPTS();
    for( ;; );
}

/*-----------------------------------------------------------*/

//Rutina de interrupcion del pulsador SW2 (p1.1).
void __attribute__ ((interrupt(PORT1_VECTOR))) Button1_ISR (void)

        {
    BaseType_t xHigherPriorityTaskWoken=pdFALSE; //debe ponerse a false

    __delay_cycles(8000);	//antirrebote Sw "cutre", gasta tiempo de CPU ya que las interrupciones tienen prioridad sobre las tareas
    //Por simplicidad lo dejamos así...
    if (GPIO_getInputPinValue(BUTTON1_PORT, BUTTON1_PIN))
    {
        //Ahora mismo no hago nada !!!//
    }

    GPIO_clearInterrupt(BUTTON1_PORT, BUTTON1_PIN);
    _low_power_mode_off_on_exit();

    portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
        }



//Rutina de interrupcion del pulsador 1 (P2.1)
void __attribute__ ((interrupt(PORT2_VECTOR))) Button2_ISR (void)

        {
    BaseType_t xHigherPriorityTaskWoken=pdFALSE; //debe ponerse a false

    __delay_cycles(8000);	//antirrebote Sw "cutre", gasta tiempo de CPU ya que las interrupciones tienen prioridad sobre las tareas
    //Por simplicidad lo dejamos así...
    if (GPIO_getInputPinValue(BUTTON2_PORT, BUTTON2_PIN))
    {
        /* Ahora mismo no hago nada */
    }

    GPIO_clearInterrupt(BUTTON2_PORT, BUTTON2_PIN);
    _low_power_mode_off_on_exit();

    portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
        }

/*
 * ======== UNMI_ISR ========
 * Interrupcion no enmascarable, Hay que ponerla por si se produce un error USB
 * Que esté instalado el manejador...
 */
#if defined(__TI_COMPILER_VERSION__) || (__IAR_SYSTEMS_ICC__)
#pragma vector = UNMI_VECTOR
__interrupt void UNMI_ISR (void)
#elif defined(__GNUC__) && (__MSP430__)
void __attribute__ ((interrupt(UNMI_VECTOR))) UNMI_ISR (void)
#else
#error Compiler not found!
#endif
{
    switch (__even_in_range(SYSUNIV, SYSUNIV_BUSIFG ))
    {
    case SYSUNIV_NONE:
        __no_operation();
        break;
    case SYSUNIV_NMIIFG:
        __no_operation();
        break;
    case SYSUNIV_OFIFG:
#ifndef DRIVERLIB_LEGACY_MODE
        UCS_clearFaultFlag(UCS_XT2OFFG);
        UCS_clearFaultFlag(UCS_DCOFFG);
        SFR_clearInterrupt(SFR_OSCILLATOR_FAULT_INTERRUPT);
#else
        UCS_clearFaultFlag(UCS_BASE, UCS_XT2OFFG);
        UCS_clearFaultFlag(UCS_BASE, UCS_DCOFFG);
        SFR_clearInterrupt(SFR_BASE, SFR_OSCILLATOR_FAULT_INTERRUPT);

#endif
        break;
    case SYSUNIV_ACCVIFG:
        __no_operation();
        break;
    case SYSUNIV_BUSIFG:
        // If the CPU accesses USB memory while the USB module is
        // suspended, a "bus error" can occur.  This generates an NMI.  If
        // USB is automatically disconnecting in your software, set a
        // breakpoint here and see if execution hits it.  See the
        // Programmer's Guide for more information.
        SYSBERRIV = 0; // clear bus error flag
        USB_disable(); // Disable
    }
}




