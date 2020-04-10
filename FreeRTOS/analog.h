/*
 * analog.h
 *
 *  Created on: 30/01/2015
 *      Author: jmcg
 */

#ifndef ANALOG_H_
#define ANALOG_H_

/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "semphr.h"
#include "queue.h"
#include "event_groups.h"


/* msp430 includes */
#include "msp430.h"
#include "MSP430F5xx_6xx/driverlib.h"

typedef struct {
    uint16_t Chan0Ref;
    uint16_t Chan0Vcc;
    uint16_t BattRef;
    uint16_t BattVcc;
    uint16_t TempRef;
    uint16_t TempVcc;

} conversionData;



void AnalogInit(void);
void AnalogStart(uint16_t period, uint8_t mode);
void AnalogConfigADC(uint8_t mode );
void AnalogStop(void);
inline BaseType_t AnalogRead(conversionData *ptrtodato, TickType_t ticks);
//UMA: cuidado, ahora mismo lee mezclados las temperaturas y los otros datos.
uint16_t AnalogTempCompensate(uint16_t lectura, uint8_t Vref);
uint16_t AnalogValueCompensate(uint16_t lectura, uint8_t Vref);

#endif /* ANALOG_H_ */
