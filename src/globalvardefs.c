/*
 * globalvardefs.c
 *
 *  Created on: 20 груд. 2020 р.
 *      Author: Oliva
 */

#include "user_types.h"
#include "defines.h"
#include "modulSPI.h"

/* ---------------------------------------------------
 * Measurements
 * --------------------------------------------------- */
sSensorVoltageDataStruct sSensorData;

/* ---------------------------------------------------
 * Temperatures
 * --------------------------------------------------- */
uint32_t lTemperaturePrevious = mNormTemp(kStartTemperature); // measured temperature on previous program loop
uint32_t lTemperatureReal     = mNormTemp(kStartTemperature); // Current measured temperature
int32_t slTemperatureSpeed    = mNormTemp(kStartTemperature); // Normalized temperature increasing in 100 ms

uint32_t lTemperatureSet      = mNormTemp(kStartTemperature); // Set temperature
boolean bCelsiumOrKelvin      = TRUE;


/* ---------------------------------------------------
 * Sensor description
 * The Data are read from disc during initialization
 * --------------------------------------------------- */
uint16_t iTMH_Length;
uint16_t iTMH_Temperature_points[400];
uint16_t iTMH_Voltage_points[400];
char     SensorName[10];


/* ---------------------------------------------------
 * Regulating
 * --------------------------------------------------- */
// The Variables for non-volatile memory (saved on Disc)
// For PID regulator
uint32_t lKprop = 10,
         lKint  = 100,
         lKdiff = 1;                               // PID coefficients

//
uint16_t iHeaterEffect = 0;         // Output effect of regulation heater
uint16_t iCoolerEffect = 0;         // Output effect of regulation cooler
#ifdef VALVE_CONTROL
uint16_t iValveOutput = 0;          // Output effect of regulation Valve
#endif

// SCANNING
uint32_t lTemperatureCurrentSet = mNormTemp(kStartTemperature); // Set temperature at this very moment
uint32_t lDelta_t = mNormTime(1);
uint32_t lDelta_T = mNormTemp(1.5);
#ifdef SCANNING
boolean            bScanOrSetMode;      // Scanning flag
boolean            bTempSetAchieved;    // Desired temperature set
#endif

/* ---------------------------------------------------
 * Common mode/state determining
 * --------------------------------------------------- */
// Received values from PowerModule
fStatusUnion       fPowerModulStatus;
uint16_t           HeaterVoltage,
                   CoolerVoltage,
                   ControlDiodeVoltage;

boolean            bUART_Active;           // controlling of Serial communication with PC
boolean            bADCCalibrationProcess; // Flag of calibration process
//boolean            bShowSensorName;        // Flag for Sensors name indication

// All threads controlling
fSystemThreadControl_Union fSystemThreadControl;
