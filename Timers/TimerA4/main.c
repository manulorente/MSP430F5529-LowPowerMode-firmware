// *****************************************************************************
// PROGRAMA : TimerA4.c (v1.0)
// TARGET : Placa de desarrollo MSP-EXP430F5929LP
// DESCRIPCION : Control de luminosidad de un LED mediante PWM
// AUTOR : Ignacio Herrero
// FECHA : 08/11/14
// ESQUEMA : Acceder al pin correspondiente a OUTn en el conector de la placa0.
// Por ejemplo, el pin 1.2 para OUT1 de TimerA0
// *****************************************************************************
#include <msp430f5529.h>
#define CICLO_MAX 0x752F
#define CICLO_75 ((CICLO_MAX)/4)*3
#define CICLO_50 CICLO_MAX/2
#define CICLO_25 CICLO_MAX/4
#define CICLO_10 CICLO_MAX/10
void main(void)
{
    WDTCTL = WDTPW + WDTHOLD;       // Detiene Watchdog
    P1DIR |= BIT2;                  // P1.2 como salida
    P1SEL |= BIT2;                  // y función especial OUT1

    // TimerA0,SMCLK/1 = 1Mhz, modo up, borra TACLR
    TA0CTL |= TASSEL__SMCLK + ID__1 + MC__UP  + TACLR;

    TA0CCTL1 |= CM_1 + OUTMOD_7;    // Salida OUT1 en modo reset/set
    TA0CCR0 = CICLO_MAX;            // Carga cuenta en TA0CCR0
    TA0CCR1 = CICLO_50;            // Carga cuenta en TA0CCR1

    __low_power_mode_0(); // Entrada LPM0, habilita interrupciones
    __no_operation(); // Para depuración
}
