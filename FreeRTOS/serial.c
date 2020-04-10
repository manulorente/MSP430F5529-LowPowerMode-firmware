/*
    FreeRTOS V8.0.1 - Copyright (C) 2014 Real Time Engineers Ltd. 
    All rights reserved

    VISIT http://www.FreeRTOS.org TO ENSURE YOU ARE USING THE LATEST VERSION.

 ***************************************************************************
 *                                                                       *
 *    FreeRTOS provides completely free yet professionally developed,    *
 *    robust, strictly quality controlled, supported, and cross          *
 *    platform software that has become a de facto standard.             *
 *                                                                       *
 *    Help yourself get started quickly and support the FreeRTOS         *
 *    project by purchasing a FreeRTOS tutorial book, reference          *
 *    manual, or both from: http://www.FreeRTOS.org/Documentation        *
 *                                                                       *
 *    Thank you!                                                         *
 *                                                                       *
 ***************************************************************************

    This file is part of the FreeRTOS distribution.

    FreeRTOS is free software; you can redistribute it and/or modify it under
    the terms of the GNU General Public License (version 2) as published by the
    Free Software Foundation >>!AND MODIFIED BY!<< the FreeRTOS exception.

    >>!   NOTE: The modification to the GPL is included to allow you to     !<<
    >>!   distribute a combined work that includes FreeRTOS without being   !<<
    >>!   obliged to provide the source code for proprietary components     !<<
    >>!   outside of the FreeRTOS kernel.                                   !<<

    FreeRTOS is distributed in the hope that it will be useful, but WITHOUT ANY
    WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
    FOR A PARTICULAR PURPOSE.  Full license text is available from the following
    link: http://www.freertos.org/a00114.html

    1 tab == 4 spaces!

 ***************************************************************************
 *                                                                       *
 *    Having a problem?  Start by reading the FAQ "My application does   *
 *    not run, what could be wrong?"                                     *
 *                                                                       *
 *    http://www.FreeRTOS.org/FAQHelp.html                               *
 *                                                                       *
 ***************************************************************************

    http://www.FreeRTOS.org - Documentation, books, training, latest versions,
    license and Real Time Engineers Ltd. contact details.

    http://www.FreeRTOS.org/plus - A selection of FreeRTOS ecosystem products,
    including FreeRTOS+Trace - an indispensable productivity tool, a DOS
    compatible FAT file system, and our tiny thread aware UDP/IP stack.

    http://www.OpenRTOS.com - Real Time Engineers ltd license FreeRTOS to High
    Integrity Systems to sell under the OpenRTOS brand.  Low cost OpenRTOS
    licenses offer ticketed support, indemnification and middleware.

    http://www.SafeRTOS.com - High Integrity Systems also provide a safety
    engineered and independently SIL3 certified version for use in safety and
    mission critical applications that require provable dependability.

    1 tab == 4 spaces!
 */


/* BASIC INTERRUPT DRIVEN SERIAL PORT DRIVER.
 *
 * This is not a proper UART driver.  It only supports one port, uses loopback
 * mode, and is used to test interrupts that use the FreeRTOS API as part of 
 * a wider test suite.  Nor is it intended to show an efficient implementation
 * of a UART interrupt service routine as queues are used to pass individual
 * characters one at a time!
 */

/* Standard includes. */
#include <MSP430F5xx_6xx/driverlib.h>
#include <stdarg.h>
#include <stdlib.h>

#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"

/* Demo application includes. */
#include "serial.h"



/* Misc. constants. */
#define serNO_BLOCK				( ( TickType_t ) 0 )

/* The queue used to hold received characters. */
static QueueHandle_t xRxedChars;

/* The queue used to hold characters waiting transmission. */
static QueueHandle_t xCharsForTx;

/*-----------------------------------------------------------*/

xComPortHandle xSerialPortInitMinimal( unsigned long ulWantedBaud, unsigned portBASE_TYPE uxQueueLength )
{
	unsigned long ulBaudRateCount;
	volatile unsigned long modulator;

	/* Initialise the hardware. */

	portENTER_CRITICAL();
	{
		/* Create the queues used by the com test task. */
		xRxedChars = xQueueCreate( uxQueueLength, ( unsigned portBASE_TYPE ) sizeof( signed char ) );
		xCharsForTx = xQueueCreate( uxQueueLength, ( unsigned portBASE_TYPE ) sizeof( signed char ) );

		//Viausá la nueva API, que pasotá-
		GPIO_setAsPeripheralModuleFunctionInputPin( GPIO_PORT_P4, GPIO_PIN5|GPIO_PIN4);	//Aunque uno sea salida, vale asín

		USCI_A_UART_initParam param = {0};

#ifdef UART_SMCLK

		/* No funciona del todo OK si el FLL no esta activo --> no usar en LMP1 o mayor.
		/* Generate the baud rate constants for the wanted baud rate. */
		ulBaudRateCount = (configCPU_CLOCK_HZ ) / ulWantedBaud;
		modulator=(((configCPU_CLOCK_HZ - ulBaudRateCount * ulWantedBaud)*8)/ulWantedBaud);



		param.selectClockSource = USCI_A_UART_CLOCKSOURCE_SMCLK;
		param.clockPrescalar = ulBaudRateCount;
		param.firstModReg = 0;
		param.secondModReg = modulator;


#else
		/* solo para baudrate chicos */
		if (ulWantedBaud>9600)
		{
			while(1);
		}
		ulBaudRateCount = (configLFXT_CLOCK_HZ ) / ulWantedBaud;
		modulator=(((configLFXT_CLOCK_HZ - ulBaudRateCount * ulWantedBaud)*8)/ulWantedBaud);
		param.selectClockSource = USCI_A_UART_CLOCKSOURCE_ACLK;
		param.clockPrescalar = ulBaudRateCount;
		param.firstModReg = 0;
		param.secondModReg = modulator;
#endif
		param.parity = USCI_A_UART_NO_PARITY;
		param.msborLsbFirst = USCI_A_UART_LSB_FIRST;
		param.numberofStopBits = USCI_A_UART_ONE_STOP_BIT;
		param.uartMode = USCI_A_UART_MODE;
		param.overSampling = USCI_A_UART_LOW_FREQUENCY_BAUDRATE_GENERATION;

		if(STATUS_FAIL == USCI_A_UART_init(USCI_A1_BASE, &param))
		{
			while(1);	//Error
		}

		//Enable UART module for operation
		USCI_A_UART_enable(USCI_A1_BASE);

		//Enable Receive Interrupt
		USCI_A_UART_clearInterrupt(USCI_A1_BASE,
				USCI_A_UART_RECEIVE_INTERRUPT);
		USCI_A_UART_enableInterrupt(USCI_A1_BASE,
				USCI_A_UART_RECEIVE_INTERRUPT);
	}
	portEXIT_CRITICAL();

	/* Note the comments at the top of this file about this not being a generic
	UART driver. */
	return NULL;
}
/*-----------------------------------------------------------*/

signed portBASE_TYPE xSerialGetChar( xComPortHandle pxPort, signed char *pcRxedChar, TickType_t xBlockTime )
{
	/* Get the next character from the buffer.  Return false if no characters
	are available, or arrive before xBlockTime expires. */
	return xQueueReceive( xRxedChars, pcRxedChar, xBlockTime );
}

/*-----------------------------------------------------------*/

signed portBASE_TYPE xSerialPutChar( xComPortHandle pxPort, signed char cOutChar, TickType_t xBlockTime )
{
	signed portBASE_TYPE xReturn;

	/* Send the next character to the queue of characters waiting transmission,
	then enable the UART Tx interrupt, just in case UART transmission has already
	completed and switched itself off. */
	xReturn = xQueueSend( xCharsForTx, &cOutChar, xBlockTime );
	UCA1IE |= UCTXIE;

	return xReturn;
}
/*-----------------------------------------------------------*/

int xSerialRead( xComPortHandle pxPort, char *buffer, int  buffersize )
{
	char * ptrtochar=buffer;
	int i;

	for(i=0;i<buffersize;i++)
	{
		xQueueReceive( xRxedChars, ptrtochar++, portMAX_DELAY );
	}
	return i;
}

int xSerialWrite( xComPortHandle pxPort, char *buffer, int size)
{
	char * ptrtochar=buffer;
	int i;

	i=0;
	for(i=0;i<size;i++)
	{
		xSerialPutChar(0,*ptrtochar++,portMAX_DELAY);
	}
	return i;
}



int xSerialGetString( xComPortHandle pxPort, char *buffer, int  buffersize )
{
	char * ptrtochar=buffer;
	int i;
	/* Get the next character from the buffer.  Return false if no characters
	are available, or arrive before xBlockTime expires. */

	i=0;
	while (i<buffersize)
	{
		xQueueReceive( xRxedChars, ptrtochar, portMAX_DELAY );

#ifndef DISABLE_ECHO
		xSerialPutChar(0,*ptrtochar,portMAX_DELAY);
#endif
		if ('\r'==*ptrtochar)
		{
			*ptrtochar=0;
#ifndef DISABLE_ECHO
			xSerialPutChar(0,'\n',portMAX_DELAY); //Parece que el putty manda /r en lugar de /n con el retonno de carro
#endif
			return i;
		}
		else if ('\n'==*ptrtochar)
		{
			continue;
		}
		else if ('\b'==*ptrtochar)
		{
#ifndef DISABLE_ECHO
			xSerialWrite(pxPort," \b",2);
#endif
			if (i>0)
			{
				i--;
				ptrtochar--;
			}
		}
		else
		{
			i++;
			ptrtochar++;
		}
	}
	return -1; //error, he llegado al final sin recibir el enter...
}

int xSerialPutString( xComPortHandle pxPort, char *buffer)
{
	char * ptrtochar=buffer;
	int i;
	/* Get the next character from the buffer.  Return false if no characters
	are available, or arrive before xBlockTime expires. */

	i=0;
	while (*ptrtochar!=0)
	{
		if (*ptrtochar=='\n')
		{
			xSerialPutChar(0,'\r',portMAX_DELAY);
		}
		xSerialPutChar(0,*ptrtochar++,portMAX_DELAY);
		i++;
	}
	return i;
}




#ifdef WANT_CMDLINE_HISTORY
// Extended History support variables
static char g_pcUARTRxHistoryBuffer[UART_RX_HISTORY_BUF_SIZE];
static unsigned short g_usUARTRxHistoryOffset[UART_RX_HISTORY_DEPTH];
static unsigned short g_usUARTRxHistoryBufIndex = 0;
static signed char g_bUARTRxHistoryNext = 0;
static signed char g_bUARTRxHistoryCnt = 0;

//JOSE MODIFICAR JMCG!!!!


int xSerialGetStringWithHistory( xComPortHandle pxPort, char *buffer, int  buffersize )
{

	char * ptrtochar=buffer;
	short count,i;
	short bUARTRxHistoryCur = -1;
	unsigned char ubEscMode = 0;
	char duplicateCmd=0;


	/* Get the next character from the buffer.  Return false if no characters
	are available, or arrive before xBlockTime expires. */

	i=0;
	while (i<buffersize)
	{
		xQueueReceive( xRxedChars, ptrtochar, portMAX_DELAY );


		//
		// Test for ESC sequence
		//
		if (*ptrtochar == 0x1b)
		{
			ubEscMode = 1;
			continue;
		}


		if (ubEscMode != 0)
		{
			// Test if this is the first byte after the ESCape
			if (ubEscMode == 1)
			{
				// Test the known escape codes
				if (*ptrtochar == '[')
				{
					ubEscMode = 2;
					continue;
				}
				else
					// Unknown code
				{
					ubEscMode = 0;
					continue;
				}
			}
			else
			{
				// Test for UP or DOWN arrow
				if ((*ptrtochar == 'A') || (*ptrtochar == 'B'))
				{
					unsigned short copyFrom;
					int current = bUARTRxHistoryCur;

					ubEscMode = 0;

					if (*ptrtochar == 'A')
					{
						// Test if this is the first history request
						if (current == -1)
							current = g_bUARTRxHistoryNext;

						// Now decrement to previous history
						if (--current < 0)
							current = UART_RX_HISTORY_DEPTH-1;

						// Get previous history into Rx buffer
						if ((g_bUARTRxHistoryNext == bUARTRxHistoryCur) ||
								(current > g_bUARTRxHistoryCnt))
						{
							// At end of history.  Just return
							xSerialWrite(pxPort,"\x07", 1);
							continue;
						}
					}
					else
					{
						// Test if this is the first history request
						if (current == -1)
							continue;

						// Now increment to next history
						if (++current >= UART_RX_HISTORY_DEPTH)
							current = 0;

						// Test if we were on the last history item
						if (current == g_bUARTRxHistoryNext)
						{
							xSerialWrite(pxPort,"\r\x1b[2K> ", 7);
							bUARTRxHistoryCur = -1;
							continue;
						}
					}
					bUARTRxHistoryCur = current;

					// Copy the history item to the Rx Buffer
					xSerialWrite(pxPort,"\r\x1b[2K> ", 7);
					ptrtochar=buffer;
					copyFrom = g_usUARTRxHistoryOffset[current];
					for (i = 0; (g_pcUARTRxHistoryBuffer[copyFrom] != '\0')&&(i<buffersize); i++)
					{
						*ptrtochar = g_pcUARTRxHistoryBuffer[copyFrom++];
						xSerialPutChar(pxPort,*ptrtochar, portMAX_DELAY);
						ptrtochar++;
						if (copyFrom >= UART_RX_HISTORY_BUF_SIZE)
							copyFrom = 0;
					}
					*ptrtochar = '\0';

					continue;
				}
				// Test for LEFT arrow
				else if (*ptrtochar == 'C')
				{
					ubEscMode = 0;
					continue;
				}
				// Test for RIGHT arrow
				else if (*ptrtochar == 'D')
				{
					ubEscMode = 0;
					continue;
				}
				else
					// Unknown sequence
				{
					ubEscMode = 0;
					continue;
				}
			}
		}
//EN todos los casos anteriores creo que no debe haber eco...
#ifndef DISABLE_ECHO
		xSerialPutChar(pxPort,*ptrtochar,portMAX_DELAY);
#endif
		if (('\n'==*ptrtochar))
		{
			continue; //Parece que el putty manda realmente /r en lugar de /n con el enter
		}
		else if ('\r'==*ptrtochar)
		{
			//
			// Add command to the command history
			//
			*ptrtochar=0;
#ifndef DISABLE_ECHO
			xSerialPutChar(pxPort,'\n',portMAX_DELAY);
#endif
			// Test if this command matches the last command
			duplicateCmd = false;
			if (g_bUARTRxHistoryCnt > 0)
			{
				int prev,y;

				prev = g_bUARTRxHistoryNext-1;
				if (prev < 0)
					prev = UART_RX_HISTORY_DEPTH-1;

				y = g_usUARTRxHistoryOffset[prev];

				for (count=0; count < i; count++)
				{
					if (g_pcUARTRxHistoryBuffer[y++] != buffer[count])
						break;
					if (y >= UART_RX_HISTORY_BUF_SIZE)
						y = 0;
				}

				// Test if the command matches the last
				if ((count == i) && (g_pcUARTRxHistoryBuffer[y] == 0))
					duplicateCmd = true;
			}

			// Now save the command in the history buffer
			if (!duplicateCmd)
			{
				g_usUARTRxHistoryOffset[g_bUARTRxHistoryNext++] = g_usUARTRxHistoryBufIndex;
				if (g_bUARTRxHistoryNext == UART_RX_HISTORY_DEPTH)
					g_bUARTRxHistoryNext = 0;
				if (g_bUARTRxHistoryCnt != UART_RX_HISTORY_DEPTH)
					g_bUARTRxHistoryCnt++;

				for (count=0; count <=i ; count++)
				{
					g_pcUARTRxHistoryBuffer[g_usUARTRxHistoryBufIndex++] =
							buffer[count];
					if (g_usUARTRxHistoryBufIndex >= UART_RX_HISTORY_BUF_SIZE)
						g_usUARTRxHistoryBufIndex = 0;
				}
			}

			bUARTRxHistoryCur = -1;

			return i;
		}

		else if ('\b'==*ptrtochar)
		{
#ifndef DISABLE_ECHO
			xSerialWrite(pxPort," \b",2);
#endif
			if (i>0)
			{
				i--;
				ptrtochar--;
			}
			continue;	/* no me gusta, pero aqui es indicado...*/
		}

		i++;
		ptrtochar++;
	}
	return -1; //error, he llegado al final sin recibir el enter...
}

#endif /* WANT_CMDLINE_HISTORY */


static const char * const g_pcHex = "0123456789abcdef";

int xSerialvPrintf( xComPortHandle pxPort,const char *pcString, va_list vaArgP)
{
	{
		uint32_t ui32Value,  ui32Idx, ui32Neg;
		uint16_t ui16Base, ui16Pos, ui16Count;
		char *pcStr, pcBuf[16], cFill;

		//
		// Check the arguments.
		//
		//ASSERT(pcString != 0); ¿falta el include de assert.h?

		//
		// Loop while there are more characters in the string.
		//
		while(*pcString)
		{
			//
			// Find the first non-% character, or the end of the string.
			//
			for(ui32Idx = 0;
					(pcString[ui32Idx] != '%') && (pcString[ui32Idx] != '\0');
					ui32Idx++)
			{
			}

			//
			// Write this portion of the string.
			//
			xSerialWrite(pxPort,pcString, ui32Idx);

			//
			// Skip the portion of the string that was written.
			//
			pcString += ui32Idx;

			//
			// See if the next character is a %.
			//
			if(*pcString == '%')
			{
				//
				// Skip the %.
				//
				pcString++;

				//
				// Set the digit count to zero, and the fill character to space
				// (in other words, to the defaults).
				//
				ui16Count = 0;
				cFill = ' ';

				//
				// It may be necessary to get back here to process more characters.
				// Goto's aren't pretty, but effective.  I feel extremely dirty for
				// using not one but two of the beasts.
				//
				again:

				//
				// Determine how to handle the next character.
				//
				switch(*pcString++)
				{
				//
				// Handle the digit characters.
				//
				case '0':
				case '1':
				case '2':
				case '3':
				case '4':
				case '5':
				case '6':
				case '7':
				case '8':
				case '9':
				{
					//
					// If this is a zero, and it is the first digit, then the
					// fill character is a zero instead of a space.
					//
					if((pcString[-1] == '0') && (ui16Count == 0))
					{
						cFill = '0';
					}

					//
					// Update the digit count.
					//
					ui16Count *= 10;
					ui16Count += pcString[-1] - '0';

					//
					// Get the next character.
					//
					goto again;
				}

				//
				// Handle the %c command.
				//
				case 'c':
				{
					//
					// Get the value from the varargs.
					//
					ui32Value = va_arg(vaArgP, uint32_t);

					//
					// Print out the character.
					//
					//UARTwrite((char *)&ui32Value, 1);
					xSerialPutChar(pxPort,ui32Value,portMAX_DELAY);
					//
					// This command has been handled.
					//
					break;
				}

				//
				// Handle the %d and %i commands.
				//
				case 'd':
				case 'i':
				{
					//
					// Get the value from the varargs.
					//
					ui32Value = va_arg(vaArgP, uint32_t);

					//
					// Reset the buffer position.
					//
					ui16Pos = 0;

					//
					// If the value is negative, make it positive and indicate
					// that a minus sign is needed.
					//
					if((int32_t)ui32Value < 0)
					{
						//
						// Make the value positive.
						//
						ui32Value = -(int32_t)ui32Value;

						//
						// Indicate that the value is negative.
						//
						ui32Neg = 1;
					}
					else
					{
						//
						// Indicate that the value is positive so that a minus
						// sign isn't inserted.
						//
						ui32Neg = 0;
					}

					//
					// Set the base to 10.
					//
					ui16Base = 10;

					//
					// Convert the value to ASCII.
					//
					goto convert;
				}

				//
				// Handle the %s command.
				//
				case 's':
				{
					//
					// Get the string pointer from the varargs.
					//
					pcStr = va_arg(vaArgP, char *);

					//
					// Determine the length of the string.
					//
					for(ui32Idx = 0; pcStr[ui32Idx] != '\0'; ui32Idx++)
					{
					}

					//
					// Write the string.
					//
					xSerialWrite(pxPort,pcStr, ui32Idx);

					//
					// Write any required padding spaces
					//
					if(ui16Count > ui32Idx)
					{
						ui16Count -= ui32Idx;
						while(ui16Count--)
						{
							//UARTwrite(" ", 1);
							xSerialPutChar(pxPort,' ',portMAX_DELAY);
						}
					}

					//
					// This command has been handled.
					//
					break;
				}

				//
				// Handle the %u command.
				//
				case 'u':
				{
					//
					// Get the value from the varargs.
					//
					ui32Value = va_arg(vaArgP, uint32_t);

					//
					// Reset the buffer position.
					//
					ui16Pos = 0;

					//
					// Set the base to 10.
					//
					ui16Base = 10;

					//
					// Indicate that the value is positive so that a minus sign
					// isn't inserted.
					//
					ui32Neg = 0;

					//
					// Convert the value to ASCII.
					//
					goto convert;
				}

				//
				// Handle the %x and %X commands.  Note that they are treated
				// identically; in other words, %X will use lower case letters
				// for a-f instead of the upper case letters it should use.  We
				// also alias %p to %x.
				//
				case 'x':
				case 'X':
				case 'p':
				{
					//
					// Get the value from the varargs.
					//
					ui32Value = va_arg(vaArgP, uint32_t);

					//
					// Reset the buffer position.
					//
					ui16Pos = 0;

					//
					// Set the base to 16.
					//
					ui16Base = 16;

					//
					// Indicate that the value is positive so that a minus sign
					// isn't inserted.
					//
					ui32Neg = 0;

					//
					// Determine the number of digits in the string version of
					// the value.
					//
					convert:
					for(ui32Idx = 1;
							(((ui32Idx * ui16Base) <= ui32Value) &&
									(((ui32Idx * ui16Base) / ui16Base) == ui32Idx));
							ui32Idx *= ui16Base, ui16Count--)
					{
					}

					//
					// If the value is negative, reduce the count of padding
					// characters needed.
					//
					if(ui32Neg)
					{
						ui16Count--;
					}

					//
					// If the value is negative and the value is padded with
					// zeros, then place the minus sign before the padding.
					//
					if(ui32Neg && (cFill == '0'))
					{
						//
						// Place the minus sign in the output buffer.
						//
						pcBuf[ui16Pos++] = '-';

						//
						// The minus sign has been placed, so turn off the
						// negative flag.
						//
						ui32Neg = 0;
					}

					//
					// Provide additional padding at the beginning of the
					// string conversion if needed.
					//
					if((ui16Count > 1) && (ui16Count < 16))
					{
						for(ui16Count--; ui16Count; ui16Count--)
						{
							pcBuf[ui16Pos++] = cFill;
						}
					}

					//
					// If the value is negative, then place the minus sign
					// before the number.
					//
					if(ui32Neg)
					{
						//
						// Place the minus sign in the output buffer.
						//
						pcBuf[ui16Pos++] = '-';
					}

					//
					// Convert the value into a string.
					//
					for(; ui32Idx; ui32Idx /= ui16Base)
					{
						pcBuf[ui16Pos++] =
								g_pcHex[(ui32Value / ui32Idx) % ui16Base];
					}

					//
					// Write the string.
					//
					xSerialWrite(pxPort,pcBuf, ui16Pos);

					//
					// This command has been handled.
					//
					break;
				}

				//
				// Handle the %% command.
				//
				case '%':
				{
					//
					// Simply write a single %.
					//
					xSerialWrite(pxPort,pcString - 1, 1);

					//
					// This command has been handled.
					//
					break;
				}

				//
				// Handle all other commands.
				//
				default:
				{
					//
					// Indicate an error.
					//
					xSerialWrite(pxPort,"ERROR", 5);

					//
					// This command has been handled.
					//
					break;
				}
				}
			}
		}
	}
}

int xSerialPrintf( xComPortHandle pxPort,const char *pcString,... )
{
	va_list vaArgP;

	va_start(vaArgP, pcString);
	xSerialvPrintf(pxPort,pcString,vaArgP);
	va_end(vaArgP);
}




/* The implementation of this interrupt is provided to demonstrate the use
of queues from inside an interrupt service routine.  It is *not* intended to
be an efficient interrupt implementation.  A real application should make use
of the DMA.  Or, as a minimum, transmission and reception could use a simple
RAM ring buffer, and synchronise with a task using a semaphore when a complete
message has been received or transmitted. */

void __attribute__ ((interrupt(USCI_A1_VECTOR))) prvUSCI_A1_ISR (void)
				{
	signed char cChar;
	portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;

	if( ( UCA1IFG & UCRXIFG ) != 0 )
	{
		/* Get the character from the UART and post it on the queue of Rxed
		characters. */
		cChar = UCA1RXBUF;
		xQueueSendFromISR( xRxedChars, &cChar, &xHigherPriorityTaskWoken );
	}	
	else if( ( UCA1IFG & UCTXIFG ) != 0 )
	{
		/* The previous character has been transmitted.  See if there are any
		further characters waiting transmission. */
		if( xQueueReceiveFromISR( xCharsForTx, &cChar, &xHigherPriorityTaskWoken ) == pdTRUE )
		{
			/* There was another character queued - transmit it now. */
			UCA1TXBUF = cChar;
		}
		else
		{
			/* There were no other characters to transmit - disable the Tx
			interrupt. */
			UCA1IE &= ~UCTXIE;
		}
	}


	__bic_SR_register_on_exit( SCG1 + SCG0 + OSCOFF + CPUOFF );

	/* If writing to a queue caused a task to unblock, and the unblocked task
	has a priority equal to or above the task that this interrupt interrupted,
	then lHigherPriorityTaskWoken will have been set to pdTRUE internally within
	xQueuesendFromISR(), and portEND_SWITCHING_ISR() will ensure that this
	interrupt returns directly to the higher priority unblocked task. 

	THIS MUST BE THE LAST THING DONE IN THE ISR. */	
	portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
}


