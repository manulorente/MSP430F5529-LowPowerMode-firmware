// ******************************************************************************
// PROGRAMA	: LPM_consumo10.c (v1.0)
// TARGET	: MSP-EXP430F529LP
// DESCRIPCION	: Código para la medida de consumo
// AUTOR	: Eva González Parada
// FECHA	: 15-11-14
// ESQUEMA  : Modo Activo-->ACLK=?, MCLK=SMCLK=?
// ******************************************************************************
#include <msp430.h>
#include "msp430f5529.h"

void config_puertos_LPM(void);

int main(void)
{
	WDTCTL = WDTPW + WDTHOLD;                 // Para watchdog timer
	config_puertos_LPM();
	P1DIR |= BIT0;      // P1.0 (LED) como salida digital
	P1OUT &=~ BIT0;     // P1.0 (LED) apagado
	P1DIR &=~ BIT1;      // P1.1 (S2) como entrada digital
	P1REN |= BIT1;      // Resistencia de pull-up
	P1OUT |= BIT1;      // en P1.1 (pulsador derecho)
	P1IE  |= BIT1;      // Habilita la interrupción por P1.1
	P1IES |= BIT1;      // Interrupción por flanco de bajada en P1.1
	P1IFG &=~BIT1;     	// Baja el flag de interrupción de P1.1

	P5DIR &= ~BIT4;						// Configuración XIN y XOUT de XT1
	P5DIR |= BIT5;
	P5SEL|= BIT4|BIT5;
	UCSCTL6 &= ~ (XT1DRIVE0 |XT1DRIVE1); //Configuración corriente de ataque

	do 											// Bucle de testeo de fallo de oscilador
	  {
	    UCSCTL7 &= ~(XT2OFFG + XT1LFOFFG + DCOFFG);
	                                            // Borrado de los flags de fallo XT2,XT1,DCO
	    SFRIFG1 &= ~OFIFG;                      // Borrado del flag de fallo OFIFG
	  }while (SFRIFG1&OFIFG);                   // FIN Bucle de testeo de fallo de oscilador


	__enable_interrupt();

	while (1)
	{
		__no_operation();
	}
}

#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=PORT1_VECTOR
__interrupt
#elif defined(__GNUC__)
__attribute__((interrupt(PORT1_VECTOR)))
#endif
void P1_ISR(void){
	P1IFG &= ~BIT1;     // Puesta a 0 del flag de aviso de interrupción
	P1OUT ^= BIT0;      // Conmutación LED1
}
void config_puertos_LPM(void){
	P1DIR=0xFF;
	P1OUT=0x00;
	P2DIR=0xFF;
	P2OUT=0x00;
	P3DIR=0xFF;
	P3OUT=0x00;
	P4DIR=0xFF;
	P4OUT=0x00;
	P5DIR=0xFF;
	P5OUT=0x00;
	P6DIR=0xFF;
	P6OUT=0x00;
	P7DIR=0xFF;
	P7OUT=0x00;
	P8DIR=0xFF;
	P8OUT=0x00;
	PJDIR=0xFF;
	PJOUT=0x00;
	USBPWRCTL &= ~VUSBEN;
	USBPWRCTL &= ~SLDOEN;
}

