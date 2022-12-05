/*
 * config.h
 *
 *  Created on: 25 ρεπο. 2013
 *      Author: Oliva
 */ 

#ifndef CONFIG_H_
#define CONFIG_H_


/*debugging_ivatm*/
#define debugmode

/* -------------------------------------------------------
 * Raspberry native PWM
   -------------------------------------------------------*/
#define gdb_DEBUG_config (1)


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
