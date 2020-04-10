#include <msp430f5529.h>

const unsigned int base_a = 982;    // Carga cuenta en TA1CCR1 para cuenta0 de 60ms TA1CCR1=(0.06*1048576/64)-1
const unsigned int base_b = 9829;   // Carga cuenta en TA1CCR2 para cuenta de 600ms TA1CCR2=(0.6*1048576/64)-1

void main(void)
{
    WDTCTL = WDTPW + WDTHOLD;   // Detiene Watchdog

    P1DIR |= BIT0;              // P1.0 salida
    P4DIR |= BIT7;              // P4.7 salida
    P1OUT &= ~BIT0;             // Apaga LED Rojo
    P4OUT &= ~BIT7;             // Apaga LED Verde

    // TimerA1, SMCLK (1Mhz),modo continuo, borra TACLR, habilita INT overflow
    TA1CTL |= TASSEL__SMCLK + ID__8 + MC__UP + TACLR;
    TA1EX0 |= TAIDEX_7;         // debemos dividir la frecuencia entre 256 pero sólo tenemos hasta 64
    TA1CTL |= TAIE;
    TA1CTL &= ~TAIFG;
    TA1CCR0 = 0xFFFF;

    TA1CCR1 = base_a;
    TA1CCTL1 |= CM_1 + OUTMOD2 + CCIE;
    TA1CCTL1 &= ~CCIFG;

    TA1CCR2 = base_b;
    TA1CCTL2 |= CM_1 + OUTMOD2 + CCIE;
    TA1CCTL2 &= ~CCIFG;

    __low_power_mode_0();     // Habilitacion general de interrupciones
    __no_operation();

}
// Directivas de definicion de Rutina/s de interrupción (TIMER1_A1_VECTOR)
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector= TIMER1_A1_VECTOR
__interrupt
#elif defined(__GNUC__)
__attribute__((interrupt(TIMER1_A1_VECTOR))) // Indica el valor del vector de interrupción
#endif

void TIMER1_A1_ISR(void){
    switch(__even_in_range(TA1IV, 14))
    {
    case TA1IV_TACCR1:                  // Código para TA1CCR1 (se debe ejecutar cada 60ms)
            P4OUT ^= BIT7;              // Cambia estado LED verde
            TA1CCR1 += base_a;             // Establece nuevo valor de cuenta en TA1CCR1
            break;
    case TA1IV_TACCR2:                  // Código para TA1CCR2 (se debe ejecutar cada 600ms)
            P1OUT ^= BIT0;              // Cambia estado LED rojo
            TA1CCR2 += base_b;            // Establece nuevo valor de cuenta en TA1CCR2
            break;
    case TA1IV_TAIFG:                   // Código para TAIFG
            P4OUT |= BIT7;              // Finaliza parpadeo y enciende ambos LEDs
            P1OUT |= BIT0;
            TA1CTL |= MC__STOP | TACLR; // Configura TimerA1, modo parado y borra TACLR.
            break;
    }
}

/*
// *****************************************************************************
// PROGRAMA : TimerA2.c (v1.3)
// TARGET : Placa de desarrollo MSP-EXP430F5529LP
// DESCRIPCION : Modo comparación 2 TA1CCR0 y TA1CCR1. Actuan sobre los LEDs
// para establecer distintas cadencias de parpadeo, hasta la
// llegada del Timer a su cuenta máxima.
// AUTOR : Ignacio Herrero
// FECHA : 25/10/12
// *****************************************************************************
#include <msp430f5529.h>
void main(void)
{
    WDTCTL = WDTPW + WDTHOLD;   // Detiene Watchdog

    P1DIR |= BIT0;              // P1.0 salida
    P4DIR |= BIT7;              // P4.7 salida
    P1OUT &= ~BIT0;             // Apaga LED Rojo
    P4OUT &= ~BIT7;             // Apaga LED Verde

    // TimerA1, ACLK/8 (4Khz),modo continuo, borra TACLR, habilita INT overflow
    TA1CTL |= TASSEL__ACLK + ID__8 + MC__UP + TACLR;
    TA1CTL |= TAIE;
    TA1CTL &= ~TAIFG;
    TA1CCR0 = 0xFFFF;

    TA1CCR1 = 246;              // Carga cuenta en TA1CCR1 para cuenta0 de 60ms TA1CCR1=(0.06*32768/8)-1
    TA1CCTL1 |= CM_1 + OUTMOD2 + CCIE;
    TA1CCTL1 &= ~CCIFG;

    TA1CCR2 = 2456;             // Carga cuenta en TA1CCR2 para cuenta de 600ms TA1CCR2=(0.6*32768/8)-1
    TA1CCTL2 |= CM_1 + OUTMOD2 + CCIE;
    TA1CCTL2 &= ~CCIFG;

    __low_power_mode_0();     // Habilitacion general de interrupciones
    __no_operation();

}
// Directivas de definicion de Rutina/s de interrupción (TIMER1_A1_VECTOR)
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector= TIMER1_A1_VECTOR
__interrupt
#elif defined(__GNUC__)
__attribute__((interrupt(TIMER1_A1_VECTOR))) // Indica el valor del vector de interrupción
#endif

void TIMER1_A1_ISR(void){
    switch(__even_in_range(TA1IV, 14))
    {
    case TA1IV_TACCR1:                  // Código para TA1CCR1 (se debe ejecutar cada 60ms)
            P4OUT ^= BIT7;              // Cambia estado LED verde
            TA1CCR1 += 246;             // Establece nuevo valor de cuenta en TA1CCR1
            break;
    case TA1IV_TACCR2:                  // Código para TA1CCR2 (se debe ejecutar cada 600ms)
            P1OUT ^= BIT0;              // Cambia estado LED rojo
            TA1CCR2 += 2456;            // Establece nuevo valor de cuenta en TA1CCR2
            break;
    case TA1IV_TAIFG:                   // Código para TAIFG
            P4OUT |= BIT7;              // Finaliza parpadeo y enciende ambos LEDs
            P1OUT |= BIT0;
            TA1CTL |= MC__STOP | TACLR; // Configura TimerA1, modo parado y borra TACLR.
            break;
    }
}
*/
