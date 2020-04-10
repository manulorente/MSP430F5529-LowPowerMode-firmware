// *****************************************************************************
// PROGRAMA : TimerA3.c (v1.1)
// TARGET : Placa de desarrollo MSP-EXP430F5929LP
// DESCRIPCION : Parpadeo desfasado de varios LEDs mediante PWM
// AUTOR : Ignacio Herrero
// FECHA : 25/10/14
// ESQUEMA : Decidir 3 pines con salida OUTn. Por ejemplo, OUT1, OUT2, y OUT3
// de TA0, conectados a los pines P1.2,P1.3,P1.4
//
// *****************************************************************************
#include <msp430f5529.h>

void main(void)
{
    const unsigned int time_base = 65535;               //2seg 65535=(2*32768)-1

    WDTCTL = WDTPW + WDTHOLD;                           // Detiene Watchdog
    P1DIR |= BIT2 + BIT3 + BIT4;                        // P1.2 , P1.3, y P1.4 como salidas OUT1 , OUT2, y OUT3 del TimerA0
    P1SEL |= BIT2 + BIT3 + BIT4;

    // TimerA0,ACLK/1 = 32KHz,modo cont, borra TACLR
    TA0CTL |= TASSEL__ACLK + ID__1 + MC__CONTINUOUS  + TACLR;
    TA0CCR0 = time_base;                                    // (paso por 0 cada 2 segundos) 2seg TA0CCR=(2*32768)-1

    TA0CCTL1 |= CM_1 + OUTMOD_4;                               // Salida OUT1 en modo toggle
    TA0CCTL2 |= CM_1 + OUTMOD_4;                               // Salida OUT2 en modo toggle
    TA0CCTL3 |= CM_1 + OUTMOD_4;                               // Salida OUT3 en modo toggle

    TA0CCR1 = time_base/3;                                    // Carga cuenta en TA0CCR1
    TA0CCR2 = time_base/2;                                    // Carga cuenta en TA0CCR2
    TA0CCR3 = time_base;                                      // Carga cuenta en TA0CCR

    __low_power_mode_0();           // Entrada LPM0, habilita interrupciones
    __no_operation(); // Para depuración
}

