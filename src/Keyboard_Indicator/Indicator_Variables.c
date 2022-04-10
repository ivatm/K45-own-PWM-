/*
 * Indicator_Variables.c
 *
 *  Created on: 20 груд. 2020 р.
 *      Author: Oliva
 */

#include "globalvarheads.h"


// Indicator vars ----------------------------------------
VarsForIndicator_enum eVarNumber = keTreal;  // Number of current variable to indicate

// Variable table ---------------
const sVarIndicatorDescription_struct VarForIndication[keMaxVariableNum] =
{   // Value                     Step                               Minimum                            Maximum
      { &lTemperatureReal,       0,                                 0,                                 0                                    },
      { &lTemperatureSet,        mNormTemp(0.1),                    mNormTemp(4.2),                    mNormTemp(kTemperatureMax)           },
      { &lTemperatureCurrentSet, 0,                                 0,                                 0                                    },
      { &lDelta_T,               mNormTemp(0.1),                    mNormTemp(0),                      mNormTemp(50)                        },
      { &lDelta_t,               mNormTime(kTemperatureScanUpdate), mNormTime(kTemperatureScanUpdate), mNormTime(150*kTemperatureScanUpdate) },
      { &lKprop,                 1,                                 kCoefMin,                                 kCoefMax                                  },
/*-*/ //     { &lKint,                  1,                          kCoefMin,                                 kCoefMax                                  },
      { &lKdiff,                 1,                                 kCoefMin,                                 kCoefMax                                  },
      { &sSensorData.lUcurrent,  0,                                 0,                                 0                                    },
      { &lCryoLevel,             0,                                 0,                                 100                                    },

};

// Old conception --------------------------------------------------------------
uint8_t cChangingSpeed = 1;    // Speed of variable setting


boolean bPointShifted = FALSE;

fIndicatorLeds_Union fIndicatorLeds;

// End of old conception -------------------------------------------------------

// Keypad Variables ------------------------------------------------------------

/* -----------------------------------
 * Current zone with cursor (focus)
 * If the mode allows the ARROW buttons are changing the current zone
 * ----------------------------------- */
display_zone_enum eCurrentCursorZone = eUserValue;

KeyboardMode_enum eKeyboardState = keWaitForCommand;
