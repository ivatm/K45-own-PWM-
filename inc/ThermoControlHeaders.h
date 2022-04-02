/*
 * ThermoControlHeaders.h
 *
 *  Created on: 15 лист. 2020 р.
 *      Author: Oliva
 */

#ifndef INC_THERMOCONTROLHEADERS_H_
#define INC_THERMOCONTROLHEADERS_H_

/***************************************************************/
/*   File: variablesini.h                                      */
/*   Project: KXX                                              */
/*   File for initialization of variables                      */
/*                                                             */
/***************************************************************/

// Global Includs
#include "stdint.h"
// Includs -----------------------------------------------------
//#include "incs.h"
#include "defines.h"

// state of system work
typedef enum
{
   keStable = 0,                      // Setting the caught current temperature
   keSetting,                         // Setting required temperature inputed from keyboard
   keScanning,                        //
   keIdle                             // the regulating is disabled for some reason
} SystemStateType;



// -- Global variables  -----------------------

extern uint16_t  iWorkTemperature;      // Work temperature
extern SystemStateType eSystemState;



#endif /* INC_THERMOCONTROLHEADERS_H_ */
