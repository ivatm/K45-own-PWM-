/*
 * for_tests_interface.c
 *
 *  All functions necessary for testing
 *
 *  Created on: 3 груд. 2021 р.
 *      Author: Oliva
 */
#include "DEV_Config.h"

#ifdef TEST
UBYTE ADS1256_init(void)
{
   return(0);
}

void DEV_ModuleExit(void)
{
   // Nothing to do
   return;
}

void K45_Exit(uint16_t iReason)
{
   // Nothing to do
   return;
}

UDOUBLE ADS1256_GetChannalValue(UBYTE Channel)
{
   return(65535);
}

void ADS1256_Cal(void)
{
   // Nothing to do
   return;
}

#endif


