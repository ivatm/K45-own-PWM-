/*
 * user_types.h
 *
 *  Created on: 20 ����. 2020 �.
 *      Author: Oliva
 */

#ifndef INC_USER_TYPES_H_
#define INC_USER_TYPES_H_
#include "stdint.h"
#include "defines.h"

// Structures for voltage measurement data processing
typedef struct
{
   float fVoltageCurrent;     // float current voltage value
   float fVoltageLast;        // float last voltage value (measurement before)
   uint32_t lTimeDelta;       // the time period between two last measurements
   uint32_t lUlast;           // Integer last measured voltage value
   uint32_t lUcurrent;        // Integer voltage value
   boolean  bMeasurementReady;// voltage value received and may be used
} sSensorVoltageDataStruct;

/* All information needed for indicator*/
typedef struct
{
   uint32_t* plVarValue;    // Pointer of variables value
   uint32_t lVarStep;     // Pointer of variables step if possibly inputting
   uint32_t lVarMin;      // Pointer of variables low range limit
   uint32_t lVarMax;      // Pointer of variables high range limit
} sVarIndicatorDescription_struct;

typedef union
{
   struct
   {
      uint8_t bTemperatureAchieved  :1;
      uint8_t bScanningMode         :1;
      uint8_t bCelsiumOrKelvin      :1;
      uint8_t bHeatingOrCooling     :1;
      uint8_t bPowerPlateNotFound   :1;
      uint8_t bControlDiodeError    :1;
      uint8_t bCoolerError          :1;
      uint8_t bHeaterError          :1;

   } s;
   uint8_t cAllLeds;
} fIndicatorLeds_Union;

typedef union
{
   struct
   {
      uint8_t bADC_ThreadOn           :1;
      uint8_t bTemperatureRegulatorOn :1;
      uint8_t bInterfaceOn            :1;
      uint8_t bKeypad_ThreadOn        :1;
      uint8_t bPowerEquipmentOn       :1;
      uint8_t bUARTCommThreadOn       :1;
      uint8_t b6                      :1;
      uint8_t b7                      :1;

   } s;
   uint8_t cAllThreadStates;
} fSystemThreadControl_Union;


/* -------------------------------------------------------
 * The list of all buttons on the keypad
 * ------------------------------------------------------- */
typedef enum
{
   // According values were received experimental on the dedicated connection
   keNothingPressed  = 0,
   ke_F1             = 102,
   ke_F2             = 70,
   keSharp           = 35,
   keStar            = 42,
   ke_1              = 49,
   ke_2              = 50,
   ke_3              = 51,
   ke_4              = 52,
   ke_5              = 53,
   ke_6              = 54,
   ke_7              = 55,
   ke_8              = 56,
   ke_9              = 57,
   ke_0              = 48,
   ke_Up             = 94,
   ke_Down           = 95,
   ke_Left           = 60,
   ke_Right          = 62,
   ke_Enter          = 69,
   ke_Esc            = 99,
   ke_ButtonUnknown  = 255,
} Buttons_enum;

/* -------------------------------------------------------
 * The list of all commands from the keypad
 * ------------------------------------------------------- */
typedef enum
{
   keNotReadyCommand,
   keF1_Command,                     // Set Scan switch
   keF2_Command,                     // Stop temperature
   keLeftCommand,
   keRightCommand,
   keUpCommand,
   keDownCommand,
   keInputReadyCommand,
   keDirectReadyCommand,
   keCancelInputCommand,
   keTset_input             = 2,
   keTstep_input            = 4,
   ketime_step_input        = 5 ,
   keKprop_input            = 6 ,
   keKdiff_input            = 7 ,
   keSet_ScanSelect         = 9 ,
   keTemperatureUnitSwitch  = 11,
   keSaveConfigs            = 12,
   keADCCalibration         = 13,
   keShowSensor             = 14,
   // Configuration of Crioliqudes level meter
   keSetCryoLevelSwitch     = 15, // On/Off Cryolevel indication
   keSetFrequency_LowLevel  = 16, // On this command the measured frequency accords to 0 %
   keSetFrequency_HighLevel = 17, // On this command the measured frequency accords to 100 %
   keSetSensorNumber        = 18, // On this command the current Sensor gets changed

   // Commands for Sensor transmition
   keSendSensor     = 100, // Command for a line transmitting. Subcommand is the number. If 0 - start initialization
   keSensorComplete = 101,

   keNOP                    = 253, // Nothing to do - just life signal
   keRestoreDefault         = 254,
   keExit                   = 255,
   keUnknownCommand         = 0xFFFF,
} KeyPadCommands_enum;

// the possible length of a command
#define kComLength 10

/* -------------------------------------------------------
 * The list of states during a file reception
 * ToDo: not realized yet 
 * ------------------------------------------------------- */
typedef enum
{
   keSendCommandWait   = 0,
   keFileTransmitting  = 1,
   keFileCoping        = 2,
   keFileTransmitError = 3,
   keFileCopyError     = 4
} FileReceiveStates_enum;

#endif /* INC_USER_TYPES_H_ */
