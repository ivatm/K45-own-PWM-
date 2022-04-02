/*
 * Interface.c
 *
 *  Created on: 13 ????. 2020 ?.
 *      Author: Oliva
 */

#include <stdint.h>
#include <stdio.h>
#include "defines.h"

/************************************************************************************************
 * The procedure gets the inputed value for set Temperature
 ************************************************************************************************/
uint16_t getTemperatureSet(void);

uint16_t getTemperatureSet(void)
{
   float iTset;

//   printf("Enter a Temperature : ");
   scanf("%f", &iTset);

//   printf("the name entered is: %f\n", iTset);

   return(iTset);
}

