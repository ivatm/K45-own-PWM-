/********************************************************/
/* File: indicator.h                                    */
/* Project: Kxx-Medic regulator project                 */
/* Types for correct indicator                          */
/*                                                      */
/********************************************************/

#include "user_types.h"
#include "lcd_struct.h"

// Defines
#define IND_DATA_PIN   5
#define IND_CLOCK_PIN  6
#define IND_STROBE_PIN 13

#define kShortZoneLength      11
#define kLongZoneLength   21

// state of keyboard
typedef enum
{
   keWaitForCommand = 0,             // In the state every button is considered as a command
   keEnterMode,                      // Enter Mode - a long command expected
   keDirectCommandEnterMode,         // Directly number of command entering in format *123#
   keEnterInterlock,                 // Enter pause to prevent repeating enter mode
} KeyboardMode_enum;


/* All possible keys              */
typedef enum
{
   keKey0_Treal            = 0x01,  // T real
   keKey1_Tset             = 0x02,  // T set
   keKey2_DeltaTemperature = 0x04,  // Delta Temperature
   keKey3_Delta_time       = 0x08,  // Delta time
   keKey4_Shift            = 0x10,  // K prop
   keKey5_Enter            = 0x20,  // K integral
   keKey6_Down             = 0x40,  // Down
   keKey7_Up               = 0x80,  // Up
// Special keys
   keKey_Exit              = 0x17, //Low 4 buttons for exit
   keUnknown
} KeyboardKeys_enum;

/* List of Leds                   */
typedef enum
{
   kelight1,
   kelight2,
   kelight3,
   kelight4,
   kelight5,
   kelight6,
   kelight7,
   kelight8
} IndicationLeds_enum;

typedef enum
{
   keTreal,      //   lTemperatureReal,
   keTset,       //   lTemperatureSet,
   keTcurSet,    //   lTemperatureCurrentSet,
   keDeltaT,     //   lDelta_T,
   keDeltat,     //   lDelta_t,
   keKprop,      //   lKprop,
//   keKint,       //   lKint,
   keKdiff,      //   lKdiff,
   keUreal,       //  sSensorData.iUcurrent,
   keMaxVariableNum
} VarsForIndicator_enum;

/* --------------------------------------------------------------------------------
 * Structure for description of some indication in some zone
 *
 * -------------------------------------------------------------------------------- */
typedef struct
{
   VarsForIndicator_enum eVariable;         // Number of structure in variable list
   boolean bOnOrOff;                        // Activation / Inactivation
   display_type_enum eDataTypeDisplay;      // type description
   char    cDisplayedString[kLongZoneLength];
} Display_Zone_struct;

/* --------------------------------------------------------------------------------
 * Structure for description of current input string
 * -------------------------------------------------------------------------------- */
typedef struct
{
   boolean bReady;            // Activation / Inactivation
   char    InputString[10];      // type description
   uint8_t cInputCursoPosition;
} InputString_struct;
