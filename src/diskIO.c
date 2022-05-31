/************************************************************************
 * Unit for get/save data on disk
 * The file procedures are used for it
 *
 ************************************************************************/

#include "incs.h"
#include <string.h>
//#include <libio.h>
#include <stdio.h>
#include <ctype.h>
#include <unistd.h>
#include "globalvarheads.h"

// Local constants
static const char* str_Kprop = "#Kprop";
static const char* str_Kint  = "#Kint";
static const char* str_Kdiff = "#Kdiff";
static const char* str_CelsOrKel = "#CelsOrKel";
static const char* str_LevelMeasuringOn = "#LevelMeasuringOn";
static const char* str_LowLevelFrequency = "#LowLevelFrequency";
static const char* str_HighLevelFrequency = "#HighLevelFrequency";

void getSensCharacteristic(uint16_t* piPointNumber, uint16_t* piTemperature_Points, uint16_t* piVoltage_Points);
boolean getPIDcoefs(void);
uint16_t getDirectory(void);
uint16_t saveSettings(void);
void RestoreDefault(void);

/***********************************************************************************************************
 * The Procedure completes the arrays on pointers
 * piPointNumber - pointer on the number of points in table
 * pflTemperature_Points - pointer on temperature knots
 * pflTemperature_Points - pointer on voltage values
 ***********************************************************************************************************/
void getSensCharacteristic(uint16_t* piPointNumber, uint16_t* piTemperature_Points, uint16_t* piVoltage_Points)
{
   #define TMH_file_common "sensor.out"
   #define TMH_file "KV188.OUT"

   FILE* pFilePointer;
   char  str[100];
   char* ptr;

   char Localchar;

   uint32_t iPointQuantity;
   float flTemperature, flVoltage;
   unsigned int iLocalVar, iLocalVar2;

   pFilePointer = fopen(TMH_file_common,"r");

   if (pFilePointer == NULL)
   {
      pFilePointer = fopen(TMH_file,"r");
   }

   if (pFilePointer == NULL)
   {
      printf("Impossible to open file %s\r\n",TMH_file);
      getDirectory();
      return;
   }
   else
   {
      // read first string

      fgets(str, 50, pFilePointer);

      uint16_t ctr = 0;
      Localchar = '_';
      while (str[ctr])
      {
         if (isspace(Localchar) && isspace(str[ctr]))
         {
            str[ctr-1] = '\0';
            break;
         }
         else
         {
            Localchar=str[ctr];
            ctr++;
         }
      }

      // find last occurrence of space bar
      ptr = strrchr( str, ' ');

      // Sensor name

      iLocalVar2 = strlen(ptr);
      for (iLocalVar = 0;iLocalVar < iLocalVar2;iLocalVar++, ptr++)
      {
         SensorName[iLocalVar] = *ptr;
      }
      printf("Sensor name: %s\r\n",SensorName);

      iPointQuantity = 0;

      // 3 lines pass - nothing to save
      (void)fgets(str, 50, pFilePointer);
      (void)fgets(str, 50, pFilePointer);
      (void)fgets(str, 50, pFilePointer);

      // Get table data
      while( fscanf(pFilePointer, "%f %f %u\n"
                      , &flTemperature, &flVoltage, &iLocalVar) != EOF )
      {
         // If we get hear it means the string was read, otherwise the loop would be end

          //printf("%f %f %d \n", flTemperature, flVoltage, iLocalVar);
          iPointQuantity++;

          // get the read values
          *piTemperature_Points = (uint16_t)(flTemperature * 100.0);
          *piVoltage_Points = (uint16_t)(flVoltage * 10000.0);

         // Increase pointers for next point-values
          piTemperature_Points++;
          piVoltage_Points++;
      }

      *piPointNumber = iPointQuantity;

      fclose(pFilePointer);
   }
}

/***********************************************************************************************************
 * The Procedure completes the PID - coefficients
 * plKprop   -
 * plKintegr -
 * plKdiff   -
 ***********************************************************************************************************/
boolean getPIDcoefs(void)
{

   FILE* pFilePointer;
   char  str[100];
   uint32_t lLocal;
   uint32_t lLocalKoef;
   uint16_t iResult;

   pFilePointer = fopen(config_file,"r");
   iResult = 0;

   printf("Im here\n");

   if (pFilePointer == NULL)
   {
      printf("Impossible to open file %s\n",config_file);
      return(FALSE);
   }
   else
   {

      while (fscanf(pFilePointer, "%s", str ) != EOF)
      {

         if (str[0] == '#')
         {
            printf("And here Im trying: %s\n", str);
            lLocal = fscanf(pFilePointer, "%u", &lLocalKoef);
            if (lLocal > 0)
            {
               if (!strcmp(str, str_Kprop))
               {
                  lKprop = lLocalKoef;
                  printf("lKprop: %u\n", lKprop);
                  iResult++;
               }
               else if (!strcmp(str, str_Kint))
               {
                  lKint = lLocalKoef;
                  printf("lKint: %u\n", lKint);
                  iResult++;
               }
               else if (!strcmp(str, str_Kdiff))
               {
                  lKdiff = lLocalKoef;
                  printf("lKdiff: %u\n", lKdiff);
                  iResult++;
               }
               else if (!strcmp(str, str_CelsOrKel))
               {
                  bCelsiumOrKelvin = lLocalKoef;
                  printf("bCelsiumOrKelvin: %u\n", bCelsiumOrKelvin);
                  iResult++;
               }
               else if (!strcmp(str, str_LevelMeasuringOn))
               {
                  bCryoLevelMeasuring = lLocalKoef;
                  printf("bCryoLevelMeasuring: %u\n", bCryoLevelMeasuring);
                  iResult++;
               }
               else if (!strcmp(str, str_LowLevelFrequency))
               {
                  LowLevelFrequency = lLocalKoef;
                  printf("LowLevelFrequency: %u\n", LowLevelFrequency);
                  iResult++;
               }
               else if (!strcmp(str, str_HighLevelFrequency))
               {
                  HighLevelFrequency = lLocalKoef;
                  printf("HighLevelFrequency: %u\n", HighLevelFrequency);
                  iResult++;
               }
            }

         }
      }

      fclose(pFilePointer);
   }

   /// Exactly 3 coefficients should be read
   return(iResult == 3);
}


/*
 * Get directory
 *
 * */
uint16_t getDirectory(void)
{
   char cwd[20];
   if (getcwd(cwd, sizeof(cwd)) != NULL) {
       printf("Current working dir: %s\n", cwd);
   } else {
       perror("getcwd() error");
       return 1;
   }
   return 0;
}



/* ---------------------------------------------------------------------------------
 * Settings to be saved
 * --------------------------------------------------------------------------------- */
uint16_t saveSettings(void)
{

   FILE *fp;

   fp = fopen(config_file,"w");
   if (fp)
   {
      fprintf(fp, "// The configurations K43 controller\r\n");
      fprintf(fp, "\n");

      fprintf(fp, "\n%s\r\n", str_Kprop);
      fprintf(fp, "%u\r\n", lKprop);

#ifdef debugmode
      printf("Saving: Kprop %d\n", lKprop);
      printf("Saving: Kint %d\n", lKint);
      printf("Saving: Kdiff %d\n", lKdiff);

#endif

      fprintf(fp, "\n%s\r\n", str_Kint);
      fprintf(fp, "%u\r\n", lKint);

      fprintf(fp, "\n%s\r\n", str_Kdiff);
      fprintf(fp, "%u\r\n", lKdiff);

      fprintf(fp, "\n");
      fprintf(fp, "// Kelvin/Celsium mode\r\n");

      fprintf(fp, "\n%s\r\n", str_CelsOrKel);
      fprintf(fp, "%u\r\n", bCelsiumOrKelvin);

      fprintf(fp, "\n");
      fprintf(fp, "// Show CryoLevel mode\r\n");

      fprintf(fp, "\n%s\r\n", str_LevelMeasuringOn);
      fprintf(fp, "%u\r\n", bCryoLevelMeasuring);

      fprintf(fp, "\n%s\r\n", str_LowLevelFrequency);
      fprintf(fp, "%u\r\n", LowLevelFrequency);

      fprintf(fp, "\n%s\r\n", str_HighLevelFrequency);
      fprintf(fp, "%u\r\n", HighLevelFrequency);

      fclose(fp);
   }
   else
   {
      // Error
      return(1);
   }

   return(0);
}


void RestoreDefault(void)
{
   lKprop = Def_Kprop ;
   lKint  = Def_Kint  ;
   lKdiff = Def_Kdiff ;

   bCryoLevelMeasuring = Def_CryoLevelMeasuring;
   LowLevelFrequency   = Def_LowLevelFrequency;
   HighLevelFrequency  = Def_HighLevelFrequency;
   bCelsiumOrKelvin    = Def_CelsiumOrKelvin;


}
