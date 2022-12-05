/************************************************************************
 * Unit for get/save data on disk
 * The file procedures are used for it
 *
 ************************************************************************/

#include "incs.h"
#include <string.h>
//#include <libio.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include "globalvarheads.h"
#include <errno.h>

// Local constants
static const char* str_Kprop = "#Kprop";
static const char* str_Kint  = "#Kint";
static const char* str_Kdiff = "#Kdiff";
static const char* str_CelsOrKel = "#CelsOrKel";
static const char* str_LevelMeasuringOn = "#LevelMeasuringOn";
static const char* str_LowLevelFrequency = "#LowLevelFrequency";
static const char* str_HighLevelFrequency = "#HighLevelFrequency";

// Global procedures ------------------------
void getSensCharacteristic(uint16_t* piPointNumber, uint16_t* piTemperature_Points, uint16_t* piVoltage_Points);
boolean getPIDcoefs(void);
uint16_t getDirectory(void);
uint16_t saveSettings(void);
void RestoreDefault(void);

boolean appendSensReceivedLine(uint16_t ReceivedLineNumber, char* pcLine);
int completeSensDataFile(void);
int copyFile(void);
int BashCopyFile(void);

/***********************************************************************************************************
 * The Procedure completes the arrays on pointers
 * piPointNumber - pointer on the number of points in table
 * pflTemperature_Points - pointer on temperature knots
 * pflTemperature_Points - pointer on voltage values
 ***********************************************************************************************************/
void getSensCharacteristic(uint16_t* piPointNumber, uint16_t* piTemperature_Points, uint16_t* piVoltage_Points)
{

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
   uint32_t lLocalWorkValue;
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
            lLocal = fscanf(pFilePointer, "%u", &lLocalWorkValue);
            if (lLocal > 0)
            {
               if (!strcmp(str, str_Kprop))
               {
                  lKprop = lLocalWorkValue;
                  printf("lKprop: %u\n", lKprop);
                  iResult++;
               }
               else if (!strcmp(str, str_Kint))
               {
                  lKint = lLocalWorkValue;
                  printf("lKint: %u\n", lKint);
                  iResult++;
               }
               else if (!strcmp(str, str_Kdiff))
               {
                  lKdiff = lLocalWorkValue;
                  printf("lKdiff: %u\n", lKdiff);
                  iResult++;
               }
               else if (!strcmp(str, str_CelsOrKel))
               {
                  bCelsiumOrKelvin = lLocalWorkValue;
                  printf("bCelsiumOrKelvin: %u\n", bCelsiumOrKelvin);
                  iResult++;
               }
               else if (!strcmp(str, str_LevelMeasuringOn))
               {
                  bCryoLevelMeasuring = lLocalWorkValue;
                  printf("bCryoLevelMeasuring: %u\n", bCryoLevelMeasuring);
                  iResult++;
               }
               else if (!strcmp(str, str_LowLevelFrequency))
               {
                  LowLevelFrequency = lLocalWorkValue;
                  printf("LowLevelFrequency: %u\n", LowLevelFrequency);
                  iResult++;
               }
               else if (!strcmp(str, str_HighLevelFrequency))
               {
                  HighLevelFrequency = lLocalWorkValue;
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

// --------------------------------------------------------------------------------------------------------------

/***********************************************************************************************************
 * The Function adds line to the temporary file in the ram-disk.
 * (Store the files at /dev/shm/, it's a tmpfs based ramdisk already.)
 * pcLine                 - the string to be appended
 * Return                 - It checks weather received line is new one in text (the number is 1 more then the length).
 *                          If not or any other error return if FALSE
 * if wCharackteristicNumber = 0 - the file will be newly created
 ***********************************************************************************************************/
boolean appendSensReceivedLine(uint16_t ReceivedLineNumber, char* pcLine)
{
   #define LineLength 81

   static uint16_t wLineIndex;
   FILE *fp;
   boolean bResult;

   bResult = TRUE;

   // First line
   if (ReceivedLineNumber == 0)
   {
	   fModulStatusByte2.sStatus.bSensorDataFileReceived = FALSE;

      fp = fopen(Received_TMH_file,"w");
      wLineIndex = 0;

      while (*pcLine != '\n')
      {
         fputc(*pcLine, fp);
         pcLine++;
      }
      fputc('\n', fp);
      fclose(fp);
   } // Next line received
   else if (wLineIndex+1 == ReceivedLineNumber)
   {
      fp = fopen(Received_TMH_file,"a");
      while (*pcLine != '\n')
      {
         fputc(*pcLine, fp);
         pcLine++;
      }
      fputc('\n', fp);

      wLineIndex++;

      fclose(fp);
   }
   else
   {
      bResult = FALSE;
   }

   return(bResult);
}

/***********************************************************************************************************
 ***********************************************************************************************************/
int completeSensDataFile(void)
{
  // int Result;

   printf("File received");

   return(0);
}

/***********************************************************************************************************
 * int copyFile(void)
 * The procedure copies received file "Received_TMH_file" in "TMH_file_common"
 * The realized method:
 * - direct symbol by symbol coping in C-code
 ***********************************************************************************************************/
int copyFile(void)
{
   #define  kLengthOneTransmition 80
   #define  kFileMaxLength 100000

   static uint16_t wTotalCounter;
   uint16_t wLocalCounter;

   int result;
   FILE *fSrc;
   FILE *fDst;
   int sym;

   result = 0;

   if (!fModulStatusByte2.sStatus.bSensorDataFileReceived)
   {
      wTotalCounter = 0;
      return(0);
   }
   else
   {
     fSrc = fopen(Received_TMH_file, "r");

     if (wTotalCounter == 0)
     {
        fDst = fopen(TMH_file_common, "w");
     }
     else
     {
        fDst = fopen(TMH_file_common, "a");
     }
   }

   if ((fSrc != NULL) && (fDst != NULL))
   {
      wLocalCounter = fseek(fSrc, wTotalCounter, 0);

      if (wLocalCounter == 0)
      {
         sym = fgetc(fSrc);

         while ((sym != EOF)
                &&(wLocalCounter < kLengthOneTransmition))
         {
            wLocalCounter++;

            fputc((char)sym, fDst);
            printf("%c", (char)sym);
            sym = fgetc(fSrc);
         }

         if (wTotalCounter < kFileMaxLength)
         { // Update the total read symbols
            wTotalCounter = wTotalCounter + wLocalCounter;
         }
         else
         {
            // Too long file
            result = 1;
         }
      }
      else
      {
         // seeking is not successful
         result = 1;
      }

   }
   else
   {
      // opens not successful
      result = 1;
   }

   if (fSrc != NULL)
   {
      fclose(fSrc);
   }

   if (fDst != NULL)
   {
      fclose(fDst);
   }

   if ((sym == EOF)       // The coping ended successfully
        || (result > 0))  // some problems
   {
      // anyway the copying ends
      fModulStatusByte2.sStatus.bSensorDataFileReceived = FALSE;
   }

   return(result);
}

/***********************************************************************************************************
 * int copyFile(void)
 * The procedure copies received file "Received_TMH_file" in "TMH_file_common"
 * The realized method - using the bush-script
 ***********************************************************************************************************/
int BashCopyFile(void)
{
    int Result;
    char* WorkStr = "cp ..//..";
    char* WorkStr2 = " ";
    char  BashCommand[50]; // seems to be enough

    Result = 0;

    if (!fModulStatusByte2.sStatus.bSensorDataFileReceived)
    {
       Result = 0;
    }
    else
    {
       sprintf(BashCommand, "%s%s%s%s",WorkStr, Received_TMH_file, WorkStr2, TMH_file_common);

       Result = system(BashCommand);
       fModulStatusByte2.sStatus.bSensorDataFileReceived = FALSE;
    }

    return(Result);
}
