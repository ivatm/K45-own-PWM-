/*
 * ADC_Unit.c
 *
 *  Created on: 12 זמגע. 2020 נ.
 *      Author: Iva
 */

#include <stdlib.h>     //exit()
#include <time.h>
#include "ads1256.h"
#include "stdio.h"
#include <time.h>
#include <string.h>
#include <sys/timeb.h>

#ifndef TEST
#include <pthread.h>
#endif
#include "defines.h"

#include "globalvarheads.h"

// Extern Procedures-----------------------------------------------------------------
extern uint_fast32_t appMillis(void);
extern void ADS1256_Cal(void);
extern void K45_Exit(uint16_t iReason);

extern uint_fast32_t deltaMilis(struct timespec* specStatic);
// Local Procedures ----------------------------------------------------------------
float convertCodeADC(uint32_t ADCCode);
int32_t getMicroVoltsADC(uint32_t ADCCode);

// Local variables -----------------------------------------------------------------
#ifndef TEST
static pthread_mutex_t data_lock = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t ADC_lock = PTHREAD_MUTEX_INITIALIZER;
#else
static uint32_t data_lock = 0;
static uint32_t ADC_lock = 0;
#endif
// ADC Variables -------------------------------------------------
uint16_t iUref; // Reference value for ADC
float    fUref; // Reference float value for ADC

int ADC_Init(void);


int ADC_Init(void)
{

    if(ADS1256_init() == 1)
    {
       // It means erroneous initialize,
       // so the Program exits
        printf("\r\nEND                  \r\n");
        DEV_ModuleExit();
        K45_Exit(1);
    }

    return TRUE;
}

UDOUBLE GetADCData(uint16_t ChannelNumber)
{
   UDOUBLE Result;
   //UDOUBLE ADC[8];
   //float x;

   Result = ADS1256_GetChannalValue(ChannelNumber);

   return(Result);
}

/* ------------------------------------------------------------------------------------
 *  void updateCurrentVoltages(void)
 *  The procedure measures voltage on kSensorInput, and fixes the time of measurement
 *  The new voltage replaces the position of previous and the previous becomes last one for
 *  the next calculation
 *  Before calling:
 *  Ucurrent = Ux
 *  Ulast    = Uy
 *  After calling:
 *  New measured value Uz
 *  Ucurrent = Uz
 *  Ulast    = Ux
 * ------------------------------------------------------------------------------------ */
void updateCurrentVoltages(void)
{
   static struct timespec spec;
   struct timespec specLocal;
   uint64_t llWorkLocal;
   uint32_t lMilliSeconds;

   // Variable to count the fulfilled measurements
   static uint16_t iLocalCounter;
   #define kFirstWrongMeasurementsNumber 3

   UDOUBLE lLocalADCValue = GetADCData(kSensorInput);

/*
   static struct timespec TimerLocal; // Local timer for save last state
   printf("dTime of GetADCData  calling %d\n\r", deltaMilis(&TimerLocal));
   printf("duration GetADCData  calling %d\n\r", deltaMilis(&TimerLocal));
*/

 //  UDOUBLE lLocalADCValue = GetADCData(kSensorInput);
   float NewVoltageValue  = convertCodeADC(lLocalADCValue);

   //printf("NewVoltageValue  = %d\n\r", lLocalADCValue);

#ifndef TEST
   clock_gettime(CLOCK_MONOTONIC, &specLocal);
#else
   specLocal.tv_nsec = 0xFF;
   specLocal.tv_sec = 0xFF;
#endif
   llWorkLocal = 1000000000*(specLocal.tv_sec - spec.tv_sec) + (specLocal.tv_nsec - spec.tv_nsec);
   lMilliSeconds = llWorkLocal / 1000000;

   spec = specLocal;

#ifndef TEST
   pthread_mutex_lock(&data_lock);
#else
   data_lock = 1;
#endif

   sSensorData.fVoltageLast    = sSensorData.fVoltageCurrent;
   sSensorData.fVoltageCurrent = NewVoltageValue;
   sSensorData.lUlast          = sSensorData.lUcurrent;
   sSensorData.lUcurrent       = lLocalADCValue;

   sSensorData.lTimeDelta = lMilliSeconds;

#ifndef TEST
   pthread_mutex_unlock(&data_lock);
#else
   data_lock = 0;
#endif

#ifdef debugmode
//   printf("TimeDelta  = %d\n\r", sSensorData.lTimeDelta);
#endif

   /* To prevent using of several starting wrong measurements
      the flag bMeasurementReady set On only after kFirstWrongMeasurementsNumber
   */
   if (iLocalCounter < kFirstWrongMeasurementsNumber)
   {
      iLocalCounter++;
   }
   else
   {
      sSensorData.bMeasurementReady = TRUE;
   }

}

/* -----------------------------------------------------------------
 * sSensorVoltageDataStruct getSensorData(void)
 * Global procedure to get the current state of sensor measurements in any time
 *
 * ----------------------------------------------------------------- */
sSensorVoltageDataStruct getSensorData(void)
{
   return(sSensorData);
}

/* -----------------------------------------------------------------
 * float_t convertCodeADC(uint32_t)
 * Procedure returns the float value according to input ADC Code
 * -----------------------------------------------------------------*/
float convertCodeADC(uint32_t ADCCode)
{
   float x;
   int32_t WorkLocal;

   WorkLocal = (int32_t)ADCCode;

   if (WorkLocal >= 0)
   {
      x = WorkLocal * kfUmax / 0x7fffff;
   }
   else
   {
      x = WorkLocal * kfUmax / 0x7fffff;
   }

   return(x);
}


/* -----------------------------------------------------------------
 * int32_t getMicroVoltsADC(uint32_t)
 * Procedure returns the float value according to input ADC Code
 * -----------------------------------------------------------------*/
int32_t getMicroVoltsADC(uint32_t ADCCode)
{
   int32_t WorkLocal;

   WorkLocal = (int32_t)ADCCode;

   if (WorkLocal >= 0)
   {
      WorkLocal = (int32_t)(((int64_t)WorkLocal * (kfUmax * 1000000)) / 0x7fffff);
   }
   else
   {
      WorkLocal = (int32_t)(((int64_t)WorkLocal * (kfUmax * 1000000)) / 0x7fffff);
   }

   return(WorkLocal);
}


/* ===============================================================================
 * The ADC full Calibration.
 * The external library for ADC plate ADS1256 is used
 * =============================================================================== */
void ADC_service(void)
{

   bADCCalibrationProcess = TRUE;

   return;
}

/* ===============================================================================
 * The ADC full Calibration.
 *
 * =============================================================================== */
void ADC_Calibration(void)
{

#ifndef TEST
   pthread_mutex_lock(&ADC_lock);
#else
   ADC_lock = 1;
#endif

   ADS1256_Cal();

#ifndef TEST
   pthread_mutex_unlock(&ADC_lock);
#else
   ADC_lock = 0;
#endif

   bADCCalibrationProcess = FALSE;

   return;
}

