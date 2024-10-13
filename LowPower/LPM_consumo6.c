// ******************************************************************************
// PROGRAMA	: LPM_consumo6.c (v1.0)
// TARGET	: MSP-EXP430F529LP
// DESCRIPCION	: C�digo para la medida de consumo
// AUTOR	: Eva Gonz�lez Parada
// FECHA	: 15-11-14
// ESQUEMA  : Modo Activo-->ACLK=XT1CLK=32Khz, MCLK=DCODIVCLK=1Mhz, SMCLK=XT2CLK=4Mhz
// ******************************************************************************
#include <msp430.h>
#include "msp430f5529.h"

void config_puertos_LPM(void);

int main(void)
{
	WDTCTL = WDTPW + WDTHOLD;                 // Para watchdog timer
	config_puertos_LPM();

	P5DIR &= ~BIT4;						// Configuraci�n XIN y XOUT de XT1
	P5DIR |= BIT5;
	P5SEL|= BIT4|BIT5;
	UCSCTL6 &= ~ (XT1DRIVE0 |XT1DRIVE1); //Configuraci�n corriente de ataque

	P5DIR &= ~BIT2;							//XT2-> XIN y XOUT
	P5DIR |= BIT3;
	P5SEL|= BIT2|BIT3;
	UCSCTL6 &= ~ XT2OFF;					//Activaci�n XT2
	UCSCTL6 &= ~ (XT2DRIVE0|XT2DRIVE1);		//Corriente de ataque m�nima 4MHz
	UCSCTL4 |= SELS_5;               		// SMCLK=XT2CLK

	do 											// Bucle de testeo de fallo de oscilador
	  {
	    UCSCTL7 &= ~(XT2OFFG + XT1LFOFFG + DCOFFG);
	                                            // Borrado de los flags de fallo XT2,XT1,DCO
	    SFRIFG1 &= ~OFIFG;                      // Borrado del flag de fallo OFIFG
	  }while (SFRIFG1&OFIFG);                   // FIN Bucle de testeo de fallo de oscilador


	__enable_interrupt();

	while (1)
	{
		__low_power_mode_2();
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

