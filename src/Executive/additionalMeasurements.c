/*
 * additionalMeasurements.c
 *
 *  Created on: 8 лют. 2022 р.
 *      Author: Oliva
 */

#ifndef TEST
#include <pthread.h>
#endif
#include "pigpio.h"
#include <stdio.h>

#include "addMeasurements.h"
#include "defines.h"
#include "globalvarheads.h"
#include <bcm2835.h>
#include "user_const.h"

void FrequencyMeasurement(void);
void ImpulsCounter(int gpio, int level, uint32_t tick);
void HeaterCheck(int gpio, int level, uint32_t tick);
void CoolerCheck(int gpio, int level, uint32_t tick);
void ControlDiodeCheck(int gpio, int level, uint32_t tick);

int PigpioInit(void);

static pthread_mutex_t ImpulsCounter_lock = PTHREAD_MUTEX_INITIALIZER;
static uint32_t lLocalImpulsCounter;
uint32_t lMeasuredFrequency;

//
extern int gpioCfgClock(unsigned cfgMicros, unsigned cfgPeripheral, unsigned cfgSource);
extern uint32_t GetCryoLevel(uint32_t MesuredFrequency);

// ------------------------------------------------------------------------------------------------
/* ----------------------------------------------
 * Pigpio Initialization
 * ---------------------------------------------- */
int PigpioInit(void)
{
   gpioCfgClock(g_opt_s, 1, 1);

   if (gpioInitialise()<0) return 1;

   // if (gpioRead(fPowerModulStatus.sStatus.bHeaterError ))
   if (bcm2835_gpio_lev(kGPIO_check_diode))
   {
      fPowerModulStatus.sStatus.bControlDiodeError = TRUE;
   }

   if (bcm2835_gpio_lev(kGPIO_check_heater))
   {
      fPowerModulStatus.sStatus.bHeaterError  = TRUE;
   }

   if (bcm2835_gpio_lev(kGPIO_check_cooler))
   {
      fPowerModulStatus.sStatus.bCoolerError  = TRUE;
   }

   // Frequency measurement.
   // Only front edge checked
   gpioSetISRFunc(kGPIO_frequency_input, RISING_EDGE, -1, ImpulsCounter);

   #ifndef NO_CONTROL_LEVELS
      // Inition heater control voltage
      // Both edges are checked to be able to find out where the impuls ended
      gpioSetAlertFunc(kGPIO_check_heater, HeaterCheck);

      // Inition cooler (evaporator) control voltage
      // Both edges are checked to be able to find out where the impuls ended
      gpioSetAlertFunc(kGPIO_check_cooler, CoolerCheck);

      // Inition diode control voltage
      // Both edges are checked to be able to find out where the impuls ended
      gpioSetAlertFunc(kGPIO_check_diode, ControlDiodeCheck);
   #endif

   return 0;
}

/* ----------------------------------------------
 * Callback function to count impulse
 * ---------------------------------------------- */
void ImpulsCounter(int gpio, int level, uint32_t tick)
{

   if (level == 1)
   {

      if (lLocalImpulsCounter < Def_HighLimitFrequence)
      {
         lLocalImpulsCounter++;
      }
   }
   else
   {
      if (level == 2)
      {
         printf("Watchdog\r\n");
         lLocalImpulsCounter = 0;
      }
   }
}

/* ----------------------------------------------
 * Calculation procedure every 1 second
 * ---------------------------------------------- */
void FrequencyMeasurement(void)
{
   static struct timespec spec;
   struct timespec specLocal;
   uint64_t llWorkLocal;
   uint32_t lMilliSeconds;

   clock_gettime(CLOCK_MONOTONIC, &specLocal);
   llWorkLocal = 1000000000*(specLocal.tv_sec - spec.tv_sec) + (specLocal.tv_nsec - spec.tv_nsec);
   lMilliSeconds = llWorkLocal / 1000000;
   spec = specLocal;


   if (lMilliSeconds > 0)
   {
      pthread_mutex_lock(&ImpulsCounter_lock);
      lMeasuredFrequency = (lLocalImpulsCounter * 1000) / lMilliSeconds; // Hz
      lLocalImpulsCounter = 0;
      pthread_mutex_unlock(&ImpulsCounter_lock);
   }
   else
   {
      lMeasuredFrequency = 0;
   }

   lMeasuredFrequency = lMeasuredFrequency * kCoefFrequency;

   lCryoLevel = GetCryoLevel(lMeasuredFrequency);
}

// ------------------------------------------------------------------------------------------------
/* ----------------------------------------------
 * Callback on GPIO - interrupt on state change
 * ---------------------------------------------- */
void HeaterCheck(int gpio, int level, uint32_t tick)
{

   if (level == 1)
   { // change to high (a rising edge)
      fPowerModulStatus.sStatus.bHeaterError  = FALSE;
   }
   else
   {
      fPowerModulStatus.sStatus.bHeaterError  = TRUE;
   }
}

/* ----------------------------------------------
 * Callback on GPIO - interrupt on state change
 * ---------------------------------------------- */
void CoolerCheck(int gpio, int level, uint32_t tick)
{
   //if (bcm2835_gpio_lev(kGPIO_check_cooler))
   if (level == 1)
   {// change to high (a rising edge)
      fPowerModulStatus.sStatus.bCoolerError  = FALSE;
   }
   else
   {
      fPowerModulStatus.sStatus.bCoolerError  = TRUE;
   }
}

/* ----------------------------------------------
 * Callback on GPIO - interrupt on state change
 * ---------------------------------------------- */
void ControlDiodeCheck(int gpio, int level, uint32_t tick)
{
   // if (bcm2835_gpio_lev(kGPIO_check_heater))
   if (level == 1)
   {// change to high (a rising edge)
      //printf("Clear\r\n");
      fPowerModulStatus.sStatus.bControlDiodeError = FALSE;
   }
   else
   {
      //printf("Set\r\n");
      fPowerModulStatus.sStatus.bControlDiodeError = TRUE;
   }
}

void ControlVoltagesCheck(void)
{
   // ???
}
