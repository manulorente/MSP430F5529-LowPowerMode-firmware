// ******************************************************************************
// PROGRAMA	: usci_a1_uart_115200_XT2_4MHz.c (v1.2)
// TARGET	: MSP-EXP430F5529LP
// DESCRIPCION	: Recepción de datos por interrupción vía UART y transmisión
// AUTOR	: Manuel Lorente Almán
// FECHA	: 14-11-2018
// ESQUEMA  : ACLK= XT1CLK=32KHz, MCLK= SMCLK= XT2CLK=4MHz
// ******************************************************************************
//!
//!                MSP430F5529
//!             -----------------
//!         /|\|                 |
//!          | |                 |
//!          --|RST              |
//!            |                 |
//!            |     P4.4/UCA1TXD|------------>
//!            |                 | 115200 bps
//!            |     P4.5/UCA1RXD|<------------
//!
//!
//******************************************************************************

#include "driverlib.h"

#define BAUD_RATE                               115200


uint8_t receivedData = 0x00;

void main(void)
{
    //Stop WDT
    WDT_A_hold(WDT_A_BASE);

    PMM_setVCore(PMM_CORE_LEVEL_0); // Este valor del PMM permite llegar al MCLK trabajar hasta con 8MHz

    //Configuracion del reloj

    //Conecta el cristal XT2 -> P5.2 XIN y P5.3 XOUT
    GPIO_setAsPeripheralModuleFunctionInputPin(
        GPIO_PORT_P5,
        GPIO_PIN2
        );

    GPIO_setAsPeripheralModuleFunctionOutputPin(
            GPIO_PORT_P5,
            GPIO_PIN3
            );

    //Aranca el XT2 y lo pone a 4MHz con XT2OFF = 0 y XT2BYPASS = 0 (por defecto, pues no se llama a sus funciones de activación)
    UCS_XT2Start(UCS_XT2DRIVE_4MHZ_8MHZ);

    //Configuro SMCLK como XT2CLK
	UCS_clockSignalInit(
	   UCS_SMCLK,
	   UCS_XT2CLK_SELECT,
	   UCS_CLOCK_DIVIDER_1);

    //Configura los leds como salidas:
    GPIO_setAsOutputPin(GPIO_PORT_P4,GPIO_PIN7);
    GPIO_setAsOutputPin(GPIO_PORT_P1,GPIO_PIN0);

    //Configura los pines de la UART con la funcion UART
    //P4.4,5 = USCI_A1 TXD/RXD
    GPIO_setAsPeripheralModuleFunctionInputPin(
        GPIO_PORT_P4,
        GPIO_PIN5
        );

    GPIO_setAsPeripheralModuleFunctionOutputPin(
            GPIO_PORT_P4,
            GPIO_PIN4
            );

    //Inicializa los parámetros de la comunicación UART
    //Baudrate = 115200, clock freq XT2 = 4MHz
    USCI_A_UART_initParam param = {0};

    param.selectClockSource = USCI_A_UART_CLOCKSOURCE_SMCLK;
    param.clockPrescalar = 36; 	    // UCBRx - Parte entera de la división 4194304 (HZ) / 115200 (bps)
    param.firstModReg = 0;          // UCBRFx - Valor correspondiente en la tabla en modo UCOS16=0
    param.secondModReg = 3;         // UCBRSx - Valor correspondiente en la tabla
    param.parity = USCI_A_UART_NO_PARITY;
    param.msborLsbFirst = USCI_A_UART_LSB_FIRST;
    param.numberofStopBits = USCI_A_UART_ONE_STOP_BIT;
    param.uartMode = USCI_A_UART_MODE;
    param.overSampling = USCI_A_UART_LOW_FREQUENCY_BAUDRATE_GENERATION;

    if(STATUS_FAIL == USCI_A_UART_init(USCI_A1_BASE, &param))
    {
        while(1);	//Si falla la inicializacion nos pararemos aqui
    }

    //Enable UART module for operation
    USCI_A_UART_enable(USCI_A1_BASE);

    //Borra el flag de interrupción de recepción de la UART
    //Habilita la interrpción de la UART por recepción
    USCI_A_UART_clearInterruptFlag(USCI_A1_BASE,
                                   USCI_A_UART_RECEIVE_INTERRUPT);
    USCI_A_UART_enableInterrupt(USCI_A1_BASE,
                                USCI_A_UART_RECEIVE_INTERRUPT);

    //Enter LPM2, interrupts enabled
    while(1)
    {
    	__bis_SR_register(LPM2_bits + GIE);
    	__no_operation();
    }
}

//******************************************************************************
//
//This is the USCI_A1 interrupt vector service routine.
//
//******************************************************************************
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=USCI_A1_VECTOR
__interrupt
#elif defined(__GNUC__)
__attribute__((interrupt(USCI_A1_VECTOR)))
#endif
void USCI_A1_ISR(void)
{
    switch(__even_in_range(UCA1IV,4))
    {
    //Vector 2 - RXIFG
    case 2:

    	 //Recibe el dato y lo mete en la variable receivedData
        receivedData = USCI_A_UART_receiveData(USCI_A1_BASE);

        switch (receivedData)
        {
        	case 'a':
        		GPIO_setOutputLowOnPin(GPIO_PORT_P1,GPIO_PIN0);
        		break;
        	case 'e':
        		GPIO_setOutputHighOnPin(GPIO_PORT_P1,GPIO_PIN0);
        		break;
        	case 'A':
        		GPIO_setOutputLowOnPin(GPIO_PORT_P4,GPIO_PIN7);
        		break;
        	case 'E':
        		GPIO_setOutputHighOnPin(GPIO_PORT_P4,GPIO_PIN7);
        		break;
        	default:
        		break;
        }

        //Envía el eco
        USCI_A_UART_transmitData(USCI_A1_BASE,receivedData);
        break;
    default:
    	break;
    }
}
