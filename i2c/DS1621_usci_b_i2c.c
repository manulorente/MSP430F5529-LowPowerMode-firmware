// ******************************************************************************
// PROGRAMA :I2C_MSP430_DS1621.c (v1.1)
// TARGET   : MSP-EXP430F5529LP
// DRIVERLIB: 2_91_03_00 (03/11/2017)
// MPSWARE: 3_80_02_10 (03/11/2017)
// DESCRIPCION  : Lectura de temperatura del sensor I2C DS1621
// AUTOR    : EGP DTE
// FECHA    : 10-12-17
// ESQUEMA  : ACLK= REFO=32KHz, MCLK= SMCLK= DCODIVCLK=1048576Hz
// ******************************************************************************
//!
//!                MSP430F5529
//!             -----------------
//!         /|\|                 |
//!          | |                 |
//!          --|RST              |
//!            |                 |
//!            |     P3.0/SDA    |<------------>
//!            |                 |
//!            |     P3.1/SCL    |<------------>
//!
//!
//******************************************************************************

#include "driverlib.h"

#define SLAVE_ADDRESS_DS1621        0x48

// COMMANDS
#define K_DS1621_ACCES_CONFIG       0xAC
#define K_DS1621_READ_TEMP          0xAA
#define K_DS1621_START_CONVERT      0xEE
#define K_DS1621_READ_COUNTER       0xA8
#define K_DS1621_READ_SLOPE         0xA9

// OPERATION AND CONTROL
#define K_DS1621_POL_CONFIG         0x02
#define K_DS1621_1SHOT_CONFIG       0x01
#define K_DS1621_CONTINUOUS_CONFIG  0x00

// CONSTANTS

#define TEMP_CONVERSION_TIME        300000

//*****************************************************************************
//
//Global Variables
//
//*****************************************************************************

char ucRegData[2];
char Count_Remain, Count_Per_C;
float temp_DS1621, temp;


void main ()
{
    //Stop WDT
    WDT_A_hold(WDT_A_BASE);

    //Assign I2C pins to USCI_B0
    // P3.0 (SDA) y P3.1 (SCL)

    GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P3,
                                         GPIO_PIN0 + GPIO_PIN1);

    GPIO_setAsPeripheralModuleFunctionInputPin(
            GPIO_PORT_P3,
            GPIO_PIN0 + GPIO_PIN1
    );


    //Initialize Master
    USCI_B_I2C_initMasterParam param = {0};
    param.selectClockSource = USCI_B_I2C_CLOCKSOURCE_SMCLK;
    param.i2cClk = UCS_getSMCLK();
    param.dataRate = USCI_B_I2C_SET_DATA_RATE_100KBPS;
    USCI_B_I2C_initMaster(USCI_B0_BASE, &param);

    //Specify slave address
    USCI_B_I2C_setSlaveAddress(USCI_B0_BASE,SLAVE_ADDRESS_DS1621);

    //Set Transmit mode
    USCI_B_I2C_setMode(USCI_B0_BASE,
                       USCI_B_I2C_TRANSMIT_MODE
    );

    //Enable I2C Module to start operations
    USCI_B_I2C_enable(USCI_B0_BASE);


    //CONFIGURATION: ADDRESS BYTE + COMMAND BYTE + DATA BYTE
    // SLAVE_ADDRESS_DS1621 + K_DS1621_ACCES_CONFIG + K_DS1621_CONTINUOUS_CONFIG

    USCI_B_I2C_masterSendMultiByteStart(USCI_B0_BASE,K_DS1621_ACCES_CONFIG);

    USCI_B_I2C_masterSendMultiByteFinish(USCI_B0_BASE,K_DS1621_CONTINUOUS_CONFIG);

    //Delay until transmission completes
    while (USCI_B_I2C_isBusBusy(USCI_B0_BASE)) ;


    //START COMMAND- continuous consecutive conversions mode
    USCI_B_I2C_masterSendSingleByte(USCI_B0_BASE,K_DS1621_START_CONVERT);

    //Delay until transmission completes
    while (USCI_B_I2C_isBusBusy(USCI_B0_BASE)) ;
    __delay_cycles(TEMP_CONVERSION_TIME);

    //READ FROM A SINGLE-BYTE SLOPE
    USCI_B_I2C_masterSendMultiByteStart(USCI_B0_BASE, K_DS1621_READ_SLOPE);

    //Poll for transmit interrupt flag.
    while (!(HWREG8(USCI_B0_BASE + OFS_UCBxIFG) & UCTXIFG));

    USCI_B_I2C_masterReceiveSingleStart (USCI_B0_BASE);
    Count_Per_C=USCI_B_I2C_masterReceiveSingle(USCI_B0_BASE);

    while (USCI_B_I2C_isBusBusy(USCI_B0_BASE)) ;

    // Temperature READ LOOP
    while (1)
    {
        //READ FROM A SINGLE-BYTE COUNTER
        USCI_B_I2C_masterSendMultiByteStart(USCI_B0_BASE, K_DS1621_READ_COUNTER);
        //Poll for transmit interrupt flag.
        while (!(HWREG8(USCI_B0_BASE + OFS_UCBxIFG) & UCTXIFG));
        USCI_B_I2C_masterReceiveSingleStart (USCI_B0_BASE);
        Count_Remain=USCI_B_I2C_masterReceiveSingle(USCI_B0_BASE);

        while (USCI_B_I2C_isBusBusy(USCI_B0_BASE)) ;


        //READ FROM A TWO-BYTE REGISTER TEMPERATURE
        USCI_B_I2C_masterSendMultiByteStart(USCI_B0_BASE,K_DS1621_READ_TEMP);
        //Poll for transmit interrupt flag.
        while (!(HWREG8(USCI_B0_BASE + OFS_UCBxIFG) & UCTXIFG));
        USCI_B_I2C_masterReceiveMultiByteStart (USCI_B0_BASE);
        while((!(HWREG8(USCI_B0_BASE + OFS_UCBxIFG) & UCRXIFG)) && (!(HWREG8(USCI_B0_BASE+ OFS_UCBxIFG) & UCNACKIFG)));

        if((HWREG8(USCI_B0_BASE+ OFS_UCBxIFG) & UCNACKIFG))
        {
            //Send stop condition.
            HWREG8(USCI_B0_BASE + OFS_UCBxCTL1) |= UCTXSTP;
            //Wait for Stop to finish
            while (HWREG8(USCI_B0_BASE + OFS_UCBxCTL1) & UCTXSTP);

        }
        else{
            //MSB TEMPERATURE
            ucRegData[0]=USCI_B_I2C_masterReceiveMultiByteFinish(USCI_B0_BASE);

            //LSB TEMPERATURE
            ucRegData[1]=USCI_B_I2C_masterReceiveMultiByteNext(USCI_B0_BASE);

            //TEMPERATURE
            temp_DS1621=(ucRegData[1]==0 ? (float) ucRegData[0] : (float) ucRegData[0] + 0.5);

            //HIGHER RESOLUTION TEMPERATURE
            temp=temp_DS1621-0.25+((float)(Count_Per_C-Count_Remain)/(float)Count_Per_C);
        }

        while (USCI_B_I2C_isBusBusy(USCI_B0_BASE)) ;
    }
}


