// *****************************************************************************
// PROGRAMA : TimerA1.c (v1.2)
// TARGET : Placa de desarrollo MSP-EXP430F5929LP
// DESCRIPCION : Parpadea el LED de la placa mediante el temporizador HW TA1
// AUTOR : Ignacio Herrero
// FECHA : 25/10/14
// *****************************************************************************
#include <msp430f5529.h>

void main(void)
{
    WDTCTL = WDTPW + WDTHOLD;           // Detiene Watchdog
    P1DIR |= BIT0;                      // P1.0 salida
    P1OUT |= BIT0;                      // Enciende LED
                                        // Configuración TIMERA Instrucción 1: TimerA1, ACLK/1,modo up, reinicia TACLR
    TA1CTL |= TASSEL__ACLK | ID__1 | MC__UP | TACLR;
    TA1CCR0 = 3276;                    // Instrucción 2: Carga cuenta en TA1CCR0 0.1seg TA1CCR=(0.1*32768)-1
    TA1CCTL0 = CCIE;                   // Instrucción: Habilitar interrupción

    __low_power_mode_0();               // Entrada LPM0, habilitación general de interrupciones
    __no_operation();
}
                                        // Rutina de interrupción de TIMER1_A0
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=TIMER1_A0_VECTOR
__interrupt
#elif defined(__GNUC__)
__attribute__((interrupt(TIMER1_A0_VECTOR)))
#endif
void TIMER1_A0_ISR(void){
    P1OUT ^= 0x01;                      // Toggle P1.0
}
