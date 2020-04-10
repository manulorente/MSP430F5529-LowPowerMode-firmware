// ******************************************************************************
// PROGRAMA	: LPM_consumo1.c (v1.0)
// TARGET	: MSP-EXP430F5529LP
// DESCRIPCION	: Código para la medida de consumo
// AUTOR	: Eva González Parada
// FECHA	: 15-11-14
// ESQUEMA  : Modo Activo-->ACLK=REFOCLK/XT1CKL=32KHz, MCLK=SMCLK=DCODIVCLK=1Mhz
// ******************************************************************************
#include <msp430.h>
#include "msp430f5529.h"

void config_puertos_LPM(void);

int main(void)
{
	WDTCTL = WDTPW + WDTHOLD;                 // Para watchdog timer
	config_puertos_LPM();
	__enable_interrupt();
	while (1)
	{
		__low_power_mode_4();
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

