/*
 * endifPIDCalculs.c
 *
 *  Created on: 13 ????. 2020 ?.
 *      Author: Oliva
 */

#include <ads1256.h>
#include "ThermoControlHeaders.h"
#include <stdio.h>
#include <pthread.h>
#include "user_types.h"
#include "globalvarheads.h"

// Extern procedures -----------------------------------
extern void getSensCharacteristic(uint16_t* piPointNumber, uint16_t* piTemperature_Points, uint16_t* piVoltage_Points);
extern sSensorVoltageDataStruct getSensorData(void);
extern uint32_t getTemperatureValue(float fVoltage);
// Extern Procedures-----------------------------------------------------------------
extern uint_fast32_t appMillis(void);

// Local procedures ------------------------------------
uint16_t Kprop  (uint16_t T);
uint32_t Kdif   (uint16_t T);
uint16_t Kintegr(uint16_t T);
void getRegulatorOuputs(void);

// Global procedures ------------------------------------
void iniTemperaturController(void);
void CalculRegulator(void);
void CalculRegulatorDebug(void);

uint16_t GetHeaterSetpoint(void);
uint16_t GetCoolerSetpoint(void);

#ifdef SCANNING
void AutoSettingTemperature(void);
#endif

// Local static Variables -----------------------------------
static pthread_mutex_t PID_lock = PTHREAD_MUTEX_INITIALIZER;
static uint16_t iRegulatorEffect;

/* ------------------------------------------------------------------------------
 * void getTemperatures()
 * The Procedure reads current state of sensors data and converts it to temperature
 * ------------------------------------------------------------------------------*/
void getTemperatures()
{
   sSensorVoltageDataStruct sSensorVoltageData;
   uint32_t  lWork1, lWork2;
   int64_t   sllWork;

   static boolean bFirstMeasurement;

   sSensorVoltageData = getSensorData();

#ifdef debugmode
   //printf("U = %f\r\n",sSensorVoltageData.fVoltageCurrent);
   //printf("t = %d\r\n",sSensorVoltageData.lTimeDelta);
#endif // debugmode

   lWork1 = getTemperatureValue(sSensorVoltageData.fVoltageCurrent);
   lWork2 = getTemperatureValue(sSensorVoltageData.fVoltageLast);
   pthread_mutex_lock(&PID_lock);

   lTemperatureReal     = lWork1;
   lTemperaturePrevious = lWork2;

   pthread_mutex_unlock(&PID_lock);

   if (!sSensorData.bMeasurementReady)
   {
      slTemperatureSpeed = mNormTime(kTemperaturControlPeriod);
      return;
   }
   else
   {
      if (!bFirstMeasurement )
      {
         bFirstMeasurement = TRUE;
         lTemperatureSet = lTemperatureReal;
      }

      /* according to measured time the relative reduced value of temperature changing speed
       * DT/dt = DTx/100ms => DTx = DT * 100ms / dt
      */

      sllWork = (int64_t)((int64_t)lTemperatureReal - lTemperaturePrevious) * mNormTime(kTemperaturControlPeriod);

      if (sSensorVoltageData.lTimeDelta)
      {
         slTemperatureSpeed = (int32_t)(sllWork / sSensorVoltageData.lTimeDelta);
      }
      else
      {
         //printf(" error!: Timedelta = 0");
      }
   }

}

/* -------------------------------------------------------------------------------
   CalculRegulator
   Procedure for calculation output regulator effect out PID

   Call from
   ------------------------------------------------------------------------------- */
void CalculRegulator(void)
{

   int64_t sllAccumulator;
#define kKpropWeight     50
#define kKdiffWeight     1

#ifdef SCANNING
   int64_t sllDeltaT = (int64_t)lTemperatureCurrentSet - lTemperatureReal;
#else
   int64_t sllDeltaT = (int64_t)lTemperatureSet - lTemperatureReal;
#endif

   getTemperatures();
   // Derivative (diff-Part) with "-"
   sllAccumulator = (int64_t)(sllDeltaT * Kprop(lTemperatureReal))/kKpropWeight - (int64_t)((int64_t)slTemperatureSpeed * Kdif(lTemperatureReal))/kKdiffWeight;

   sllAccumulator = sllAccumulator + (int64_t)iRegulatorEffect;

   iRegulatorEffect = mLimitWord(sllAccumulator);                                       // Output effect for regulator

   lTemperaturePrevious = lTemperatureReal;

   // Calculation output effects
   getRegulatorOuputs();
}

#ifdef debugmode
//void CalculRegulatorDebug(void)
//{
   //uint16_t  iWork1;

//   float fVolt;

//   fVolt = sSensorData.fVoltageCurrent;

//   /*iWork1 =*/ getTemperatureValue(fVolt);
//}
#endif

/* -------------------------------------------------------------------------------
   Kprop
   Procedure for calculation proportional enclosure of PID-low
   Call from CalculRegulator
   ------------------------------------------------------------------------------- */
uint16_t Kprop(uint16_t T)
{
   if (lKprop > kCoefMax)
   {
      lKprop = kCoefMax;
   }
   return (lKprop);
}

/* -------------------------------------------------------------------------------
   Kdif
   Procedure for calculation differential enclosure of PID-low
   Call from CalculRegulator
   ------------------------------------------------------------------------------- */
uint32_t Kdif(uint16_t T)
{
   if (lKdiff > kCoefMax)
   {
      lKdiff = kCoefMax;
   }
   return (lKdiff);//(uint32_t)lKdiff * 512);
}

/* -------------------------------------------------------------------------------
   Kintegr
   Procedure for calculation integral enclosure of PID-low
   Call from CalculRegulator
   ------------------------------------------------------------------------------- */
uint16_t Kintegr(uint16_t T)
{
   return 1; // Kintegr = 1 by definition
}

/* -------------------------------------------------------------------------------
   getRegulatorOuputs
   Procedure for output regulator effect by means of two PWM
   1. Heating Ni * K(T);
      ControlLevelChandge = 200 K
      K ~ T -> T=0..200K; K = 200/T;
      K = 1 -> T>200 K;

   2. Cooling (Nmax-Ni)^2.

   3. Cooling on PWM output timer 2 with low frequency.
   Call from MainLoop
   ------------------------------------------------------------------------------- */
void getRegulatorOuputs(void)
{
   uint16_t  iOut;
//   unsigned char cOut;
   uint32_t lLocal;

   #if (kTemperatureMin == 0)
      #define kControlLevelChandge mNormTemp(200)
   #else
      #define kControlLevelChandge 200
      float         fWork;
   #endif

   lLocal = (uint32_t)iRegulatorEffect;

   #if (kTemperatureMin == 0)
      if (lTemperatureReal < kControlLevelChandge)
      {
         lLocal = lLocal * lTemperatureReal;
         lLocal = lLocal / kControlLevelChandge;
      }
   #else
      fWork = mDeNormTemp(lTemperatureReal);
      if (fWork < kControlLevelChandge)
      {
         fWork = fWork / kControlLevelChandge;
         lLocal = lLocal * fWork;
      }
   #endif

   iOut   = mAccToRegEffect((lLocal * kMaxRegulatorEffect) / 65535);

   pthread_mutex_lock(&PID_lock);
   iHeaterEffect  = iOut;
   pthread_mutex_unlock(&PID_lock);

   iOut   = 65535 - iRegulatorEffect;
   lLocal = (uint32_t)iOut * iOut;
   lLocal = lLocal / 65535;

   iOut   = mAccToRegEffect((lLocal * kMaxRegulatorEffect) / 65535);

   pthread_mutex_lock(&PID_lock);
   iCoolerEffect = iOut;
   pthread_mutex_unlock(&PID_lock);

   #ifdef VALVE_CONTROL
      pthread_mutex_lock(&PID_lock);
      iValveOutput = iOut;
      pthread_mutex_unlock(&PID_lock);
   #endif

#ifdef debugmode
   // Control point
   //   printf("iRegulatorEffect  = %u;\r\n", iRegulatorEffect);
   //   printf("iHeaterEffect  = %u;\r\n", iHeaterEffect);
   //   printf("iCoolerEffect  = %u;\r\n", iCoolerEffect);
   //   printf("iValveOutput  = %u;\r\n", iValveOutput);
#endif // debugmode

}

/*
 * */
uint16_t GetHeaterSetpoint(void)
{
   return(iHeaterEffect);
}

uint16_t GetCoolerSetpoint(void)
{
   return(iCoolerEffect);
}

#ifdef SCANNING
/* -------------------------------------------------------------------------------
   AutoSettingTemperature
   Procedure for setting temperature in automatic scanning mode
   The flag of scanning mode is difference from 0 lDelta_t and lDelta_T
   Call from MainLoop
   ------------------------------------------------------------------------------- */
void AutoSettingTemperature(void)
{
   static uint32_t lTau;
   boolean bLocalBool;

   bLocalBool = FALSE;

  if (sSensorData.bMeasurementReady)
  {
     //printf("lDelta_t = %d \r\n", lDelta_t);

     if (bScanOrSetMode
         && (lDelta_t > mNormTime(0))
         && (lDelta_T > mNormTemp(0)))
     {

        if ((appMillis() - lTau) <= lDelta_t)
        {
           // No any iTemperatureCurent changing
        }
        else
        {
           lTau = appMillis();
           if (mAbs((int32_t)lTemperatureCurrentSet - lTemperatureSet) > lDelta_T)
           {
              if (lTemperatureCurrentSet > lTemperatureSet)
              {
                 lTemperatureCurrentSet -= lDelta_T;
              }
              else // iTemperatureCurent < lTemperatureSet
              {
                 lTemperatureCurrentSet += lDelta_T;
              }
           }
           else
           {
              // Temperature achieved, stop scanning
              bLocalBool = TRUE;
              lTemperatureCurrentSet = lTemperatureSet;
           }
        }
     }
     else
     {
        lTemperatureCurrentSet = lTemperatureSet;
        lTau = appMillis();
     }
  }
  else
  {
     // scanning impossible
  }

  bTempSetAchieved = bLocalBool;
}

#endif

