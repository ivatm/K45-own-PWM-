/*
 * user_const.h
 *
 *  Created on: 19 Okt. 2014
 *      Author: Oliva
 */

#ifndef USER_CONST_H_
#define USER_CONST_H_

#define kHotTemperature  mNormTemp(35 + kCelsiumShift)
#define kWarmTemperature mNormTemp(5 + kCelsiumShift)

#define kTsetMax  mNormTemp((-20) + (kCelsiumShift))
#define kTsetMin  mNormTemp((-180) + (kCelsiumShift))

#define kCoefFrequency  256

#endif /* USER_CONST_H_ */
