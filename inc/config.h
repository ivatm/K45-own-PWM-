/*
 * config.h
 *
 *  Created on: 25. 03. 2013
 *      Author: Oliva
 */

#ifndef CONFIG_H_
#define CONFIG_H_

// if defined, Processor ID should be considered
#define PROTECTION
#ifdef PROTECTION
// Necessary (requisite) HW_ID
#define HW_ID     0xceef6b49
#endif
/*debugging_ivatm*/
//#define debugmode

/* -------------------------------------------------------
 * Raspberry native PWM
   -------------------------------------------------------*/
#define gdb_DEBUG_config (0)


/* -------------------------------------------------------
 * Definition of interpolation type
   -------------------------------------------------------*/
#define LINEAR_INTERPOLATION

/* -------------------------------------------------------
 * Definition of sensor data storage mode:
 * - STEP_CONST
   -------------------------------------------------------*/
#define STEP_CONST

/* -------------------------------------------------------
 * The mode of even steady uniform temperature scanning
   -------------------------------------------------------*/
#define SCANNING

/* -------------------------------------------------------
 * Valve is also needed
   -------------------------------------------------------*/
#define VALVE_CONTROL

/* -------------------------------------------------------
 * Raspberry native PWM
   -------------------------------------------------------*/
#define RASPBERRY_PWM

// Local defines
//#define NO_CONTROL_LEVELS


#endif /* CONFIG_H_ */
