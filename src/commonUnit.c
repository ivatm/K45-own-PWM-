/*
 * commonUnit.c
 *
 *  Created on: 28 лист. 2020 р.
 *      Author: Oliva
 */

#include <time.h>
#include <stdint.h>

// global procedures -----------------------------------------------------------
uint_fast32_t millis(void);
uint_fast32_t appMillis(void);
uint_fast32_t deltaMilis(struct timespec* specStatic);

/*
 * The procedure returns the
 * */
uint_fast32_t millis(void)
{
    long            ms; // Milliseconds
    time_t          s;  // Seconds
    struct timespec spec;

    clock_gettime(CLOCK_MONOTONIC, &spec);

    s  = spec.tv_sec;
    // ms = round(spec.tv_nsec / 1.0e6); // Convert nanoseconds to milliseconds
    ms = (spec.tv_nsec / 1.0e6); // Convert nanoseconds to milliseconds
    if (ms > 999)
    {
        s++;
        ms = 0;
    }

    return (ms);
}

/*
 * IvA: Application millis
 * The procedure returns the milliseconds after boot
 * */
uint_fast32_t appMillis(void)
{
    long            ms; // Milliseconds
    time_t          s;  // Seconds
    struct timespec spec;

    clock_gettime(CLOCK_MONOTONIC, &spec);

    s  = spec.tv_sec;
    // ms = round(spec.tv_nsec / 1.0e6); // Convert nanoseconds to milliseconds
    ms = (spec.tv_nsec / 1.0e6) + 1000 * s; // Convert nanoseconds to milliseconds

    return (ms);
}

/*
 * IvA:  deltaMilis
 * The procedure returns the milliseconds after last its call
 * */
uint_fast32_t deltaMilis(struct timespec* specStatic)
{
//   static struct timespec spec;
   struct timespec specLocal;
   uint64_t llWorkLocal;
   uint32_t lMilliSeconds;


   clock_gettime(CLOCK_MONOTONIC, &specLocal);
   llWorkLocal = 1000000000*(specLocal.tv_sec - specStatic->tv_sec) + (specLocal.tv_nsec - specStatic->tv_nsec);
   lMilliSeconds = llWorkLocal / 1000000;

   *specStatic = specLocal;

   return(lMilliSeconds);

}
