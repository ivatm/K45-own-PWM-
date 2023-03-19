/*
 * keypad.c : Raspberry PI (bcm2835) compatible switching keypad library
 *
 *  Created on: Oct 8, 2013
 *      Author: O. Ivashchenko
 */
#include <stdio.h>

#include "keypad.h"
#include "user_types.h"
#include "globalvarheads.h"

// extern procedures
extern void GetStringOfVariable(Display_Zone_struct* psDisplayStructData, char* pOutString, boolean bVariableOnly);
extern uint16_t saveSettings(void);
extern void ADC_service(void);
extern void ShowSensor(void);
extern void ShowApplicationName(void);
extern int BashApplicationUpdate(void);

extern void K45_Exit(uint16_t iReason);
extern void RestoreDefault(void);

// Local procedures
boolean PossibleInput(VarsForIndicator_enum eVariable);
void PerceiveInputValue(void);
void PerceiveInputCommand(void);

uint8_t get_key(void)
{
   //pirform a tiny tiny riset
   bcm2835_gpio_write(PIN_COL1, LOW);
   bcm2835_gpio_write(PIN_COL2, LOW);
   bcm2835_gpio_write(PIN_COL3, LOW);
   bcm2835_gpio_write(PIN_COL4, LOW);
   uint8_t i, j;
   for (i = 0; i < COLS; i++)
   {
      bcm2835_gpio_write(cols[i], HIGH);
      if (i - 1 == -1)
         bcm2835_gpio_write(cols[COLS - 1], LOW); //TODO: Avoid this chapuza
      else
         bcm2835_gpio_write(cols[i - 1], LOW);
      for (j = 0; j < ROWS; j++)
      {
         if (bcm2835_gpio_lev(row[j]))
            return map[j][i];
      }
   }
   return 0;
}

Buttons_enum wait_key(void)
{
   uint8_t Key;
   while (1)
   {
      bcm2835_delay(antibounce);
      Key = get_key();
      if (Key != 0)
         break;
   }

   if (Key)
   {
      // Check the pressed key
      switch((Buttons_enum)Key)
      {
        case ke_F1:
        case ke_F2:
        case keSharp:
        case keStar:
        case ke_1:
        case ke_2:
        case ke_3:
        case ke_4:
        case ke_5:
        case ke_6:
        case ke_7:
        case ke_8:
        case ke_9:
        case ke_0:
        case ke_Up:
        case ke_Down:
        case ke_Left:
        case ke_Right:
        case ke_Enter:
        case ke_Esc:
           return ((Buttons_enum)Key);
           break;

        default:
           return (ke_ButtonUnknown);
           break;
      }
   }
   else
   {
      // Nothing semantical
      return (keNothingPressed);
   }

   return ((Buttons_enum)Key);
}

uint8_t init_keypad(void)
{
 //Start bcm2835 library
 //  if (!bcm2835_init ())
 //     return 1;

//set columns as outputs (pulses)
   bcm2835_gpio_fsel (PIN_COL1, BCM2835_GPIO_FSEL_OUTP);
   bcm2835_gpio_fsel (PIN_COL2, BCM2835_GPIO_FSEL_OUTP);
   bcm2835_gpio_fsel (PIN_COL3, BCM2835_GPIO_FSEL_OUTP);
   bcm2835_gpio_fsel (PIN_COL4, BCM2835_GPIO_FSEL_OUTP);

   //set rows as inputs (pulses)
   bcm2835_gpio_fsel (PIN_ROW1, BCM2835_GPIO_FSEL_INPT);
   bcm2835_gpio_fsel (PIN_ROW2, BCM2835_GPIO_FSEL_INPT);
   bcm2835_gpio_fsel (PIN_ROW3, BCM2835_GPIO_FSEL_INPT);
   bcm2835_gpio_fsel (PIN_ROW4, BCM2835_GPIO_FSEL_INPT);
   bcm2835_gpio_fsel (PIN_ROW5, BCM2835_GPIO_FSEL_INPT);

   bcm2835_gpio_set_pud (PIN_ROW1, BCM2835_GPIO_PUD_DOWN);
   bcm2835_gpio_set_pud (PIN_ROW2, BCM2835_GPIO_PUD_DOWN);
   bcm2835_gpio_set_pud (PIN_ROW3, BCM2835_GPIO_PUD_DOWN);
   bcm2835_gpio_set_pud (PIN_ROW4, BCM2835_GPIO_PUD_DOWN);
   bcm2835_gpio_set_pud (PIN_ROW5, BCM2835_GPIO_PUD_DOWN);

#ifdef debugmode
   //printf("Keypad configured! \n");
#endif
   return 0;

}

/* -----------------------------------------------------------
 * KeyboardProcess()
 * Procedure to get and process a command from the keypad
 * ----------------------------------------------------------- */
void KeyboardProcess(void)
{
   // Command to be fulfilled
   KeyPadCommands_enum eCommand;
   // get the key already checked on its consistency
   Buttons_enum eKey = wait_key();

#ifdef debugmode
   //printf("Key pressed eKey = %d", eKey);
#endif// debugmode

   switch (eKey )
   {
      case ke_ButtonUnknown:
         // error
         break;

      case keNothingPressed:
         // NothingPressed
         break;

      default:
         // the key correctly received
         //printf("%d\n", eKey);

         eCommand = GetCommand(eKey);

         switch (eCommand)
         {
            case keNotReadyCommand:
               // Do not ready command operation -> print the symbol
               break;

            case keUnknownCommand:
               // Error on command interpretation
               break;

            default:
               // do the received command
               ProcessCommand(eCommand);
               break;
         }
        break;

   }

}

/* -----------------
 * Processing the key into the according Command
 * ----------------- */
KeyPadCommands_enum GetCommand(Buttons_enum eKey)
{
   KeyPadCommands_enum eResult;

   eResult = keUnknownCommand;

   switch (eKeyboardState)
   {
      case keWaitForCommand:
         switch(eKey)
         {
            case ke_F1:
               eResult = keF1_Command;
               break;
            case ke_F2:
               eResult = keF2_Command;
               break;
            case ke_Up:
               eResult = keUpCommand;
               break;
            case ke_Down:
               eResult = keDownCommand;
               break;
            case ke_Left:
               eResult = keLeftCommand;
               break;
            case ke_Right:
               eResult = keRightCommand;
               break;

            case ke_Enter:
               if (((eCurrentCursorZone == eUserValue)
                     || (eCurrentCursorZone == keZone3)
                     || (eCurrentCursorZone == keZone4))
                     &&
                     (PossibleInput(sDisplay_Zone[eCurrentCursorZone].eVariable)))
               {
                  // Get initial string
                  GetStringOfVariable(&sDisplay_Zone[eCurrentCursorZone], &sInputString.InputString[0], TRUE);
                  sInputString.cInputCursoPosition = 0;
                  sInputString.bReady = FALSE;

                  eKeyboardState = keEnterMode;
                  eResult = keNotReadyCommand;
               }
               break;

            case keStar:
               // Go to User zone
               eCurrentCursorZone = eUserValue;
               sInputString.InputString[0] = '\0';
               sInputString.cInputCursoPosition = 0;
               sInputString.bReady = FALSE;
               eKeyboardState = keDirectCommandEnterMode;
               break;

            default:
               eResult = keUnknownCommand;
               break;
         }
         break;

      case keEnterMode:

         if (eKey == ke_Enter)
         {
            sInputString.bReady = TRUE;
            eResult = keInputReadyCommand;
            eKeyboardState = keWaitForCommand;
         }
         else
         {
            if (eKey == ke_Esc)
            {
               sInputString.bReady = TRUE;
               eResult = keCancelInputCommand;
               eKeyboardState = keWaitForCommand;
            }
            else
            {
               char SymbolToAdd;

               // To avoid a warning
               SymbolToAdd = 0;

               switch (eKey)
               {
                  case (ke_1):
                     SymbolToAdd = '1';
                     break;

                  case (ke_2):
                     SymbolToAdd = '2';
                     break;

                  case (ke_3):
                     SymbolToAdd = '3';
                     break;

                  case (ke_4):
                     SymbolToAdd = '4';
                     break;

                  case (ke_5):
                     SymbolToAdd = '5';
                     break;

                  case (ke_6):
                     SymbolToAdd = '6';
                     break;

                  case (ke_7):
                     SymbolToAdd = '7';
                     break;

                  case (ke_8):
                     SymbolToAdd = '8';
                     break;

                  case (ke_9):
                     SymbolToAdd = '9';
                     break;

                  case (ke_0):
                     SymbolToAdd = '0';
                     break;

                  case (keStar):
                     SymbolToAdd = '.';
                     break;

                  case (keSharp):
                     if (bCelsiumOrKelvin)
                     {
                        SymbolToAdd = '-';
                     }
                     else
                     {
                        // nothing
                     }
                     break;

                  default:
                     // Wrong symbol - not ot be used
                     SymbolToAdd = 0;
                     break;
               }

               if (SymbolToAdd)
               {
                  sInputString.InputString[sInputString.cInputCursoPosition] = SymbolToAdd;
                  // Let say 6 the maximum input length
                  if (sInputString.cInputCursoPosition < 6)
                  {
                     sInputString.cInputCursoPosition++;
                  }
                  // Move the null symbol further
                  sInputString.InputString[sInputString.cInputCursoPosition] = '\0';
                  eResult = keNotReadyCommand;
               }
            }
         }
         break; // Enter mode

      case keDirectCommandEnterMode:
         if (eKey == keSharp)
         {
            sInputString.bReady = TRUE;
            eResult = keDirectReadyCommand;
            eKeyboardState = keWaitForCommand;
         }
         else
         {
            if (eKey == ke_Esc)
            {
               sInputString.bReady = TRUE;
               eResult = keCancelInputCommand;
               eKeyboardState = keWaitForCommand;
            }
            else
            {
               char SymbolToAdd;
               SymbolToAdd = 0;


               switch (eKey)
               {
                  case (ke_1):
                     SymbolToAdd = '1';
                     break;

                  case (ke_2):
                     SymbolToAdd = '2';
                     break;

                  case (ke_3):
                     SymbolToAdd = '3';
                     break;

                  case (ke_4):
                     SymbolToAdd = '4';
                     break;

                  case (ke_5):
                     SymbolToAdd = '5';
                     break;

                  case (ke_6):
                     SymbolToAdd = '6';
                     break;

                  case (ke_7):
                     SymbolToAdd = '7';
                     break;

                  case (ke_8):
                     SymbolToAdd = '8';
                     break;

                  case (ke_9):
                     SymbolToAdd = '9';
                     break;

                  case (ke_0):
                     SymbolToAdd = '0';
                     break;

                  default:
                     SymbolToAdd = 0;
                     break;
               }

               if (SymbolToAdd != 0)
               {
                  sInputString.InputString[sInputString.cInputCursoPosition] = SymbolToAdd;
                  // Let say 6 the maximum input length
                  if (sInputString.cInputCursoPosition < 6)
                  {
                     sInputString.cInputCursoPosition++;
                  }
                  // Move the null symbol further
                  sInputString.InputString[sInputString.cInputCursoPosition] = '\0';
                  eResult = keNotReadyCommand;
               }
            }
         }
         break;

      case keEnterInterlock:
         break;

      default:
         break;

   }

   return eResult;
}

void ProcessCommand(KeyPadCommands_enum eCommand)
{
   VarsForIndicator_enum eLokalVariable;

   switch(eCommand)
   {
      case keLeftCommand:

         if (eCurrentCursorZone == eUserValue)
         {
            eLokalVariable = sDisplay_Zone[eUserValue].eVariable;

            if (eLokalVariable == keTreal) // 0
            {
               eLokalVariable = keMaxVariableNum - 1;
            }
            else
            {
               eLokalVariable--;
            }

            // if the level is not measured to hide the according value
            if (!bCryoLevelMeasuring)
            {
               if (eLokalVariable == keCLevel)
               {
                  if (keCLevel > 0)
                  {
                     eLokalVariable--;
                  }
                  else
                  {
                     eLokalVariable = keMaxVariableNum - 1;
                  }
               }
            }

            //sDisplay_Zone[eUserValue].bOnOrOff = TRUE;
            sDisplay_Zone[eUserValue].eDataTypeDisplay = keSetValue;
            sDisplay_Zone[eUserValue].eVariable = eLokalVariable;
         }
         else
         {
            if ((eCurrentCursorZone == eStepTempZone)&&(sDisplay_Zone[eStepTimeZone].bOnOrOff))
            {
               eCurrentCursorZone = eStepTimeZone;
            }
            else
            {
               if ((eCurrentCursorZone == eStepTimeZone)&&(sDisplay_Zone[eStepTempZone].bOnOrOff))
               {
                  eCurrentCursorZone = eStepTempZone;
               }
               else
               {
                  // Nothing
               }
            }
         }
         break;

      case keRightCommand:
         if (eCurrentCursorZone == eUserValue)
         {
            eLokalVariable = sDisplay_Zone[eUserValue].eVariable;

            if (eLokalVariable == (keMaxVariableNum - 1))
            {
               eLokalVariable = keTreal;
            }
            else
            {
               eLokalVariable++;
            }

            // if the level is not measured to hide the according value
            if (!bCryoLevelMeasuring)
            {
               if (eLokalVariable == keCLevel)
               {
                  if (keCLevel < (keMaxVariableNum - 1))
                  {
                     eLokalVariable++;
                  }
                  else
                  {
                     eLokalVariable = 0;
                  }
               }
            }


            //sDisplay_Zone[eUserValue].bOnOrOff = TRUE;
            sDisplay_Zone[eUserValue].eDataTypeDisplay = keSetValue;
            sDisplay_Zone[eUserValue].eVariable = eLokalVariable;
         }
         else
         {
            if ((eCurrentCursorZone == eStepTempZone)&&(sDisplay_Zone[eStepTimeZone].bOnOrOff))
            {
               eCurrentCursorZone = eStepTimeZone;
            }
            else
            {
               if ((eCurrentCursorZone == eStepTimeZone)&&(sDisplay_Zone[eStepTempZone].bOnOrOff))
               {
                  eCurrentCursorZone = eStepTempZone;
               }
               else
               {
                  // Nothing
               }
            }
         }
         break;

      case keUpCommand:
      case keDownCommand:
         if ((eCurrentCursorZone == eUserValue)&&(sDisplay_Zone[eStepTempZone].bOnOrOff))
         {
            eCurrentCursorZone = eStepTempZone;
         }
         else
         {
            if (((eCurrentCursorZone == eStepTempZone) || (eCurrentCursorZone == eStepTimeZone))
                  &&(sDisplay_Zone[eUserValue].bOnOrOff))
            {
               eCurrentCursorZone = eUserValue;
            }
            else
            {
               // Nothing
            }
         }
         break;

      case keF1_Command:
         bScanOrSetMode = !bScanOrSetMode;
         break;

      case keF2_Command:
         lTemperatureSet = lTemperatureReal;
         break;

      case keInputReadyCommand:
         // Convert inputed string into according value
         PerceiveInputValue();
         break;

      case keDirectReadyCommand:
         // Convert inputed string into according value
         PerceiveInputCommand();
         break;

      default:
         // nothing to do
         break;

   }
}

/* ------------------------------------------------------------------------------------------------
 * void PerceiveInputValue(void)
 * Converting inputed string into the according value
 * ------------------------------------------------------------------------------------------------ */
void PerceiveInputValue(void)
{
   uint32_t lOutValue;
   boolean bToBeInput;
   float    fLocalValue;

   // Init to avoid warning
   lOutValue = 0;
   bToBeInput = FALSE;

   switch(sDisplay_Zone[eCurrentCursorZone].eVariable)
   {
      case keTreal:
         // Impossible to input
         break;

      case keTset:
         bToBeInput = TRUE;

         fLocalValue = atof(sInputString.InputString);
         if (bCelsiumOrKelvin)
         {
            fLocalValue += kCelsiumShift;
         }
         else
         {
            // Nothing
         }
         lOutValue = mNormTemp(fLocalValue);
         break;

      case keTcurSet:
         // Nothing
         break;

      case keDeltaT:
         bToBeInput = TRUE;
         fLocalValue = atof(sInputString.InputString);
         lOutValue = mNormTemp(fLocalValue);
         break;

      case keDeltat:
         bToBeInput = TRUE;
         fLocalValue = atof(sInputString.InputString);
         lOutValue = mNormTime(fLocalValue);
         break;

      case keKprop:

//         case keKint:
      case keKdiff:
         bToBeInput = TRUE;
         lOutValue = atol(sInputString.InputString);
         break;

      case keUreal:
         // Nothing
         break;

      case keCLevel:
         // Nothing
         break;

      default:
         break;
   }

   if (bToBeInput)
   {
      // Limiting range
      if (lOutValue > VarForIndication[sDisplay_Zone[eCurrentCursorZone].eVariable].lVarMax)
      {
         lOutValue = VarForIndication[sDisplay_Zone[eCurrentCursorZone].eVariable].lVarMax;
      }
      else
      {
         if (lOutValue < VarForIndication[sDisplay_Zone[eCurrentCursorZone].eVariable].lVarMin)
         {
            lOutValue = VarForIndication[sDisplay_Zone[eCurrentCursorZone].eVariable].lVarMin;
         }
      }

      // Get it
      *VarForIndication[sDisplay_Zone[eCurrentCursorZone].eVariable].plVarValue = lOutValue;
   }
}

boolean PossibleInput(VarsForIndicator_enum eVariable)
{
   if ((eVariable == keTset)
        || (eVariable == keDeltaT)
        || (eVariable == keDeltat)
        || (eVariable == keKprop)
        || (eVariable == keKdiff))
   {
      return (TRUE);
   }
   else
   {
      return (FALSE);
   }

}

/* ------------------------------------------------------------------------------------------------
 * void PerceiveInputCommand(void)
 * Converting inputed string into the according value
 * ------------------------------------------------------------------------------------------------ */
void PerceiveInputCommand(void)
{
   uint16_t wCommandNumber;

   wCommandNumber = atol(sInputString.InputString);

   switch (wCommandNumber)
   {
      case keTemperatureUnitSwitch:
         bCelsiumOrKelvin = !bCelsiumOrKelvin;
         break;

      case keExit:
         K45_Exit(0);
         break;

      case keSaveConfigs:
         saveSettings();
         break;

      case keADCCalibration:
         ADC_service();
         break;

      case keShowSensor:
         ShowSensor();
         break;

      case keShowApplication:
         ShowApplicationName();
         break;

      case keUpdateApplication:
         BashApplicationUpdate();
         break;

      case keSetCryoLevelSwitch:
         bCryoLevelMeasuring = !bCryoLevelMeasuring;
         break;

      case keSetFrequency_LowLevel:
         // only if measured frequency more then high level
         if (MeasuredFrequency > HighLevelFrequency)
         {
            LowLevelFrequency = MeasuredFrequency;
         }
         else
         {
            // otherwise both levels equal
            LowLevelFrequency = HighLevelFrequency;
         }
         break;

      case keSetFrequency_HighLevel:
         if (MeasuredFrequency < HighLevelFrequency)
         {
            HighLevelFrequency = MeasuredFrequency;
         }
         else
         {
            HighLevelFrequency = LowLevelFrequency;
         }
         break;

      case keSetSensorNumber:
         if (lSensorNumber >=1 )
         {
            lSensorNumber = 0;
         }
         else
         {
            lSensorNumber++;
         }
         saveSettings();
         break;

      case keSendSensor:
         break;

      case keSensorComplete:
         break;

      case keRestoreDefault:
         RestoreDefault();
         break;

      default:
         // unknown
         break;

   }
}

