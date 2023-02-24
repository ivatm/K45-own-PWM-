/*
 * swprotection.c
 *
 * Example was taken here: https://raspberrypi.stackexchange.com/questions/49374/how-do-i-read-just-the-pis-serial-number-with-a-program-coded-in-c
 *  Created on: 24.02.2023
 *      Author: omivash
 */
#include <stdio.h>
#include <stdint.h>
#include <strings.h>


uint64_t getSerialID(void);


uint64_t getSerialID(void)
{
   static uint64_t serial = 0;

   FILE *filp;
   char buf[512];

   filp = fopen ("/proc/cpuinfo", "r");

   if (filp != NULL)
   {
      while (fgets(buf, sizeof(buf), filp) != NULL)
      {
         if (!strncasecmp("serial\t\t:", buf, 9))
         {
            sscanf(buf+9, "%Lx", &serial);
         }
      }

      fclose(filp);
   }
   return serial;
}

/* Using example
int main(int argc, char *argv[])
{
   printf("%Lx\n", getSerialID());
}
*/
