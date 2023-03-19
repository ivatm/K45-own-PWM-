/*l
 * lcd.c
 *
 *  Created on: 3 april 2021 
 *      Author: Oliva
 */
#include <i2c_lcd.h>
#include <bcm2835.h>
#include <stdio.h>
#include <string.h>

#include "globalvarheads.h"
#include "ThermoControlHeaders.h"

// Variables ---------------------

// externe functions
extern int32_t getMicroVoltsADC(uint32_t ADCCode);

// Here implemented functions --------

void lcd_Init(void);
void ShowVariable(Display_Zone_struct* psDisplayStructData, display_zone_enum DisplayZoneNumber);
void ShowErrorExecutivePlate(Display_Zone_struct* psDisplayStructData, display_zone_enum DisplayZoneNumber);
void ShowScanOrSetMode(Display_Zone_struct* psDisplayStructData, display_zone_enum DisplayZoneNumber);
void GetStringOfVariable(Display_Zone_struct* psDisplayStructData, char* pOutString, boolean bVariableOnly);
void GetStringOfEnterMode(Display_Zone_struct* psDisplayStructData, char* pOutString);
void WriteFullString(char* pString, uint16_t iFullLength);
void HelloShow(void);
void K45_Exit(uint16_t iReason);

// Local variables
static boolean bLocalShowingSensorName;
static boolean bLocalShowingApplicationName;
static boolean bLocalShowingUpdateError;

// Static procedures
//static void ShowStringBytes(char* cString);

void HelloShow(void)
{
   LCDI2C_setCursor(10, 1);
   LCDI2C_write_String("K45");
   LCDI2C_setCursor(5, 2);
   LCDI2C_write_String("STARTING....");

}

// Procedure shows goodbye message on exit
static void GoodbyeShow(void)
{
   LCDI2C_setCursor(5, 2);
   LCDI2C_write_String("GOODBYE");

}

// Procedure shows goodbye message on exit
static void RebootShow(void)
{
   LCDI2C_setCursor(5, 2);
   LCDI2C_write_String("REBOOT");

}


// Code implementation ----------------------------------------------------------------------------------------------
void lcd_Init(void)
{

   LCDI2C_init(LCD_Slave_Address, 20, 4);                             //Start I2C operations.

   // Initialization of data description of current lcd state
   sDisplay_Zone[eModeZone].bOnOrOff = TRUE;
   sDisplay_Zone[eModeZone].eDataTypeDisplay = keScanningState;
   sDisplay_Zone[eModeZone].eVariable = keMaxVariableNum; // there is no variable structure

   // -----------------------------------------------------------------------------------
   sDisplay_Zone[eErrorsZone].bOnOrOff = TRUE;
   sDisplay_Zone[eErrorsZone].eDataTypeDisplay = keExecutiveModulErrors;
   sDisplay_Zone[eErrorsZone].eVariable = keMaxVariableNum; // there is no variable structure

   // -----------------------------------------------------------------------------------
   sDisplay_Zone[eStepTempZone].bOnOrOff = TRUE;
   sDisplay_Zone[eStepTempZone].eDataTypeDisplay = keSetValue;
   sDisplay_Zone[eStepTempZone].eVariable = keDeltaT;

   // -----------------------------------------------------------------------------------
   sDisplay_Zone[eStepTimeZone].bOnOrOff = TRUE;
   sDisplay_Zone[eStepTimeZone].eDataTypeDisplay = keSetValue;
   sDisplay_Zone[eStepTimeZone].eVariable = keDeltat;

   // -----------------------------------------------------------------------------------
   sDisplay_Zone[eTemperatureZone].bOnOrOff = TRUE;
   sDisplay_Zone[eTemperatureZone].eDataTypeDisplay = keMeasuredValue;
   sDisplay_Zone[eTemperatureZone].eVariable = keTreal;

   // -----------------------------------------------------------------------------------
   sDisplay_Zone[eUserValue].bOnOrOff = TRUE;
   sDisplay_Zone[eUserValue].eDataTypeDisplay = keSetValue;
   sDisplay_Zone[eUserValue].eVariable = keTset;

}

static void SetCursorInZone(display_zone_enum DisplayZoneNumber, uint16_t ShiftInLine)
{
   uint16_t col, row;

   if (DisplayZoneNumber < keZone5)
   {
      col = 10 * (DisplayZoneNumber % 2);
      row = DisplayZoneNumber / 2;
   }
   else
   {
      col = 0;

      if (DisplayZoneNumber == keZone5)
      {
         row = 2;
      }
      else
      {
         if (DisplayZoneNumber == keZone6)
         {
            row = 3;
         }
         else
         {
            // should be an error !
            row = 2;
         }
      }
   }

   if (ShiftInLine > 20)
      ShiftInLine = 20;

   LCDI2C_setCursor(col + ShiftInLine, row);
}

void clearZone(display_zone_enum DisplayZoneNumber)
{
   char string[21] = {"                    "};

   switch (DisplayZoneNumber)
   {
      case eModeZone:
      case eStepTempZone:
         string[10] = '\0';
         break;

      case eErrorsZone:
      case eStepTimeZone:
         string[11] = '\0';
         break;

      default:
         break;
   }

   sDisplay_Zone[DisplayZoneNumber].cDisplayedString[0] = '\0';

   SetCursorInZone(DisplayZoneNumber, 0);
   LCDI2C_write_String(string);
 //  printf("Ich bin da %d\r\n", DisplayZoneNumber);
}

/* --------------------------------------------------------------------------
 * Updating lcd Display
 * According to initialization all zones are updated
 * -------------------------------------------------------------------------- */
void lcd_update(void)
{
   display_zone_enum Index;

   // go through all the Zones
   for (Index = keZone1; Index < keZoneAmount; Index++)
   {
      // According to the variable/data to be displayed in the zone selection of the procedure
      switch (sDisplay_Zone[Index].eDataTypeDisplay)
      {
         case (keScanningState):
               ShowScanOrSetMode(&sDisplay_Zone[Index], Index);
               break;

         case (keExecutiveModulErrors):
               ShowErrorExecutivePlate(&sDisplay_Zone[Index], Index);
               break;

         case (keSetValue):
               ShowVariable(&sDisplay_Zone[Index], Index);
               break;

         case (keMeasuredValue):
               ShowVariable(&sDisplay_Zone[Index], Index);
               break;


         default:
               // Impossible
               break;

      }
   }

   switch (eKeyboardState)
   {
      case keEnterMode:
         LCDI2C_blink_on();
         // After prefix + number of inputed symbol show the cursor
         SetCursorInZone(eCurrentCursorZone, 4 + sInputString.cInputCursoPosition);
         break;

      case keDirectCommandEnterMode:
         LCDI2C_blink_on();
         // After prefix + number of inputed symbol show the cursor
         SetCursorInZone(eCurrentCursorZone, 2 + sInputString.cInputCursoPosition);
         break;

      default:
         // otherwise - delete the cursor
         LCDI2C_blink_off();
         break;
   }

}

// ------------------------------------------------------------------------------------------------------------------
void ShowScanOrSetMode(Display_Zone_struct* psDisplayStructData, display_zone_enum DisplayZoneNumber)
{
   char cString[kShortZoneLength];

   //clearZone(DisplayZoneNumber);
   if (psDisplayStructData->bOnOrOff)
   {

      if (bScanOrSetMode)
      {
         sprintf(cString, "%s","Scan");
      }
      else
      {
         sprintf(cString, "%s","Set");
      }
   }
   else
   {
      // Clear Zone
      // clearZone(DisplayZoneNumber);
   }

   if (strcmp(psDisplayStructData->cDisplayedString, cString))
   {
      //WriteFullString(&cString[0], kShortZoneLength);
      clearZone(DisplayZoneNumber);
      SetCursorInZone(DisplayZoneNumber, 0);
      LCDI2C_write_String(cString);
      strcpy(psDisplayStructData->cDisplayedString, cString);
   }
}

/* --------------------------------------------------------------
 * Show the current error or temporary information
 * -------------------------------------------------------------- */
void ShowErrorExecutivePlate(Display_Zone_struct* psDisplayStructData, display_zone_enum DisplayZoneNumber)
{

   sStatusByte1_Struct  sModulStatus1State;
   static eError_ToShow_enum eError_ToShow;
   boolean bNeedsToShow;
   uint16_t i;

   if (psDisplayStructData->bOnOrOff)
   {
      SetCursorInZone(DisplayZoneNumber, 0);

      /* In case of calibration, there is nothing
      to show except of message about calibration */
      if (bADCCalibrationProcess)
      {
         LCDI2C_write_String("Calibr.");
      }
      else if (bLocalShowingSensorName)
      {
         char dest[12] = "SEN:";
         strcat(dest, SensorName);
         LCDI2C_write_String(dest);
         delay(2000);
         bLocalShowingSensorName = FALSE;
      }
      else if (bLocalShowingApplicationName)
      {
         char dest[12] = "APP:";
         SetCursorInZone(keZone1, 6);
         strcat(dest, myNameApplication);
         LCDI2C_write_String(dest);
         delay(2000);
         bLocalShowingApplicationName = FALSE;
         clearZone(0);
         ShowScanOrSetMode(&sDisplay_Zone[0], 0);
      }
      else if (bLocalShowingUpdateError)
      {
         char dest[12] = "UPD_ERROR";
         LCDI2C_write_String(dest);
         delay(2000);
         bLocalShowingUpdateError = FALSE;
      }
      else
      {
         sModulStatus1State = fModulStatusByte1.sStatus;
         bNeedsToShow = FALSE;

         for (i = 0; (i < keErrorStateNumber)&&(!bNeedsToShow); i++)
         {
            if (eError_ToShow >= keErrorStateNumber)
               eError_ToShow = keHeaterVoltage;
            else
               eError_ToShow++;

            switch (eError_ToShow)
            {
               case keHeaterVoltage:
                  if (sModulStatus1State.bHeaterError )
                  {
                     bNeedsToShow = TRUE;
                     LCDI2C_write_String("Err!Heater");
                  }
                  break;
               case keCoolerVoltage:
                  if (sModulStatus1State.bCoolerError )
                  {
                     bNeedsToShow = TRUE;
                     LCDI2C_write_String("Err!Cooler");
                  }
                  break;
               case keControlDiodeVoltage:
                  if (sModulStatus1State.bControlDiodeError)
                  {
                     bNeedsToShow = TRUE;
                     LCDI2C_write_String("Err!Diode ");
                  }
                  break;
               case kePowerPlateError:
                  if (sModulStatus1State.bNotFoundErr)
                  {
                     bNeedsToShow = TRUE;
                     LCDI2C_write_String("Err!PowPl");
                  }
                  break;

               case keErrorStateNumber:
               default:
                  //eError_ToShow = keHeaterVoltage;
                  break;
            }

            if (bNeedsToShow)
            {
               // stop "for" cycle
               break;
            }
         }

         if (i >= keErrorStateNumber)
         {
            // Nothing to show = clear zone
            clearZone(DisplayZoneNumber);
         }
         else
         {
            // Nothing to do - is already shown
         }
      }
   }
   else
   {
      clearZone(DisplayZoneNumber);
   }
}

/* --------------------------------------------------------------
 * Show the variable value according to Zone configuration
 * -------------------------------------------------------------- */
void ShowVariable(Display_Zone_struct* psDisplayStructData, display_zone_enum DisplayZoneNumber)
{
   char cString[kLongZoneLength];

  // clearZone(DisplayZoneNumber);


   // For Set-mode the zones eStepTempZone and eStepTimeZone are unnecessary
   if ((DisplayZoneNumber == eStepTempZone) || (DisplayZoneNumber == eStepTimeZone))
   {
      if (!bScanOrSetMode)
      {
         psDisplayStructData->bOnOrOff = FALSE;
      }
      else
      {
         psDisplayStructData->bOnOrOff = TRUE;
      }

   }

   if (!bScanOrSetMode)
   {
      eCurrentCursorZone = eUserValue;
   }

   if (psDisplayStructData->bOnOrOff)
   {

      if (((eKeyboardState == keEnterMode) || (eKeyboardState == keDirectCommandEnterMode))
            && (DisplayZoneNumber == eCurrentCursorZone))
      {
         GetStringOfEnterMode(psDisplayStructData, &cString[0]);
      }
      else
      {
         // Returns the string in second parameter
         GetStringOfVariable(psDisplayStructData, &cString[0], FALSE);
      }

      if (eCurrentCursorZone == DisplayZoneNumber)
      {
         cString[0] = '>';
      }

      if (strcmp(psDisplayStructData->cDisplayedString, cString))
      {


         clearZone(DisplayZoneNumber);
         SetCursorInZone(DisplayZoneNumber, 0);

         LCDI2C_write_String(cString);

         strcpy(psDisplayStructData->cDisplayedString, cString);

#ifdef debugmode
         //printf("after --\r\n");
         //ShowStringBytes(psDisplayStructData->cDisplayedString);
         //ShowStringBytes(cString);
#endif // debugmode

      }

   }
   else
   {
      clearZone(DisplayZoneNumber);
   }
}

/* --------------------------------------------------------------
 * GetStringOfVariable(): converts data into the string to be displayed
 * psDisplayStructData - pointer on data configuration
 * pOutString          - Returns the string in second parameter
 * bVariableOnly       - if TRUE , only variable converted in string
 * -------------------------------------------------------------- */
void GetStringOfVariable(Display_Zone_struct* psDisplayStructData, char* pOutString, boolean bVariableOnly)
{
   uint32_t lValueToIndicator;
   char StringOut[21] = {0};
   char String[13] = {0};
   char prefix[5] = {0};
   char sufix[3] = {0};

   lValueToIndicator = *VarForIndication[psDisplayStructData->eVariable].plVarValue;

   switch(psDisplayStructData->eVariable)
   {
      case keTreal:
         prefix[0] = ' ';prefix[1] = 'T';prefix[2] = 'r';prefix[3] = '=';

         if (bCelsiumOrKelvin)
         {
            sprintf(String, "%.2f", (mDeNormTemp(lValueToIndicator) - kCelsiumShift));
            sufix[0] = 0xDF/*'°'*/; sufix[1] = 'C';
         }
         else
         {
            sprintf(String, "%.2f", mDeNormTemp(lValueToIndicator));
            sufix[0] = ' '; sufix[1] = 'K';
         }

         break;

      case keTset:
         prefix[0] = ' ';prefix[1] = 'T';prefix[2] = 's';prefix[3] = '=';

         if (bCelsiumOrKelvin)
         {
            sprintf(String, "%.2f", (mDeNormTemp(lValueToIndicator) - kCelsiumShift));
            sufix[0] = 0xDF/*'°'*/; sufix[1] = 'C';
         }
         else
         {
            sprintf(String, "%.2f", mDeNormTemp(lValueToIndicator));
            sufix[0] = ' '; sufix[1] = 'K';
         }
         break;

      case keTcurSet:
         prefix[0] = ' ';prefix[1] = 'T';prefix[2] = 'c';prefix[3] = '=';
         if (bCelsiumOrKelvin)
         {
            sprintf(String, "%.2f", (mDeNormTemp(lValueToIndicator) - kCelsiumShift));
            sufix[0] = 0xDF/*'°'*/; sufix[1] = 'C';
         }
         else
         {
            sprintf(String, "%.2f", mDeNormTemp(lValueToIndicator));
            sufix[0] = ' '; sufix[1] = 'K';
         }
         break;

      case keDeltaT:
         prefix[0] = ' ';prefix[1] = 'D';prefix[2] = 'T';prefix[3] = '=';

         if (bCelsiumOrKelvin)
         {
            sprintf(String, "%.1f", mDeNormTemp(lValueToIndicator));
            sufix[0] = 0xDF/*'°'*/; sufix[1] = 'C';
         }
         else
         {
            sprintf(String, "%.1f", mDeNormTemp(lValueToIndicator));
            sufix[0] = ' '; sufix[1] = 'K';
         }
         break;

      case keDeltat:
         prefix[0] = ' ';prefix[1] = 'd';prefix[2] = 't';prefix[3] = '=';
         sufix[0] = ' '; sufix[1] = 's';
         sprintf(String, "%.1f", mDeNormTime(lValueToIndicator));
         break;

      case keKprop:
         prefix[0] = ' ';prefix[1] = 'K';prefix[2] = 'p';prefix[3] = '=';
         sufix[0] = ' '; sufix[1] = ' ';
         sprintf(String, "%d", lValueToIndicator);
         break;

//         case keKint:
//            pcPrefix = "Kin";
//            bPointShifted = FALSE;
//            break;

      case keKdiff:
         prefix[0] = ' ';prefix[1] = 'K';prefix[2] = 'd';prefix[3] = '=';
         sufix[0] = ' '; sufix[1] = ' ';
         sprintf(String, "%d", lValueToIndicator);
         break;


      case keUreal:
         prefix[0] = ' ';prefix[1] = 'U';prefix[2] = 'r';prefix[3] = '=';
         sufix[0] = ' '; sufix[1] = 'V';
         lValueToIndicator = getMicroVoltsADC(lValueToIndicator);
         sprintf(String, "%.5f", ((float)((int32_t)lValueToIndicator)/1e6));

         break;

      case keCLevel:
         prefix[0] = ' ';prefix[1] = 'L';prefix[2] = 'c';prefix[3] = '=';
         sufix[0] = ' '; sufix[1] = '%';
         sprintf(String, "%d", lValueToIndicator);
         break;

      default:
         break;
   }

   if (!bVariableOnly)
   {
      sprintf(StringOut,"%s%s%s",prefix,String,sufix);
   }
   else
   {
      sprintf(StringOut,"%s",String);
   }

   char* psWork = &StringOut[0];
   while ((*psWork != '\0') && (psWork < (&StringOut[20])))
   {
      *pOutString = *psWork;
      psWork++;
      pOutString++;
   }
   *pOutString = '\0';

   return;
}

/* --------------------------------------------------------------
 * GetStringOfEnterMode(): converts data into the string to be displayed in Enter mode
 * psDisplayStructData - pointer on data configuration
 * pOutString          - Returns the string in second parameter
 * -------------------------------------------------------------- */
void GetStringOfEnterMode(Display_Zone_struct* psDisplayStructData, char* pOutString)
{
   char StringOut[21] = {0};
   char prefix[5] = {0};
   char sufix[3] = {0};

   if (eKeyboardState == keDirectCommandEnterMode)
   {
      prefix[0] = '>'; prefix[1] = 0x2A; prefix[2] = '\0';
      sprintf(StringOut,"%s",sInputString.InputString);
   }
   else
   {
      switch(psDisplayStructData->eVariable)
      {
         case keTreal:
            prefix[0] = ' ';prefix[1] = 'T';prefix[2] = 'r';prefix[3] = '=';

            if (bCelsiumOrKelvin)
            {
               sufix[0] = 0xDF/*'°'*/; sufix[1] = 'C';
            }
            else
            {
               sufix[0] = ' '; sufix[1] = 'K';
            }

            break;

         case keTset:
            prefix[0] = ' ';prefix[1] = 'T';prefix[2] = 's';prefix[3] = '=';

            if (bCelsiumOrKelvin)
            {
               sufix[0] = 0xDF/*'°'*/; sufix[1] = 'C';
            }
            else
            {
               sufix[0] = ' '; sufix[1] = 'K';
            }
            break;

         case keTcurSet:
            prefix[0] = ' ';prefix[1] = 'T';prefix[2] = 'c';prefix[3] = '=';
            if (bCelsiumOrKelvin)
            {
               sufix[0] = 0xDF/*'°'*/; sufix[1] = 'C';
            }
            else
            {
               sufix[0] = ' '; sufix[1] = 'K';
            }
            break;

         case keDeltaT:
            prefix[0] = ' ';prefix[1] = 'D';prefix[2] = 'T';prefix[3] = '=';

            if (bCelsiumOrKelvin)
            {
               sufix[0] = 0xDF/*'°'*/; sufix[1] = 'C';
            }
            else
            {
               sufix[0] = ' '; sufix[1] = 'K';
            }
            break;

         case keDeltat:
            prefix[0] = ' ';prefix[1] = 'd';prefix[2] = 't';prefix[3] = '=';
            sufix[0] = ' '; sufix[1] = 's';
            break;

         case keKprop:
            prefix[0] = ' ';prefix[1] = 'K';prefix[2] = 'p';prefix[3] = '=';
            sufix[0] = ' '; sufix[1] = ' ';
            break;

   //         case keKint:
   //            pcPrefix = "Kin";
   //            bPointShifted = FALSE;
   //            break;

         case keKdiff:
            prefix[0] = ' ';prefix[1] = 'K';prefix[2] = 'd';prefix[3] = '=';
            sufix[0] = ' '; sufix[1] = ' ';
            break;


         case keUreal:
            prefix[0] = ' ';prefix[1] = 'U';prefix[2] = 'r';prefix[3] = '=';
            sufix[0] = ' '; sufix[1] = 'V';
            break;

         case keCLevel:
            prefix[0] = ' ';prefix[1] = 'L';prefix[2] = 'c';prefix[3] = '=';
            sufix[0] = ' '; sufix[1] = '%';
            break;

         default:
            break;
      }
   }

   sprintf(StringOut,"%s%s%s",prefix,sInputString.InputString,sufix);

   char* psWork = &StringOut[0];
   while ((*psWork != '\0') && (psWork < (&StringOut[20])))
   {
      *pOutString = *psWork;
      psWork++;
      pOutString++;
   }
   *pOutString = '\0';

   return;
}

/* --------------------------------------------------------------
 * void WriteFullString(char* pString, uint16_t iFullLength)
 * The procedure extends the string to the desired length
 * Initial length is to "\0" - end symbol
 * -------------------------------------------------------------- */
void WriteFullString(char* pString, uint16_t iFullLength)
{
   uint16_t iStringOutLength;

   iStringOutLength = strlen(pString);

#ifdef debugmode_str
   uint16_t iLocalWork;
   //printf("\r\nLength %s = %d\r\n", pString, iStringOutLength);
   //printf("Length desired %d\r\n", iFullLength);
   //printf("Symbol On Length: %c\r\n", *(pString + iStringOutLength-1));
   for (iLocalWork = 0; iLocalWork < iFullLength; iLocalWork++)
   {
      //printf("%hhx",*(pString + iLocalWork));//printf(";");
   }
   //printf("\r\n");
#endif // debugmode

   while (iStringOutLength < iFullLength-1)
   {
      *(pString + iStringOutLength) = ' ';
      iStringOutLength++;
   }

   *(pString + iStringOutLength) = '\0';

}

void ShowSensor(void)
{
   // Only necessary is to set this flag,
   // and the procedure knows what to do
   bLocalShowingSensorName = TRUE;
}

void ShowApplicationName(void)
{
   // Only necessary is to set this flag,
   // and the procedure knows what to do
   bLocalShowingApplicationName = TRUE;
}

void ShowUpdateError(void)
{
   // Only necessary is to set this flag,
   // and the procedure knows what to do
   bLocalShowingUpdateError = TRUE;
}

void K45_Exit(uint16_t iReason)
{
   switch (iReason)
   {

      case 0:
           // Bring system to idle mode (the command to executive plate)
           eSystemState = keIdle;
           // Stop display updating
           fSystemThreadControl.s.bInterfaceOn = FALSE;
           // clear it off
           LCDI2C_clear();
           GoodbyeShow();
           delay(1000);
           break;

      // ADC not found
      case 1:
         // ADC Problem
         printf("ADC error initialization\n\r");
         break;

      // ADC not found
      case 2:
         printf("Stopped by user\n\r");
         break;

      case 3:
           // Bring system to idle mode (the command to executive plate)
           eSystemState = keIdle;
           // Stop display updating
           fSystemThreadControl.s.bInterfaceOn = FALSE;
           // clear it off
           LCDI2C_clear();
           RebootShow();
           delay(1000);
           break;


      default:
         break;
   }

   exit(0);

}


#ifdef debugmode
/*
static void ShowStringBytes(char* cString)
{
   uint16_t iLocalWork;

   for (iLocalWork = 0; iLocalWork < kLongZoneLength; iLocalWork++)
   {
      //printf("%hhx",cString[iLocalWork]);//printf(";");
   }
   //printf("\r\n");
}
*/
#endif
