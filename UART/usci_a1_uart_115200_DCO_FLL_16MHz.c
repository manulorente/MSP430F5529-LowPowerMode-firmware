// ******************************************************************************
// PROGRAMA	: usci_a1_uart_DCO_FLL_16MHz.c (v1.2)
// TARGET	: MSP-EXP430F5529LP
// DESCRIPCION	: Recepción de datos por interrupción vía UART y transmisión
// AUTOR	: José M. Cano y Eva González Parada
// FECHA	: 10-12-14
// ESQUEMA  : ACLK= XT1CLK=32KHz, MCLK= SMCLK= DCODIVCLK=16MHz
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

#define BAUD_RATE                               9600


uint8_t receivedData = 0x00;

void main(void)
{
    //Stop WDT
    WDT_A_hold(WDT_A_BASE);

    PMM_setVCore(PMM_CORE_LEVEL_3);

    //Configuracion del reloj

    //Conecta el cristal XT1
    GPIO_setAsPeripheralModuleFunctionInputPin(
        GPIO_PORT_P5,
        GPIO_PIN4
        );

    GPIO_setAsPeripheralModuleFunctionOutputPin(
            GPIO_PORT_P5,
            GPIO_PIN5
            );


    //Aranca el XT1 a LF (y espera a que se estabilice)
    UCS_LFXT1Start(UCS_XT1_DRIVE3,UCS_XCAP_3);

    //Configuro el FLL para usar XT1CLK como referencia
    UCS_clockSignalInit(
	   UCS_FLLREF,
	   UCS_XT1CLK_SELECT,
	   UCS_CLOCK_DIVIDER_1);

    //Configuro ACLK como XT1CLK, a modo de ejemplo, por en este programa ACLK no se utiliza
	UCS_clockSignalInit(
	   UCS_ACLK,
	   UCS_XT1CLK_SELECT,
	   UCS_CLOCK_DIVIDER_1);

	//Configuro la frecuencia (16MHz) que quiero para el DCO FLL (y espera unos ciclos a que se estabilice)
    UCS_initFLLSettle(
        16000000/1000,
        16000000/32768);

    //Ojo!! SI en vez de llamar a esta llamo a UCS_initFLL, no espera a que se estabilice, y por tanto al entrar en
    //un modo más profundo o igual que LPM1, se desconecta el FLL, el DCO no se acaba de estabilizar
    // y la UART no recibe a la frecuencia correcta

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
    //Baudrate = 115200, clock freq DCO FLL = 16MHz
    USCI_A_UART_initParam param = {0};

    param.selectClockSource = USCI_A_UART_CLOCKSOURCE_SMCLK;
    param.clockPrescalar = 145; 	// UCBRx - Parte entera de la división 16777216 (HZ) entre 115200 (bps)
    param.firstModReg = 0;          // UCBRFx - Valor correspondiente en la tabla en modo UCOS16=0
    param.secondModReg = 5;         //UCBRSx - Valor correspondiente en la tabla
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
