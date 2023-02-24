/*
 * lcd_struct.h
 *
 *  Created on: 13 03. 2021.
 *      Author: Oliva
 */

#ifndef INC_LCD_STRUCT_H_
#define INC_LCD_STRUCT_H_

#include "defines.h"
#include "modulSPI.h"


typedef enum
{
   keHeaterVoltage,
   keCoolerVoltage,
   keControlDiodeVoltage,
   kePowerPlateError,
   keErrorStateNumber
} eError_ToShow_enum;


/* --------------------------------------------------------------------------------
 * enumeration of data types to be displayed
 *
 * -------------------------------------------------------------------------------- */
typedef enum
{
   keScanningState = 0,
   keExecutiveModulErrors,
   keMeasuredValue,
   keSetValue,
   keDisplayTypeMax
} display_type_enum;

/* --------------------------------------------------------------------------------
 * enumeration of available zones to show displayed info
 *
 * -------------------------------------------------------------------------------- */
typedef enum
{
   keZone1 = 0,
   keZone2,
   keZone3,
   keZone4,
   keZone5,
   keZone6,
   keZoneAmount
} display_zone_enum;

// Some defines of zone names -----------------------------------------------------
#define eModeZone        (keZone1)
#define eErrorsZone      (keZone2)
#define eStepTempZone    (keZone3)
#define eStepTimeZone    (keZone4)
#define eTemperatureZone (keZone5)
#define eUserValue       (keZone6)


#endif /* INC_LCD_STRUCT_H_ */
