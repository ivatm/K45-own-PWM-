/*
 * SPI1.c
 *
 *  Created on: 1 זמגע. 2020 נ.
 *      Author: Oliva
 */

#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include "ADS1256.h"
#include "stdio.h"
#include <time.h>
#include <string.h>
#include <sys/timeb.h>

#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

#include "defines.h"
//#include "Indicator.h"
#include "modulSPI.h"

#include "user_types.h"
#include "globalvarheads.h"

void Ind_GPIOConfig(void);
boolean periferal_SPI1_Init(void);


// Global variables --------------------------------------------------------------

// Variables
// Variable table ---------------

void Ind_GPIOConfig(void)
{
    //output
 //  bcm2835_gpio_fsel(IND_CS_PIN, BCM2835_GPIO_FSEL_OUTP);
   bcm2835_gpio_fsel(MODUL_ISP_PIN, BCM2835_GPIO_FSEL_OUTP);
}

// Local Variables ---------------------------------------------------------------


/******************************************************************************
function: boolean periferal_SPI1_Init(void)
parameter: -
return:    boolean is it in order init
Info:      The function fulfills the initialization of SPI1
******************************************************************************/
boolean periferal_SPI1_Init(void)
{
    if(!bcm2835_init())
    {
        //printf("bcm2835 init failed  !!! \r\n");
        return (FALSE);
    }
    else
    {
       // printf("bcm2835 init success !!! \r\n");
    }

    Ind_GPIOConfig();

    uint16_t BoolFlag =
    bcm2835_aux_spi_begin();                                         //Start spi interface, set spi pin for the reuse function

    if (BoolFlag)
    {
      // printf("spi_begin success !!! \r\n");
    }
    else
    {
       //printf("spi_begin Problem !!! \r\n");
       return (FALSE);
    }

 //   bcm2835_spi_setBitOrder(BCM2835_SPI_BIT_ORDER_MSBFIRST);          //High first transmission
 //   bcm2835_spi_setDataMode(BCM2835_SPI_MODE1);                       //spi mode 0
 //   bcm2835_spi_setClockDivider(BCM2835_SPI_CLOCK_DIVIDER_256);     // The default
 //   bcm2835_spi_chipSelect(BCM2835_SPI_CS_NONE);                      // The default
 //   bcm2835_spi_setChipSelectPolarity(BCM2835_SPI_CS_NONE, LOW);      // the default

    // Service pin configure
    //  bcm2835_gpio_set(IND_CS_PIN);
      bcm2835_gpio_set(MODUL_ISP_PIN);


    return (TRUE);
}

