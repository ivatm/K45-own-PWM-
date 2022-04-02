/*
 * Interpolations.c
 *
 *  Created on: 13 ????. 2020 ?.
 *      Author: Oliva
 */

// Includs ------------------------------------------------
#include "ThermoControlHeaders.h"
#include "globalvarheads.h"
#include "stdio.h"
#include <time.h>

// extern procedures --------------------------------------
extern void getSensCharacteristic(uint16_t* piPointNumber, uint16_t* piTemperature_Points, uint16_t* piVoltage_Points);
extern boolean getPIDcoefs(void);

// Local procedures ---------------------------------------

uint16_t TableLineInterpolation(float fXX, uint16_t* piPointNumber, uint16_t* piX_Points, uint16_t* piY_Points);
uint16_t TableLineInterpolationFloat(float fXX, uint16_t* piPointNumber, uint16_t* piX_Points, uint16_t* piY_Points);
// Variables ----------------------------------------------


#ifdef  STEP_CONST
/* -------------------------------------------------------------------------------
   TableLineInterpolation
   Procedure for calculation Y (temperature) from given X (voltage)
   Calculations are realized by means of tables T(U) inputed as iX and number of temperature point.
   Correctly works when sensor table has constant temperature step.
   Call from TemperatureCalc
   ------------------------------------------------------------------------------- */
uint16_t TableLineInterpolation(float fXX, uint16_t* piPointNumber, uint16_t* piX_Points, uint16_t* piY_Points)
{
   uint32_t lLocalWork;
   uint16_t iIndex;
   uint16_t iFirstPoint, iYStep;

   uint32_t lXX;

   #define mGetLongVoltage(px, shift) (uint32_t)((*(px + shift))*100)

   // Get Micro volts in integer
   lXX = fXX * 1000000;

#ifdef debugmode
//   lXX = 1016711;
#endif

   iIndex = 0;
   iFirstPoint = *piY_Points;
   iYStep = *(piY_Points + 1) - *piY_Points;

   if ((piPointNumber   == NULL)
         || (piX_Points == NULL)
         || (piY_Points == NULL)
         || (piY_Points == NULL)
         || (*piPointNumber == 0)
         || (*piY_Points >= *(piY_Points + 1))
         )
   {
      // Error
      return(0);
   }
   else
   {
      if (lXX > (mGetLongVoltage(piX_Points, iIndex)))
      {
         // Out first temperature point
         return(*piY_Points);
      }
      else
      {
         if (lXX < (mGetLongVoltage(piX_Points, (*piPointNumber - 1))))
         {
            // Out last temperature point

            return(*(piY_Points + *piPointNumber - 1));
         }
      }
   }


   while ((lXX < (mGetLongVoltage(piX_Points, iIndex))) && (iIndex <= (*piPointNumber - 1)))
   {
      iIndex++;
   }

   if (iIndex)
   {
      iIndex--;
   }

   lLocalWork = ((uint32_t)iFirstPoint + (uint32_t)iIndex * iYStep);

   if ((lXX == (mGetLongVoltage(piX_Points, iIndex)))
         || (iIndex == 0)//
         || (iIndex == (*piPointNumber - 1))//
       //  || (iIndex <= (mGetLongVoltage(piX_Points, *piPointNumber - 1)))//
         || ((mGetLongVoltage(piX_Points, iIndex)) <= (mGetLongVoltage(piX_Points, iIndex+1))))                       // For divide by zero escaping
   {
      // iLocalWork = iLocalWork;                               // This knot temperature
   }
   else
   {

      lLocalWork = lLocalWork + (((uint32_t)(mGetLongVoltage(piX_Points, iIndex)) - lXX) * (uint32_t)iYStep) /
            (uint32_t)(mGetLongVoltage(piX_Points, iIndex) - mGetLongVoltage(piX_Points, iIndex+1));

   }

   return (uint16_t)(lLocalWork);
}

/* -------------------------------------------------------------------------------
   TableLineInterpolationFloat
   ------------------------------------------------------------------------------- */
/*
uint16_t TableLineInterpolationFloat(float fXX, uint16_t* piPointNumber, uint16_t* piX_Points, uint16_t* piY_Points)
{
   float fLocalWork;
   uint16_t iIndex;
   uint16_t iFirstPoint, iYStep;

   #define mGetLongVoltage(px, shift) (float)((float)(*(px + shift))/10000)

   if ((piPointNumber   == NULL)
         || (piX_Points == NULL)
         || (piY_Points == NULL)
         || (piY_Points == NULL)
         || (*piPointNumber == 0)
         || (*piY_Points >= *(piY_Points + 1))
         )
   {
  //    printf("oOPS \r\n");
      // Error
      return(0);
   }

   iIndex = 0;
   iFirstPoint = *piY_Points;
   iYStep = *(piY_Points + 1) - *piY_Points;

   while ((fXX < (mGetLongVoltage(piX_Points, iIndex))) && (iIndex < (*piPointNumber - 1)))
   {
      iIndex++;
   }

   if (iIndex)
   {
      iIndex--;
      if (iIndex >= *piPointNumber - 1)
      {
         iIndex = *piPointNumber - 1;
      }
   }

   fLocalWork = ((float)iFirstPoint + (float)iIndex * iYStep);

   if ((fXX == (mGetLongVoltage(piX_Points, iIndex)))                                      //
         || ((mGetLongVoltage(piX_Points, iIndex)) <= (mGetLongVoltage(piX_Points, iIndex+1))))                       // For divide by zero escaping
   {
      // iLocalWork = iLocalWork;                               // This knot temperature
   }
   else if  (fXX <= (mGetLongVoltage(piX_Points, iIndex+1)))                               // Case iX > maximum(iX) here is taken into account
   {
      fLocalWork = fLocalWork + iYStep;               // Next knot temperature
   }
   else
   {

      fLocalWork = fLocalWork + (((float)(mGetLongVoltage(piX_Points, iIndex)) - fXX) * (float)iYStep) /
            (float)(mGetLongVoltage(piX_Points, iIndex) - mGetLongVoltage(piX_Points, iIndex+1));

   }

   return (uint16_t)(fLocalWork);
}
*/


#else
/* -------------------------------------------------------------------------------
   TableLineInterpolation
   Procedure for calculation Y (temperature) from given X (voltage)
   Calculations are realized by means of tables T(U) inputed as iX and iY
   Call from TemperatureCalc
   ------------------------------------------------------------------------------- */
uint16_t TableLineInterpolation(uint16_t iXX)
{
   uint16_t  iLocalWork;
   unsigned char cNumberPoints;
   unsigned char cIndex;

   /*
    * iX - Voltage
    * iY - Temperature
    *
    */

   cIndex = 0;
   cNumberPoints = iTMH_Length;
   while ((iXX < GetTableKnot(&iX[cIndex])) &&
          (cIndex < (cNumberPoints - 1)))
   {
      cIndex++;
   }


   if (cIndex) cIndex--;

   if (iXX <= GetTableKnot(&iX[cIndex]))                                      // Case iX > maximum(iX) here is taken into account
   {
      iLocalWork = (GetTableKnot(&iY[cIndex]));
   }
   else if  (iXX >= GetTableKnot(&iX[cIndex+1]))
   {
      iLocalWork = (GetTableKnot(&iY[cIndex+1]));
   }
   else
   {
      if (cIndex > cNumberPoints - 2)
      {
         cIndex = cNumberPoints - 2;
      }
      iLocalWork = GetTableKnot(&iY[cIndex]) + (((unsigned long)(GetTableKnot(&iX[cIndex])) - iXX) * (GetTableKnot(&iY[cIndex+1]) - GetTableKnot(&iY[cIndex])))
                                                 / ((unsigned long)GetTableKnot(&iX[cIndex]) - GetTableKnot(&iX[cIndex+1]));

   }

   return iLocalWork;
}
#endif

/* -------------------------------------------------------------------------------
   void iniTemperaturController(void)

   Call at start during global initial
   ------------------------------------------------------------------------------- */
void iniTemperaturController(void)
{
   getSensCharacteristic(&iTMH_Length, &iTMH_Temperature_points[0], &iTMH_Voltage_points[0]);

   getPIDcoefs();

}

/* -------------------------------------------------------------------------------
   uint32_t getTemperatureValue(float fVoltage)

   Call at start during global initial
   ------------------------------------------------------------------------------- */
uint32_t getTemperatureValue(float fVoltage)
{
   uint32_t lResult;

   lResult = TableLineInterpolation(fVoltage, &iTMH_Length, &iTMH_Voltage_points[0], &iTMH_Temperature_points[0]);

   return(lResult);
}

