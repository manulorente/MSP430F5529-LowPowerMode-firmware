// ******************************************************************************
// PROGRAMA	: LPM_consumo9.c (v1.0)
// TARGET	: MSP-EXP430F529LP
// DESCRIPCION	: Código para la medida de consumo
// AUTOR	: Eva González Parada
// FECHA	: 15-11-14
// ESQUEMA  : Modo Activo-->ACLK=?, MCLK=SMCLK=? LED1 y S2
// ******************************************************************************
#include <msp430.h>
#include "msp430f5529.h"

void config_puertos_LPM(void);

int main(void)
{
	WDTCTL = WDTPW + WDTHOLD;                 // Para watchdog timer
	config_puertos_LPM();

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

