/*
 * nativePWM.c
 *
 *  Created on: 5 груд. 2021 р.
 *      Author: Oliva
 */


// https://stackoverflow.com/questions/58949250/raspberry-pi-3b-using-bcm2835-lib-to-generate-hareware-pwm-send-specific-numbe
// https://www.airspayce.com/mikem/bcm2835/pwm_8c-example.html

#include <bcm2835.h>
#include <stdio.h>
#include "PWMdefines.h"

const PWM_configuration PWMConfs[kPWMAmount] =
{
      // Cooler
      {
            cooler_Range,
            cooler_Frequence,
            TRUE,
            cooler_PWM_CHANNEL,
            cooler_PIN,
            cooler_altFNumber
      },

      // heater
      {
            heater_Range,
            heater_Frequence,
            TRUE,
            heater_PWM_CHANNEL,
            heater_PIN,
            heater_altFNumber
      }
};

int PWM_init(void);
void PWM_set(uint16_t wChannelNumber, uint16_t wValue);

/* ---------------------------------------------------------------------------------------------------
 * void PWM_init(void)
 * All native PWM configure
 * --------------------------------------------------------------------------------------------------- */
int PWM_init(void)
{
   boolean Result;

   Result = TRUE;
   // Clock divider is set to
   bcm2835_pwm_set_clock(PWM_divider);

   for (uint16_t i = 0; i < kPWMAmount; i++)
   {
      if (PWMConfs[i].bNativeOrNot)
      {
         // Set the output pin to Alt Fun, to allow PWM channel 0 to be output there
         bcm2835_gpio_fsel(PWMConfs[i].PIN, PWMConfs[i].altFNumber);

         bcm2835_pwm_set_mode(PWMConfs[i].Channel, 1, 1);   //CTL reg

         // Range calculation
         bcm2835_pwm_set_range(PWMConfs[i].Channel, PWMConfs[i].lRange); //RNG1/2 reg
      }
      else
      {
         Result = FALSE;
      }
   }

   return(Result);
}

/* ---------------------------------------------------------------------------------------------------
 * void PWM_set(uint16_t wChannelNumber, uint16_t Value)
 * par in: wChannelNumber - PWM of the Raspberry
 *         Value          - the value in percent
 * Set the desired value to the desired channel
 * --------------------------------------------------------------------------------------------------- */
void PWM_set(uint16_t wChannelNumber, uint16_t wValue)
{
   uint32_t lData;

   lData = (uint64_t)(PWMConfs[wChannelNumber].lRange * wValue) / 100;

   bcm2835_pwm_set_data(PWMConfs[wChannelNumber].Channel, lData);
}
