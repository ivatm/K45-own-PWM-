/*
 * globalvarheads.h
 *
 *  Created on: 20 груд. 2020 р.
 *      Author: Oliva
 */


#ifndef INC_GLOBALVARHEADS_H_
#define INC_GLOBALVARHEADS_H_

#include "user_types.h"
#include "modulSPI.h"
#include "defines.h"
#include "indicator.h"
/* ---------------------------------------------------
 * Measurements
 * --------------------------------------------------- */
extern sSensorVoltageDataStruct sSensorData;

/* ---------------------------------------------------
 * Temperatures
 * --------------------------------------------------- */
extern uint32_t lTemperaturePrevious; // measured temperature on previous program loop
extern uint32_t lTemperatureReal;     // Current measured temperature
extern int32_t slTemperatureSpeed;    // Normalized temperature increasing in 100 ms

extern uint32_t lTemperatureSet;      // Set temperature at this very moment
extern boolean bCelsiumOrKelvin;      // Set the temperature unit

/* ---------------------------------------------------
 * Sensor description
 * The Data are read from disc during initialization
 * --------------------------------------------------- */
extern uint16_t iTMH_Length;
extern uint16_t iTMH_Temperature_points[400];
extern uint16_t iTMH_Voltage_points[400];
extern char     SensorName[10];

/* ---------------------------------------------------
 * Regulating
 * --------------------------------------------------- */
// The Variables for non-volatile memory (saved on Disc)
// For PID regulator
extern uint32_t lKprop;
extern uint32_t lKint ;
extern uint32_t lKdiff;                // PID coefficients

//
extern uint16_t iHeaterEffect;         // Output effect of regulation heater
extern uint16_t iCoolerEffect;         // Output effect of regulation cooler
#ifdef VALVE_CONTROL
extern uint16_t iValveOutput;          // Output effect of regulation Valve
#endif

// SCANNING
extern uint32_t lTemperatureCurrentSet; //
extern uint32_t lDelta_t;
extern uint32_t lDelta_T;
#ifdef SCANNING
extern boolean            bScanOrSetMode;
extern boolean            bTempSetAchieved;    // Desired temperature set
#endif

/* ---------------------------------------------------
 * Common mode/state determining
 * --------------------------------------------------- */
// Received values from PowerModule
extern fStatusUnion       fPowerModulStatus;
extern uint16_t           HeaterVoltage;
extern uint16_t           CoolerVoltage;
extern uint16_t           ControlDiodeVoltage;
extern boolean            bUART_Active;        // controlling of Serial communication with PC
extern boolean            bADCCalibrationProcess;

/* ---------------------------------------------------
 * Display/ Keypad processing
 * --------------------------------------------------- */
extern Display_Zone_struct sDisplay_Zone[keZoneAmount];
extern const sVarIndicatorDescription_struct VarForIndication[keMaxVariableNum];
extern display_zone_enum eCurrentCursorZone;
extern KeyboardMode_enum eKeyboardState;
extern InputString_struct sInputString;

extern fSystemThreadControl_Union fSystemThreadControl;

extern boolean bCryoLevelMeasuring;
extern uint32_t LowLevelFrequency ;
extern uint32_t HighLevelFrequency;
extern uint32_t MeasuredFrequency;
extern uint32_t lCryoLevel;

#endif /* INC_GLOBALVARHEADS_H_ */
