/*
 * PWMDefines.h
 *
 *  Created on: 15 груд. 2021 р.
 *      Author: Oliva
 */

#ifndef INC_PWMDEFINES_H_
#define INC_PWMDEFINES_H_

#include "defines.h"

// PWM common frequency divider
// PWM base clock rate of 19.2MHz
#define PWM_BaseRate 19200000
#define PWM_divider  BCM2835_PWM_CLOCK_DIVIDER_16
#define kPWMAmount   2

// PWM output pins defines
#define cooler_PWM_CHANNEL 0
#define cooler_Frequence   10 // Hz
#define cooler_altFNumber  BCM2835_GPIO_FSEL_ALT0
#define cooler_PIN         RPI_BPLUS_GPIO_J8_32
#define cooler_Range       (PWM_BaseRate / (PWM_divider * cooler_Frequence))

#define heater_PWM_CHANNEL 1
#define heater_Frequence   40 // Hz
#define heater_altFNumber  BCM2835_GPIO_FSEL_ALT0
#define heater_PIN         RPI_BPLUS_GPIO_J8_33
#define heater_Range       (PWM_BaseRate / (PWM_divider * heater_Frequence))

typedef struct
{
   uint32_t lRange;
   uint16_t wOutFrequnce;
   boolean  bNativeOrNot;
   uint8_t  Channel;
   uint8_t  PIN;
   uint8_t  altFNumber;
} PWM_configuration;


#endif /* INC_PWMDEFINES_H_ */
