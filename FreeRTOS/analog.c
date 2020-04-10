// analog.c
// Created on: 30/01/2015
// Modified: 30/11/2017
// Author: JMCG, EGP
// DTE UMA


#ifndef ANALOG_C_
#define ANALOG_C_

#include <MSP430F5xx_6xx/driverlib.h>
#include<stdbool.h>
#include<stdint.h>


// UMA
/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "semphr.h"
#include "queue.h"
#include "event_groups.h"


/* msp430 includes */
#include "msp430.h"
#include "analog.h"

static QueueHandle_t adcqueue; // UMA
static struct s_TLV_ADC_Cal_Data *adccal;
static struct s_TLV_REF_Cal_Data *refcal;



/**
 * Initializa y configura el ADC
 * para convertir muestrar  varios canales con distintas referencias
 * crea una cola de mensaje
 **/

void AnalogInit(void)
{
        uint8_t longitud;

        //lee los datos de calibracion de temperatura, ganancia, offset y referencia.
        TLV_getInfo(TLV_TAG_ADCCAL,
                    0,
                    &longitud,
                    (uint16_t **)&adccal
                    );

        TLV_getInfo(TLV_TAG_REFCAL,
                    0,
                    &longitud,
                    (uint16_t **)&refcal
                    );

        ADC12_A_configureMemoryParam param0 = {0};

        //Habilita el channel A0 del ADC
        GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P6,
                                                   GPIO_PIN0
                                                   );

        // Inicializa  ADC12
        ADC12_A_init (ADC12_A_BASE,ADC12_A_SAMPLEHOLDSOURCE_SC, ADC12_A_CLOCKSOURCE_ADC12OSC, ADC12_A_CLOCKDIVIDER_1);

        //Enciende  ADC12
        ADC12_A_enable(ADC12_A_BASE);

        //Programa la conversión por disparo sotfware y MCS=1
        ADC12_A_setupSamplingTimer (ADC12_A_BASE , ADC12_A_CYCLEHOLD_1024_CYCLES, ADC12_A_CYCLEHOLD_1024_CYCLES, ADC12_A_MULTIPLESAMPLESENABLE);



        while(REF_BUSY == Ref_isRefGenBusy(REF_BASE));

        //Enciende el módulo de referencia de tensión
        //Activa la referencia interna de 2.0V (valor por defecto)
        //
        Ref_setReferenceVoltage(REF_BASE,REF_VREF2_0V);
        Ref_enableReferenceVoltage(REF_BASE);

        // Configuración de los registros de memoria asociados a la conversión
        //se define distintos canales y distintas referencias para cada canal (interna y Vcc)
        //Definición de la secuencia de muestro y conversión

        //Canal 0 con referencia interna
        param0.memoryBufferControlIndex = ADC12_A_MEMORY_0;
        param0.inputSourceSelect = ADC12_A_INPUT_A0;
        param0.positiveRefVoltageSourceSelect = ADC12_A_VREFPOS_INT;
        param0.negativeRefVoltageSourceSelect = ADC12_A_VREFNEG_AVSS;
        param0.endOfSequence = ADC12_A_NOTENDOFSEQUENCE;
        ADC12_A_configureMemory(ADC12_A_BASE,&param0);

        //Canal 0 con referencia Vcc
        param0.memoryBufferControlIndex = ADC12_A_MEMORY_1;
        param0.inputSourceSelect = ADC12_A_INPUT_A0;
        param0.positiveRefVoltageSourceSelect = ADC12_A_VREFPOS_AVCC;
        param0.negativeRefVoltageSourceSelect = ADC12_A_VREFNEG_AVSS;
        param0.endOfSequence = ADC12_A_NOTENDOFSEQUENCE;
        ADC12_A_configureMemory(ADC12_A_BASE,&param0);

        //Vcc/2 con referencia interna
        param0.memoryBufferControlIndex = ADC12_A_MEMORY_2;
        param0.inputSourceSelect = ADC12_A_INPUT_BATTERYMONITOR;
        param0.positiveRefVoltageSourceSelect = ADC12_A_VREFPOS_INT;
        param0.negativeRefVoltageSourceSelect = ADC12_A_VREFNEG_AVSS;
        param0.endOfSequence = ADC12_A_NOTENDOFSEQUENCE;
        ADC12_A_configureMemory(ADC12_A_BASE,&param0);

        //Vcc/2 con referencia vcc (inutil)
        param0.memoryBufferControlIndex = ADC12_A_MEMORY_3;
        param0.inputSourceSelect = ADC12_A_INPUT_BATTERYMONITOR;
        param0.positiveRefVoltageSourceSelect = ADC12_A_VREFPOS_AVCC;
        param0.negativeRefVoltageSourceSelect = ADC12_A_VREFNEG_AVSS;
        param0.endOfSequence = ADC12_A_NOTENDOFSEQUENCE;
        ADC12_A_configureMemory(ADC12_A_BASE,&param0);

        //Temperatura con referencia interna
        param0.memoryBufferControlIndex = ADC12_A_MEMORY_4;
        param0.inputSourceSelect = ADC12_A_INPUT_TEMPSENSOR;;
        param0.positiveRefVoltageSourceSelect = ADC12_A_VREFPOS_INT;
        param0.negativeRefVoltageSourceSelect = ADC12_A_VREFNEG_AVSS;
        param0.endOfSequence = ADC12_A_NOTENDOFSEQUENCE;
        ADC12_A_configureMemory(ADC12_A_BASE,&param0);

        //Temperatura con referencia vcc
        param0.memoryBufferControlIndex = ADC12_A_MEMORY_5;
        param0.inputSourceSelect = ADC12_A_INPUT_TEMPSENSOR;
        param0.positiveRefVoltageSourceSelect = ADC12_A_VREFPOS_AVCC;
        param0.negativeRefVoltageSourceSelect = ADC12_A_VREFNEG_AVSS;
        param0.endOfSequence = ADC12_A_ENDOFSEQUENCE;
        ADC12_A_configureMemory(ADC12_A_BASE,&param0);

        //Crea la cola en la que se van almacenando los datos convertidos
        adcqueue=xQueueCreate(8,sizeof(conversionData)); // UMA
        if (!adcqueue)
        {
            while(1);
        }



}


/**
* Configura los modos de disparo de conversión del ADC
* Gestiona el cambio de la fuente de disparo de la conversión
* Dos posibilidades: disparo SW de la conversión
* y el disparo mediante el TIMER B
**/

void AnalogConfigADC(uint8_t mode )
{

		switch (mode){

		//Disparo SW
		case 0:
		    ADC12_A_init (ADC12_A_BASE,ADC12_A_SAMPLEHOLDSOURCE_SC, ADC12_A_CLOCKSOURCE_ADC12OSC, ADC12_A_CLOCKDIVIDER_1);
		    ADC12_A_enable(ADC12_A_BASE);
		    ADC12_A_setupSamplingTimer (ADC12_A_BASE , ADC12_A_CYCLEHOLD_1024_CYCLES, ADC12_A_CYCLEHOLD_1024_CYCLES, ADC12_A_MULTIPLESAMPLESENABLE);
		    break;

		//Disparo Timer
		case 1:
			// MLA: para programar el timer en modo UP con fuente de reloj ACLK, sin división y con el periodo establecido
			const unsigned int time_base = 65535;               //T(seg) -> 65535=(T*32768)-1

			// MLA: Fuente de reloj ACLK/1 + Modo UP + clear para comenzar a comparar
			TB0CTL = |= TBSSEL__ACLK + ID__1 + MC__UP  + TACLR;

			// MLA: Modo PWM - reset/set - ¿TB0CCTL1?
			TB0CCTL1 |= CM_1 + OUTMOD_7
			
			// MLA: Carga de cuenta para ciclo de trabajo 50% - D = TB0CCR1/(TB0CCR0+1)
			TB0CCR0 = time_base/2;
			TB0CCR1 = time_base/2;
						
			// MLA: Habilitar interrupciones
			TB0CCTL1 = CCIE;
				
			// MLA: Indicamos al ADC que se va a utilizar la salida PWM del registro CCR1 del TB0
	        ADC12_A_init (ADC12_A_BASE,ADC12_A_SAMPLEHOLDSOURCE_1, ADC12_A_CLOCKSOURCE_ADC12OSC, ADC12_A_CLOCKDIVIDER_1);
	        ADC12_A_enable(ADC12_A_BASE);
		    ADC12_A_setupSamplingTimer (ADC12_A_BASE , ADC12_A_CYCLEHOLD_1024_CYCLES, ADC12_A_CYCLEHOLD_1024_CYCLES, ADC12_A_MULTIPLESAMPLESENABLE);
			
/* 						// MLA: para programar el timer en modo UP con fuente de reloj ACLK, sin división y con el periodo establecido
			const unsigned short usACLK_Frequency_Hz = 32768;

			// MLA: Asegurar timer está apagado 
			TB0CTL = 0;

			// MLA: Fuente de reloj ACLK
			TBCTL = TBSSEL_1;

			// MLA: clear para comenzar a comparar
			TBCTL |= TBCLR;

			//Valor a comparar acorde al tick del sistema y ciclo de trabajo 50% - D = TB0CCR1/(TB0CCR0+1)
			TB0CCR0 = usACLK_Frequency_Hz / configTICK_RATE_HZ;
			TB0CCR1 = usACLK_Frequency_Hz / configTICK_RATE_HZ;
			//TB0CCTL1 = usACLK_Frequency_Hz / configTICK_RATE_HZ;

			// MLA: Habilitar interrupciones
			TB0CCTL0 = CCIE;
			
			// MLA: Modo PWM - reset/set
			TB0CCTL0 |= (OUTMOD0 | OUTMOD1 OUTMOD2);

			// MLA: Inicio de operación del timer
			TB0CTL |= TBCLR;

			// MLA: Modo UP para generar CCIFG de TB0CCTL1 cuando salte
			TB0CTL |= MC_1;
			
			// Aqui hay que indicar al ADC que se va a utilizar la salida PWM del registro CCR1 del TB0
	        ADC12_A_init (ADC12_A_BASE,ADC12_A_SAMPLEHOLDSOURCE_3, ADC12_A_CLOCKSOURCE_ADC12OSC, ADC12_A_CLOCKDIVIDER_1);
	        ADC12_A_enable(ADC12_A_BASE);
	        ADC12_A_setupSamplingTimer (); */

	        break;
		}
}


/**
 * Arranca la conversión dependiendo de la fuente de disparo seleccionada.
 * Habilita la interrupción del último elemento de la secuencia
 **/

void AnalogStart(uint16_t period, uint8_t mode  )
{

    switch (mode){
    //Disparo SW
    case 0:{

        //Arranca la conversion
         ADC12_A_startConversion(ADC12_A_BASE,ADC12_A_MEMORY_0,ADC12_A_SEQOFCHANNELS);

         //Habilita la interrupción del último elemento de la secuencia
         ADC12_A_clearInterrupt(ADC12_A_BASE,
                                   ADC12IFG5);
         ADC12_A_enableInterrupt(ADC12_A_BASE,
                                    ADC12IE5);
        break;
    }
    //Disparo Timer
    case 1:{
        //Arranca la conversion - Para que en modo timer se esté leyendo de forma repetitiva habrá que cambiar el parámetro de la función ADC12_A_startConversion() en el que solo se tomaba una sola secuencia para el modo sw.
         ADC12_A_startConversion(ADC12_A_BASE,ADC12_A_MEMORY_0,ADC12_A_SEQOFCHANNELS);

         //Habilita la interrupción del último elemento de la secuencia
         ADC12_A_clearInterrupt(ADC12_A_BASE,
                                   ADC12IFG5);
         ADC12_A_enableInterrupt(ADC12_A_BASE,
                                    ADC12IE5);

        break;
    }

    }




}


/**
 * Para la conversion (detiene el Timer B y deshabilita la interrupcion)
 */
void AnalogStop(void)
{
	Timer_B_stop(TIMER_B0_BASE);
	ADC12_A_disableConversions(ADC12_A_BASE,true);
	ADC12_A_disableInterrupt(ADC12_A_BASE,ADC12IE5);
}

/**
 * Espera de forma bloqueante la conversion y lee el dato
 * @param ptrtodato: dirección de memoria donde queremos guardar el dato (de tipo uint16_t). Se puede pasar una variable de dicho tipo por referencia
 * @param ticks: máximo numero de ticks del sistema a esperar (portMAX_DELAY para indefinido).
 **/
inline BaseType_t AnalogRead(conversionData *ptrtodato, TickType_t ticks)
{
	return xQueueReceive(adcqueue,ptrtodato,ticks);
}


/**
 * Calcula la temperatura en función de los coeficientes de calibración
 * del sensor de temperatura almacenados en la flash
 * @param lectura: El dato "en crudo" que hemos leido del conversor
 * @return temperatura en grados centígrados tras la caliabración
 * Calculos realizados para la referencia por defecto 2.0V
 */

uint16_t AnalogTempCompensate(uint16_t lectura, uint8_t Vref)
{

	// segun el manual
	float temporal;

	// MLA: Creamos dos variable auxiliares para los parámetros de calibración segun la referencia seleccionada
	uint16_t adc_30_temp, adc_85_temp;

	// MLA: Escogemos parámetro de calibración según referencia pasada como parámetro
	switch (Vref)
	{
	case 0:
	    adc_30_temp = adccal->adc_ref15_30_temp;
	    adc_85_temp = adccal->adc_ref15_85_temp;
	    break;
    case 1:
        adc_30_temp = adccal->adc_ref20_30_temp;
        adc_85_temp = adccal->adc_ref20_85_temp;
        break;
    case 2:
        adc_30_temp = adccal->adc_ref25_30_temp;
        adc_85_temp = adccal->adc_ref25_85_temp;
        break;
	}

	//MLA: Hacemos máscara al valor de temperatura leido
	temporal=(((float)(lectura&0xFFF)- adc_30_temp)*(85.0-30.0));
	temporal=temporal/(adc_85_temp - adc_30_temp);
	temporal+=30.0;

	return ((uint16_t)temporal);
}


/**
 * Corrige la medida del ADC utilizando los coeficientes de calibración
 * grabados en flash para la referencia el ADC.
 * @param lectura: El dato "en crudo" que hemos leido del conversor
 * @return valor corregido (multiplicado por 16 --> los 12 bits mas significativos son la medida y los 4 últimos, la parte decimal).
 */
uint16_t AnalogValueCompensate(uint16_t lectura, uint8_t Vref)
{
    // MLA: Creamos dos variable auxiliares para los parámetros de calibración segun la referencia seleccionada
	uint16_t cal_adc_factor;
    int32_t temporal;

    // MLA: Escogemos parámetro de offset para calibración según referencia pasada como parámetro
    switch (Vref)
    {
    case 0:
        cal_adc_factor = refcal->ref_ref15;
        break;
    case 1:
        cal_adc_factor = refcal->ref_ref20;
        break;
    case 2:
        cal_adc_factor = refcal->ref_ref25;
        break;
    }

	//ADC(calibrated) = ( (ADC(raw) x CAL_ADC15VREF_FACTOR / 2^15) x (CAL_ADC_GAIN_FACTOR / 2^15) ) + CAL_ADC_OFFSET
	//MLA: Se calcula la conversión calibrada según los parámetros
	temporal = (lectura&0xFFF)*0x0002;        //FGA: Se multiplica por 2
	temporal *= cal_adc_factor;               //FGA: Factor corrector
	temporal >>= 16;                          //FGA: Dividir por 2^16
	temporal *= 0x0002;                       //FGA: Se multiplica por 2
	temporal *= adccal->adc_gain_factor;      //MLA: Ganancia
	temporal >>= 16;                          //MLA: Dividir por 2^16
	temporal += adccal->adc_offset;           //MLA: Offset
	return ((uint16_t) temporal);
}



/** Rutina de interrupción del ADC
 * Su ejecución se programa cuando se produce la
 * conversión del último elemento de la secuencia
 * */

#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=ADC12_VECTOR
__interrupt
#elif defined(__GNUC__)
__attribute__((interrupt(ADC12_VECTOR)))
#endif
void ADC12ISR(void)
{
	BaseType_t xHigherPriorityTaskWoken=pdFALSE; //debe ponerse a false

//    uint16_t RawTemperature;
//    uint16_t RawValueA0; //UMA:
	conversionData values;

    switch(__even_in_range(ADC12IV,34))
    {
    case  0: break;       //Vector  0:  No interrupt
    case  2: break;       //Vector  2:  ADC overflow
    case  4: break;       //Vector  4:  ADC timing overflow
    case  6: break;       //Vector  6:  ADC12IFG0
    case  8: break;		  //Vector  8:  ADC12IFG1
    case 10: break;       //Vector 10:  ADC12IFG2
    case 12: break;       //Vector 12:  ADC12IFG3
    case 14: break;       //Vector 14:  ADC12IFG4
    case 16:        //Vector 16:  ADC12IFG5
        values.Chan0Ref=ADC12_A_getResults(ADC12_A_BASE,ADC12_A_MEMORY_0);
        values.Chan0Vcc=ADC12_A_getResults(ADC12_A_BASE,ADC12_A_MEMORY_1);
        values.BattRef=ADC12_A_getResults(ADC12_A_BASE,ADC12_A_MEMORY_2);
        values.BattVcc=ADC12_A_getResults(ADC12_A_BASE,ADC12_A_MEMORY_3);
        values.TempRef=ADC12_A_getResults(ADC12_A_BASE,ADC12_A_MEMORY_4);
        values.TempVcc=ADC12_A_getResults(ADC12_A_BASE,ADC12_A_MEMORY_5);
        __no_operation(); //para poner punto de ruptura.
        xQueueSendFromISR(adcqueue,&values,&xHigherPriorityTaskWoken); //UMA
        break;
    case 18: break;       //Vector 18:  ADC12IFG6
    case 20: break;       //Vector 20:  ADC12IFG7
    case 22: break;       //Vector 22:  ADC12IFG8
    case 24: break;       //Vector 24:  ADC12IFG9
    case 26: break;       //Vector 26:  ADC12IFG10
    case 28: break;       //Vector 28:  ADC12IFG11
    case 30: break;       //Vector 30:  ADC12IFG12
    case 32: break;       //Vector 32:  ADC12IFG13
    case 34: break;       //Vector 34:  ADC12IFG14
    default: break;
    }

    portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
}

#endif /* ANALOG_C_ */
