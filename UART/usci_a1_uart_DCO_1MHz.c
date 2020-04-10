// ******************************************************************************
// PROGRAMA	: usci_a1_uart_DCO_1MHz.c (v1.2)
// TARGET	: MSP-EXP430F5529LP
// DESCRIPCION	: Recepción de datos por interrupción vía UART y transmisión
// AUTOR	: José M. Cano y Eva González Parada
// FECHA	: 10-12-14
// ESQUEMA  : ACLK= REFOCLK=32KHz, MCLK= SMCLK= DCODIVCLK=1048576Hz
// ******************************************************************************
//!
//!                MSP430F5529
//!             -----------------
//!         /|\|                 |
//!          | |                 |
//!          --|RST              |
//!            |                 |
//!            |     P4.4/UCA1TXD|------------>
//!            |                 | 9600
//!            |     P4.5/UCA1RXD|<------------
//!
//!
//******************************************************************************

#include "driverlib.h"


#define BAUD_RATE                               9600

uint8_t receivedData = 0x00;

void main(void)
{
    //Stop WDT
    WDT_A_hold(WDT_A_BASE);

    //Configura los leds:
    GPIO_setAsOutputPin(GPIO_PORT_P4,GPIO_PIN7);
    GPIO_setAsOutputPin(GPIO_PORT_P1,GPIO_PIN0);


    // Configura los terminales de TX y RX
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
    //Baudrate = 9600, clock freq = 1048576Hz
    USCI_A_UART_initParam param = {0};

    param.selectClockSource = USCI_A_UART_CLOCKSOURCE_SMCLK; // Fuente de reloj SMCLK por defecto
    param.clockPrescalar = 109;	// Parte entera de la división 1048576 (HZ) entre 9600 (bps)
    							// equivalente a lo que hay que cargar en  UCBRx
    param.firstModReg = 0;		// Correspondiente a UCBRFx en modo UCOS16=0 este valor es 0
    param.secondModReg = 2;		// Valor obtenido de la tabla para UCBRSx
    param.parity = USCI_A_UART_NO_PARITY;
    param.msborLsbFirst = USCI_A_UART_LSB_FIRST;
    param.numberofStopBits = USCI_A_UART_ONE_STOP_BIT;
    param.uartMode = USCI_A_UART_MODE;
    param.overSampling = USCI_A_UART_LOW_FREQUENCY_BAUDRATE_GENERATION; // UCOS16=0

    if(STATUS_FAIL == USCI_A_UART_init(USCI_A1_BASE, &param))
    {
        while(1);	//Si falla la inicializacion nos pararemos aqui
    }

    //Habilitación de la UART para operar
    USCI_A_UART_enable(USCI_A1_BASE);

    //Borra el flag de interrupción de recepción de la UART
    //Habilita la interrupción de la UART por recepción
    USCI_A_UART_clearInterruptFlag(USCI_A1_BASE,
                                   USCI_A_UART_RECEIVE_INTERRUPT);
    USCI_A_UART_enableInterrupt(USCI_A1_BASE,
                                USCI_A_UART_RECEIVE_INTERRUPT);

    //Entra en modo LPM3, y habilita las interrupciones
    while(1)
    {
    	__bis_SR_register(LPM3_bits + GIE);
    	__no_operation();
    }
}

//******************************************************************************
//
//Rutina de tratamiento de la USCI_A1, utilizando el registro interrupt vector
// UCA1IV
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
        	case 'a': // Si el dato recibido es un 'a' entonces...
        		GPIO_setOutputLowOnPin(GPIO_PORT_P1,GPIO_PIN0);
        		break;
        	case 'e': // Si el dato recibido es un 'e' entonces...
        		GPIO_setOutputHighOnPin(GPIO_PORT_P1,GPIO_PIN0);
        		break;
        	case 'A': // Si el dato recibido es un 'A' entonces...
        		GPIO_setOutputLowOnPin(GPIO_PORT_P4,GPIO_PIN7);
        		break;
        	case 'E': // Si el dato recibido es un 'E' entonces...
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
