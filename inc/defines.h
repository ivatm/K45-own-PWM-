/********************************************************/
/* File:    defines.h                                   */
/* Project: K45-temperature regulator project           */
/* Contains all definitions                             */
/*                                                      */
/********************************************************/


// Includs -----------------------------------------------------
#include "incs.h"

// Common defines
#define boolean uint16_t
#define true  ((uint16_t)1)
#define false ((uint16_t)0)
#define TRUE  (true)
#define FALSE (false)
// ---------------------------------------------------------------

#define kCoefMax  999
#define kCoefMin  0

#define kTemperatureMax    373
#define kTemperatureMin    (0)

#define kCelsiumShift     (273.15)
#define kStartTemperature (293)

#define kTimeMax        (300)   // Max time for setting
#define kTimeMin        (30)    // Min time for setting

// ADC defines -----------------------------------------
#define kSensorInput    (2)    // ADC input number to which the sensor connected

#define kfUmax               5.0         // MAX Voltage
#define kiUmax  mNormVoltage(1.25)        // MAX Voltage
#define kfUmin               0

#define mNormVoltage(x)   (uint16_t)(((float)((float)x - kfUmin) * 65535.0) / ((float)kfUmax - kfUmin))
#define mDeNormVoltage(x) ((float)       ((float)x * (kfUmax - kfUmin)) / 65535.0 + kfUmin)


// System timer identifications
#define kF0                  8000000     // CPU frequency
#define kMainLoopTimeDivider (kF0/256/10) // To provide MainLoop frequency 10 Hz
#define kTime2ManualDivider  (4 - 1)
#define kPWM2Divicer         (4 - 1)


// Amount of variables for indication

#define kMaxRegulatorEffect 0xFFFF

// Spline defines -----------------------------------------
#define kSplinePower   3 //степінь мого сплайна
#define kNumSplineKn   2*kSplinePower+1
#define kNumSplineCoef kSplinePower+1

#define kLengthKnots 23
#define kLengthCoefs 19

#define kMaxVarValue 65535
#define kMinVarValue 0

#define kNumberPointsMax 400

// -- Macros / Normalization -----------------------------
// Macroses for all coeffs
// k = 0...1(float) => 0...65535(uint16_t)
#define mFloatToIntCoef(x) (uint16_t)(65535.0 * (float)x)
#define mIntToFloatCoef(x) (float)((uint16_t)x / 65535.0)

// Macroses for temperature
// T = 0..373 => 0..65535
#define mFloatToIntTemp(x) (uint16_t)(65535.0 * ((float)x - kTemperatureMin) / (kTemperatureMax - kTemperatureMin))
#define mIntToFloatTemp(x) (float)((float)x * (kTemperatureMax - kTemperatureMin) / 65535.0 + kTemperatureMin)

#define mRound(x)             (0.5*((double)(x)>0)-0.5*((double)(x)<0))
#define mConvPercent(x)       (uint32_t)((x)*65535.0/100.0+mRound(x))
#define mConvSignPercent(x)   (uint32_t)((x)*32767.0/100.0+mRound(x))

#define mConvPercent_32(x)       (uint64_t)((x)*((float)0xFFFFFFFF)/100.0+mRound(x))
#define mConvSignPercent_32(x)   (int64_t)((x)*((float)0x7FFFFFFF)/100.0+mRound(x))

#define mConvPercentMaxReg(x) (uint32_t)((x)*(float)kMaxRegulatorEffect/100.0+mRound(x))
#define mAbs(x)               (((x) >= 0) ? (x) : -(x))                        // Betrag
#define mLimitWord(x)         (((x) >= (int32_t)mConvPercent(100)) ? (mConvPercent(100)) : ((x) < (int32_t)mConvPercent(0) ? (mConvPercent(0)) : (x))) // int32_t auf uint16_t begrenzen
#define mLimitUnsignWord(x)   (((x) >= (int32_t)mConvPercent(100)) ? (mConvPercent(100)) : (x))                                                           // uint32_t auf uint16_t begrenzen
#define mLimitSignWord(x)     (((x) >= mConvSignPercent(100)) ? (mConvSignPercent(100)) : ((x) < -mConvSignPercent(100) ? (-mConvSignPercent(100)) : (x)))     // int32_t auf signed int begrenzen

#define mLimitUnsignLong(x)   (((x) >= mConvPercent_32(100)) ? (mConvPercent_32(100)) : (x))                                                           // uint32_t auf uint16_t begrenzen
#define mLimitSignLong(x)     (((x) >= mConvSignPercent_32(100)) ? (mConvSignPercent_32(100)) : ((x) < -mConvSignPercent_32(100) ? (-mConvSignPercent_32(100)) : (x)))     // int32_t auf signed int begrenzen

// Macros for accumulator
#define mAccToRegEffect(x)    (((x) >= (int32_t)kMaxRegulatorEffect) ? (kMaxRegulatorEffect) : ((x) < ((int32_t)0) ? (0) : (x)))

//#define mNormCoef(x)   (uint16_t)(((float)x - kCoefMin) * 65535.0) /(kCoefMax - kCoefMin)
//#define mDeNormCoef(x) (float)((uint16_t)x * (kCoefMax - kCoefMin)) / 65535.0 + kCoefMin)

#define mNormTemp(x)   (uint16_t)((float)x*100)//(uint16_t)(((float)((float)x - kTemperatureMin) * 65535.0) / ((float)kTemperatureMax - kTemperatureMin))
#define mDeNormTemp(x) (float)x/100//((uint32_t)((float)x * 10 * (kTemperatureMax - kTemperatureMin)) / 65535.0 + kTemperatureMin)

// Time defines
/* Time conception:
 * - All time constants in processor are milliseconds;
 * - User inputs and sees time in seconds with one decade after decimal point 0.0 seconds. So the value is fixed point value ;
 * - to convert real value into processor, the multiplication 1000 is used;
 * - to convert processor value into real, the multiplication 0.001 is used;
 * - to convert user's inputed value into processor, the multiplication 100 is used;
 * - to convert processor value into user's, the multiplication 0.001 is used;
 * */

#define mTimeMiliseconds(x) (uint32_t)((x) * 1000)

#define mNormTime(x)        (uint32_t)(x * 1000)
#define mDeNormTime(x)      ((float)x / 1000.0)

//#define mConvIndicTime(x)   (uint32_t)(x * 100.0)
//#define mDeConvIndicTime(x) (float)(x / 100)

#define kADCService                 (1200)                             // ADC Calibration repetition 20 min
#define kIndicationTimeUpdate       (500)
#define kKeypadTimeUpdate           (500)
#define kReceptionCheck             (200)
#define kExecutiveModulComm         (400)
#define kTemperaturControlPeriod    (0.1)                              // Temperature update 100 ms
// Should be multiple to temperature control. Indeed we cannot expect faster changing than it is measured
#define kTemperatureScanUpdate      (1 * kTemperaturControlPeriod)

// Default values
#define Def_Kprop (10)
#define Def_Kint  (100)
#define Def_Kdiff (1)
#define Def_CryoLevelMeasuring (1)

#define Def_LowLevelFrequency  (671000uL)
#define Def_HighLevelFrequency (100000uL)
#define Def_HighLimitFrequence (1000000uL)// 1 MHz the highest possible frequency

#define Def_CelsiumOrKelvin    (0)

// File name for all configurations
#define config_file "K45_settings.dat"

