

//Este fichero contiene las definiciones necesarias para soportar el protocolo entre el microcontolador y el PC.

#ifndef __protocol_H__
#define __protocol_H__

#include<stdint.h>
#include "analog.h"
#include "MSP430F5xx_6xx/ref.h"
		
typedef enum {COMANDO_PING,COMANDO_LED,COMANDO_SONDEA_BOTONES,COMANDO_BOTONES_ASYNC,COMANDO_PWM_PERIODO,COMANDO_PWM_CICLO,COMANDO_RECHAZADO, COMANDO_ADC_START,COMANDO_ADC_STOP,COMANDO_ADC_DATA,COMANDO_ADC_TEMP,COMANDO_LEERCAL_ADC,COMANDO_LEERCAL_REF,COMANDO_SETREF} TipoComando;


		
#pragma pack(1)

typedef struct {
    uint8_t comando;
    union {
        uint16_t value;
        struct {
            uint16_t bit0:1;
            uint16_t bit1:1;
            uint16_t bit2:1;
            uint16_t bit3:1;
            uint16_t bit4:1;
            uint16_t bit5:1;
            uint16_t bit6:1;
            uint16_t bit7:1;
            uint16_t bit8:1;
            uint16_t bit9:1;
            uint16_t bit10:1;
            uint16_t bit11:1;
            uint16_t bit12:1;
            uint16_t bit13:1;
            uint16_t bit14:1;
            uint16_t bit15:1;
        } bits;
        struct {
            uint8_t mode;
            uint16_t ciclos;
            uint8_t index_ref;

        } adcconf;
        struct s_TLV_ADC_Cal_Data tlv_adc;
        struct s_TLV_REF_Cal_Data tlv_ref;
        conversionData analogData;
     } parametro;

} TipoPaquete;


#pragma pack()
		


#endif  /* ndef __protocol_H__ */
