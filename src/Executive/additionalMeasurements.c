/*
 * additionalMeasurements.c
 *
 *  Created on: 8 ���. 2022 �.
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

void FrequencyMeasurement(void);
void ImpulsCounter(int gpio, int level, uint32_t tick);
void HeaterCheck(int gpio, int level, uint32_t tick);
void CoolerCheck(int gpio, int level, uint32_t tick);
void ControlDiodeCheck(int gpio, int level, uint32_t tick);

int PigpioInit(void);

static pthread_mutex_t ImpulsCounter_lock = PTHREAD_MUTEX_INITIALIZER;
static uint16_t iLocalImpulsCounter;
uint16_t iMeasuredFrequency;

//
extern int gpioCfgClock(unsigned cfgMicros, unsigned cfgPeripheral, unsigned cfgSource);

// ------------------------------------------------------------------------------------------------
/* ----------------------------------------------
 * Pigpio Initialization
 * ---------------------------------------------- */
int PigpioInit(void)
{
   gpioCfgClock(g_opt_s, 1, 1);

   if (gpioInitialise()<0) return 1;

   // Frequency measurement.
   // Only front edge checked
   gpioSetISRFunc(kGPIO_frequency_input, RISING_EDGE, -1, ImpulsCounter);

   // Inition heater control voltage
   // Both edges are checked to be able to find out where the impuls ended
   gpioSetAlertFunc(kGPIO_check_heater, HeaterCheck);

   // Inition cooler (evaporator) control voltage
   // Both edges are checked to be able to find out where the impuls ended
   gpioSetAlertFunc(kGPIO_check_cooler, CoolerCheck);

   // Inition diode control voltage
   // Both edges are checked to be able to find out where the impuls ended
   gpioSetAlertFunc(kGPIO_check_diode, ControlDiodeCheck);

   return 0;
}

/* ----------------------------------------------
 * Callback function to count impulse
 * ---------------------------------------------- */
void ImpulsCounter(int gpio, int level, uint32_t tick)
{

   if (level == 1)
   {

      if (iLocalImpulsCounter < 60000)
      {
         iLocalImpulsCounter++;
      }
   }
   else
   {
      if (level == 2)
      {
         printf("Watchdog\r\n");
         iLocalImpulsCounter = 0;
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
      iMeasuredFrequency = (iLocalImpulsCounter * 1000) / lMilliSeconds; // Hz
      iLocalImpulsCounter = 0;
      pthread_mutex_unlock(&ImpulsCounter_lock);
      printf("iMeasuredFrequency = %d\r\n", iMeasuredFrequency);
   }
   else
   {
      iMeasuredFrequency = 0;
   }
}

// ------------------------------------------------------------------------------------------------
/* ----------------------------------------------
 * Callback on GPIO - interrupt on state change
 * ---------------------------------------------- */
void ControlDiodeCheck(int gpio, int level, uint32_t tick)
{
   //if (bcm2835_gpio_lev(kGPIO_check_diode))
   if (level == 1)
   {
      fPowerModulStatus.sStatus.bErrorValue1 = FALSE;
   }
   else
   {
      fPowerModulStatus.sStatus.bErrorValue1 = TRUE;
   }
}

/* ----------------------------------------------
 * Callback on GPIO - interrupt on state change
 * ---------------------------------------------- */
void CoolerCheck(int gpio, int level, uint32_t tick)
{
   //if (bcm2835_gpio_lev(kGPIO_check_cooler))
   if (level == 1)
   {
      fPowerModulStatus.sStatus.bErrorValue2 = FALSE;
   }
   else
   {
      fPowerModulStatus.sStatus.bErrorValue2 = TRUE;
   }
}

/* ----------------------------------------------
 * Callback on GPIO - interrupt on state change
 * ---------------------------------------------- */
void HeaterCheck(int gpio, int level, uint32_t tick)
{
   // if (bcm2835_gpio_lev(kGPIO_check_heater))
   if (level == 1)
   {
      fPowerModulStatus.sStatus.bErrorValue3 = FALSE;
   }
   else
   {
      fPowerModulStatus.sStatus.bErrorValue3 = TRUE;
   }
}

void ControlVoltagesCheck(void)
{
   // ???
}