//*****************************************************************************
//
// Fichero con la definicion de comandos del interprete
//
//
//
//*****************************************************************************

#include <stdbool.h>
#include <stdint.h>
//#include <stdlib.h>
//#include <string.h>
//#include <stdio.h>
#include <assert.h>
#include <MSP430F5xx_6xx/driverlib.h>

/* FreeRTOS includes */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

#include "utils/cmdline.h"
#include "utils/ustdlib.h"

#include "serial.h"

#include "msp430.h"
#include "hal.h"



// ==============================================================================
// Implementa el comando free, que muestra cuánta memoria "heap" le queda al FreeRTOS
// ==============================================================================
int Cmd_free(int argc, char *argv[])
{
    //
    // Print some header text.
    //
	xSerialPrintf(0,"%d bytes libres\r\n", xPortGetFreeHeapSize());

    // Return success.
    return(0);
}

// ==============================================================================
// Implementa el comando task. Sólo es posible si la opción configUSE_TRACE_FACILITY de FreeRTOS está habilitada
// ==============================================================================
#if ( configUSE_TRACE_FACILITY == 1 )

extern char *__stack;
int Cmd_tasks(int argc, char *argv[])
{
	char*	pBuffer;
	unsigned char*	pStack;
	portBASE_TYPE	x;
	
	pBuffer = pvPortMalloc(1024);
	if (pBuffer)
	{
		vTaskList(pBuffer);
		xSerialPrintf(0,"\t\t\t\tUnused\r\nTaskName\tStatus\tPri\tStack\tTask ID\r\n");
		xSerialPrintf(0,"=======================================================\r\n");
		xSerialPrintf(0,"%s", pBuffer);
		vPortFree(pBuffer);
	}
	return 0;
}

#endif /* configUSE_TRACE_FACILITY */

// ==============================================================================
// Implementa el comando help
// ==============================================================================
int Cmd_help(int argc, char *argv[])
{
    tCmdLineEntry *pEntry;

    //
    // Print some header text.
    //
    xSerialPrintf(0,"Comandos disponibles\r\n");
    xSerialPrintf(0,"------------------\r\n");

    //
    // Point at the beginning of the command table.
    //
    pEntry = &g_psCmdTable[0];

    //
    // Enter a loop to read each entry from the command table.  The end of the
    // table has been reached when the command name is NULL.
    //
    while(pEntry->pcCmd)
    {
        //
        // Print the command name and the brief description.
        //
    	xSerialPrintf(0,"%s%s\r\n", pEntry->pcCmd, pEntry->pcHelp);

        //
        // Advance to the next entry in the table.
        //
        pEntry++;
    }

    //
    // Return success.
    //
    return(0);
}

// ==============================================================================
// Implementa el comando "LED"
// ==============================================================================
int Cmd_led(int argc, char *argv[])
{
	if (argc == 1)
	{
		//Si los parámetros no son suficientes, muestro la ayuda
		xSerialPrintf(0," LED [on|off]\r\n");
	}
	else
	{
		/* chequeo el parametro */
		if (0==ustrncmp( argv[1], "on",2))
		{
			xSerialPrintf(0,"Enciendo el LED\r\n");
			GPIO_setOutputHighOnPin(LED1_PORT,LED1_PIN);
		}
		else if (0==ustrncmp( argv[1], "off",3))
		{
			xSerialPrintf(0,"Apago el LED\r\n");
			GPIO_setOutputLowOnPin(LED1_PORT,LED1_PIN);
		}
		else
		{
			//Si el parámetro no es correcto, muestro la ayuda
			xSerialPrintf(0," LED [on|off]\r\n");
		}

	}
    return 0;
}

// ==============================================================================
// Implementa el comando "Blink" (G35: completar!!!)
// ==============================================================================
int Cmd_blink(int argc, char *argv[])
{
	uint16_t veces;
	uint16_t periodo;

	if (argc != 3)
	{
		//Si los parámetros no son suficientes, muestro la ayuda
		xSerialPrintf(0," blink [veces] [periodo]\r\n");
	}
	else
	{
		//usar mejor las funciones de la biblioteca utils/ustdlib en lugar de las de stdlib
		//algunas de estas últimas hemos visto que fallan sin venir a cuento...

		veces=ustrtoul(argv[1],NULL,10);
		periodo=ustrtoul(argv[2],NULL,10);

		if ((veces<=0)||(periodo<=0))
		{
			//Si el parámetro no es correcto, muestro la ayuda
			xSerialPrintf(0," blink  [on|off]\r\n");
		}
		else
		{
			//G3.3 Hacer algo con los parametros, ahora mismo lo que hago es imprimir....
			xSerialPrintf(0," %d %d \r\n",(uint32_t)veces,(uint32_t)periodo);
		}

	}

	return 0;
}


#if configGENERATE_RUN_TIME_STATS
// ==============================================================================
// Implementa el comando "Stats"
// ==============================================================================
Cmd_stats(int argc, char *argv[])
{
	char*	pBuffer;
	unsigned char*	pStack;
	portBASE_TYPE	x;

	pBuffer = pvPortMalloc(1024);
	if (pBuffer)
	{
		vTaskGetRunTimeStats(pBuffer);
		xSerialPrintf(0,"TaskName\tCycles\t\tPercent\r\n");
		xSerialPrintf(0,"===============================================\r\n");
		xSerialPrintf(0,"%s", pBuffer);
		vPortFree(pBuffer);
	}
	return 0;
}
#endif


// ==============================================================================
// Tabla con los comandos y su descripción. Si quiero añadir alguno, debo hacerlo aquí
// ==============================================================================
const tCmdLineEntry g_psCmdTable[] =
{
    { "help",     Cmd_help,      "     : Lista de comandos" },
    { "?",        Cmd_help,      "        : lo mismo que help" },
    { "led",  	  Cmd_led,       "  : Apaga y enciende el led rojo" }, /*descomentar G3.2*/
    { "free",     Cmd_free,      "     : Muestra la memoria libre" },
#if ( configUSE_TRACE_FACILITY == 1 )
	{ "tasks",    Cmd_tasks,     "    : Muestra informacion de traza de las tareas" },
#endif
#if (configGENERATE_RUN_TIME_STATS)
	{ "stats",    Cmd_stats,      "     : Muestra las estadisticas " },
#endif
    { 0, 0, 0 }
};

// ==============================================================================
// Tarea UARTTask.  Espera la llegada de comandos por el puerto serie y los ejecuta al recibirlos...
// ==============================================================================

void vUARTTask( void *pvParameters )
{
    char    cCmdBuf[64];
    int     nStatus;
	

	xSerialPrintf(0,"\r\n\r\n FreeRTOS %s \r\n",
		tskKERNEL_VERSION_NUMBER);
	xSerialPrintf(0,"\r\n Teclee ? para ver la ayuda \r\n");
	xSerialPrintf(0,"> ");

	/* Loop forever */
    while (1)
    {
		/* Wait for a signal indicating we have an RX line to process */
 		if (xSerialGetStringWithHistory( 0, cCmdBuf, sizeof(cCmdBuf))>0)
 		//if (xSerialGetString( 0, cCmdBuf, sizeof(cCmdBuf))>0)
 		{
	        //
	        // Pass the line from the user to the command processor.  It will be
	        // parsed and valid commands executed.
	        //
	        nStatus = CmdLineProcess(cCmdBuf);
	
	        //
	        // Handle the case of bad command.
	        //
	        if(nStatus == CMDLINE_BAD_CMD)
	        {
	            xSerialPrintf(0,"Comando erroneo!\r\n");	//No pongo acentos adrede
	        }
	
	        //
	        // Handle the case of too many arguments.
	        //
	        else if(nStatus == CMDLINE_TOO_MANY_ARGS)
	        {
	        	xSerialPrintf(0,"El interprete de comandos no admite tantos parametros\r\n");	//El máximo, CMDLINE_MAX_ARGS, está definido en cmdline.c
	        }
	
	        //
	        // Otherwise the command was executed.  Print the error code if one was
	        // returned.
	        //
	        else if(nStatus != 0)
	        {
	        	xSerialPrintf(0,"El comando devolvio el error %d\r\n",nStatus);
	        }
	
	        xSerialPrintf(0,"> ");
	    }
    }
}
