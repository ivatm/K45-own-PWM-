
#include "uart.h"
//#include "indicator.h"
#include "globalvarheads.h"
#include <pthread.h>

extern int32_t getMicroVoltsADC(uint32_t ADCCode);
extern uint16_t saveSettings(void);
extern void ADC_service(void);
extern void ShowSensor(void);
extern void K45_Exit(uint16_t iReason);

// Local variables -----------------------------------------------------------------
// static pthread_mutex_t uart_lock = PTHREAD_MUTEX_INITIALIZER;


// -----------------------------------------------------
char GetNextChar(void);
uint16_t DataProcess(sComm_full_structure* sDataToProcess);
// -----------------------------------------------------


int uart_init(void)
{

   //printf("Opening %s", "/dev/serial0");

   fd_PC_Communication = open("/dev/serial0", O_RDWR | O_NOCTTY | O_NDELAY);

   if (fd_PC_Communication == -1)
   {
       perror("/dev/serial0");
       return -1;
   }

   if (tcgetattr(fd_PC_Communication, &serial) < 0)
   {
       perror("Getting configuration");
       return -1;
   }

   // Set up Serial Configuration
   serial.c_iflag = 0; /* input mode flags */
   serial.c_oflag = 0; /* output mode flags */
   serial.c_lflag = 0; /* local mode flags */
   serial.c_cflag = 0; /* control mode flags */

   // serial.c_line - nothing to do /* line discipline */


   /*
    * Let’s explore the different combinations:
      VMIN = 0, VTIME = 0: No blocking, return immediately with what is available
      VMIN > 0, VTIME = 0: This will make read() always wait for bytes
                           (exactly how many is determined by VMIN), so read() could block indefinitely.
      VMIN = 0, VTIME > 0: This is a blocking read of any number chars with a maximum timeout (given by VTIME).
                           read() will block until either any amount of data is available, or the timeout occurs.
                           This happens to be my favorite mode (and the one I use the most).
      VMIN > 0, VTIME > 0: Block until either VMIN characters have been received, or VTIME after first character has elapsed.
                           Note that the timeout for VTIME does not begin until the first character is received.
    * */

   serial.c_cc[VMIN] = 0;
   serial.c_cc[VTIME] = 0;

   // 9600/8 bits /  Allow readin  (No parity)
   serial.c_cflag = kBoadRate_Default | CS8 | CREAD;

   tcsetattr(fd_PC_Communication, TCSANOW, &serial); // Apply configuration
// Variables ----------------------------------------------------------
   pBufferWritePointer = &received_data[0];
   pBufferReadPointer = &received_data[0];

   return 0;
}

/* -----------------------------------------------------------------------------
 * int uart_send(void)
 * The Procedure transmits all available data. The telegram structure:
 * b,e,g,
 * 2 byte of Treal,
 * 2 byte of Tset,
 * 2 byte of Tcurset,
 * 2 byte of DeltaT,
 * 2 byte of Deltat,
 *  2 byte of Kprop,
 *  2 byte of Kdiff,
 *  3 byte of Ureal,
 *  1 byte current Controller state
 *  1 byte Power modul state
 *  e,n,d.
 * ----------------------------------------------------------------------------- */

int uart_send(void)
{
   VarsForIndicator_enum eVarNumber;

   uint32_t lValueToTransmit;
   uint16_t iLocalIndex;
   uint16_t iValueToUart;

   iLocalIndex = 0;
   buffer_out[iLocalIndex++]='b';
   buffer_out[iLocalIndex++]='e';
   buffer_out[iLocalIndex++]='g';

   for (eVarNumber = keTreal; eVarNumber < keMaxVariableNum; eVarNumber++)
   {
      lValueToTransmit = *VarForIndication[eVarNumber].plVarValue;

      switch(eVarNumber)
      {
         case keTreal:
         case keTset:
         case keTcurSet:
         case keDeltaT:
            iValueToUart = (uint16_t)lValueToTransmit; // K = 0.01

            buffer_out[iLocalIndex++] = iValueToUart;
            buffer_out[iLocalIndex++] = iValueToUart >> 8;

            break;

         case keDeltat:
            iValueToUart = (uint16_t)lValueToTransmit; // K = 0.001

            buffer_out[iLocalIndex++] = iValueToUart;
            buffer_out[iLocalIndex++] = iValueToUart >> 8;
            break;

         case keKprop:
            iValueToUart = (uint16_t)lValueToTransmit; // K = 1

            buffer_out[iLocalIndex++] = iValueToUart;
            buffer_out[iLocalIndex++] = iValueToUart >> 8;
            break;

//            case keKint:
//               pcPrefix = "Kin";
//               bPointShifted = FALSE;
//               break;

         case keKdiff:
            iValueToUart = (uint16_t)lValueToTransmit; // K = 1

            buffer_out[iLocalIndex++] = iValueToUart;
            buffer_out[iLocalIndex++] = iValueToUart >> 8;
            break;


         case keUreal:
            lValueToTransmit = getMicroVoltsADC(lValueToTransmit); // K = 1e-6

            if (lValueToTransmit > (uint32_t)(kfUmax*1e6))
            {
               lValueToTransmit = (uint32_t)(kfUmax*1e6);
            }

            buffer_out[iLocalIndex++] = lValueToTransmit;
            lValueToTransmit = lValueToTransmit >> 8;
            buffer_out[iLocalIndex++] = lValueToTransmit;
            lValueToTransmit = lValueToTransmit >> 8;
            buffer_out[iLocalIndex++] = lValueToTransmit;

            // Last byte is not needed
            //lValueToTransmit = lValueToTransmit >> 8;
            //buffer_out[iLocalIndex++] = lValueToTransmit;
            break;

         case keCLevel:
            if (lCryoLevel > 100)
            {
               buffer_out[iLocalIndex] = 100;
            }
            else
            {
               buffer_out[iLocalIndex] = (uint8_t)lCryoLevel;
            }
            iLocalIndex++;
            break;

         default:
            break;
      }
   }

   // States of controller
   buffer_out[iLocalIndex] = 0;
   if (bScanOrSetMode)
   {
      buffer_out[iLocalIndex] |= 1 << 0;
   }

   // Desired temperature achieved
   if (bTempSetAchieved)
   {
      buffer_out[iLocalIndex] = 1 << 1;
   }

   // Temperature display mode
   if (bCelsiumOrKelvin)
   {
      buffer_out[iLocalIndex] = 1 << 2;
   }

   // the Cryolevel measurement foreseen
   if (bCryoLevelMeasuring)
   {
      buffer_out[iLocalIndex] = 1 << 3;
   }
   iLocalIndex++;

   // States of Co-Processor modul
   buffer_out[iLocalIndex++] = fPowerModulStatus.cStatusByte;

   buffer_out[iLocalIndex++]='e';
   buffer_out[iLocalIndex++]='n';
   buffer_out[iLocalIndex++]='d';

   if (iLocalIndex > kBuff_In_Out_length)
   {
      //printf("Error on transmiting!");
      //exit
      return(-1);
   }
   else
   {
      int wcount = write(fd_PC_Communication, buffer_out, iLocalIndex);
      //printf("UART Transmitted: %d\n", iLocalIndex);

      if (wcount < 0)
      {
         return(-2);
      }
   }

   return(0);
}

int uart_addBuffer(char* str)
{

   // Attempt to send and receive
#ifdef debugmode
    //printf("UART Sending: %s", str);
#endif

   int wcount = write(fd_PC_Communication, str, strlen(str));
   if (wcount < 0)
   {
       perror("Write");
       return -1;
   }
   else
   {
#ifdef debugmode
        //printf("UART Sent %d characters", wcount);
#endif
       return 0;
   }
}

int uart_send_responce(char* buffer)
{
   return(0);
}

/* --------------------------------------------------------------------------------------------------------------
 * int uart_read(void)
 * transfer data to work buffer
 * returns 0 if Ok
 *         1 if overflow
 *         -1 on "read" error
 * -------------------------------------------------------------------------------------------------------------- */
int uart_read(void)
{
   // Pointer on symbol in receiving buffer to save data in read buffer
   char* pReceivedDataPoiner;
   uint16_t iIndexData;

   // Read bytes. The behavior of read() (e.g. does it block?,
   // how long does it block for?) depends on the configuration
   // settings above, specifically VMIN and VTIME
   // rcount is the number of bytes read. n may be 0 if no bytes were received, and can also be negative to signal an error.
   int rcount = read(fd_PC_Communication, buffer_in, sizeof(buffer_in));

   if (rcount < 0)
   {
       perror("Read error!");
       return -1;
   }
   else if (rcount > kBuffInput_length)
   {
      perror("Buffer overflow!");
      return(1);
   }
   else if (rcount == 0)
   {
      // nothing received
      return(2);
   }
   else
   {

      pReceivedDataPoiner = &buffer_in[0];

      for (iIndexData = 0; iIndexData < rcount; iIndexData++, pReceivedDataPoiner++)
      {
         *pBufferWritePointer = *pReceivedDataPoiner;

         #ifdef debugmode
         //   printf("%d,", *pBufferWritePointer);
         #endif

         if (pBufferWritePointer < &received_data[kBuff_In_Out_length - 1])
         {
            pBufferWritePointer++;
         }
         else
         {
            pBufferWritePointer = &received_data[0];
         }
      }
      // its ok

      #ifdef debugmode
      // printf("\r\nReceived byte number = %d\r\n", rcount);
      #endif

      return 0;
   }


}

boolean uart_data_receive(void)
{
   static sComm_full_structure    sCommandStr;
   static CommunicationState_enum eReadingState;
   static char Sym1, Sym2, Sym3;
   char cWorkSymbol;
   uint16_t iError;

   int iLocalWorkCounter; // safeguard of overflow

   boolean bReceived;

   bReceived = FALSE;
   iLocalWorkCounter = 0;
   iError = 0;

   while ((pBufferReadPointer != pBufferWritePointer)
           && (iLocalWorkCounter < kBuff_In_Out_length)
           && !bReceived)
   {
      iLocalWorkCounter++;

      cWorkSymbol = GetNextChar();

      switch (eReadingState)
      {
         case eLookForStartTelegramm:
            Sym1 = Sym2;
            Sym2 = Sym3;
            Sym3 = cWorkSymbol;

            if ((Sym1   == 'b') && (Sym2 == 'e') && (Sym3 == 'g'))
            {
               eReadingState = eReadingCommand;
               sCommandStr.cComm = 0;
               sCommandStr.cLength = 0;
               for (int i=0; i < kCommand_length; i++)
               {
                  sCommandStr.cData[i] = 0;
               }
            }
            else
            {
               // Still looking for start telegramm
            }

         break;

         case eReadingCommand:

            sCommandStr.cComm = cWorkSymbol;

            eReadingState = eReadingTelegramm;

         break;

         case eReadingTelegramm:
            Sym1 = Sym2;
            Sym2 = Sym3;
            Sym3 = cWorkSymbol;

            if ((Sym1   == 'e') && (Sym2 == 'n') && (Sym3 == 'd'))
            {
               eReadingState = eLookForStartTelegramm;
               bReceived = TRUE;
            }
            else
            {
               sCommandStr.cData[sCommandStr.cLength] = cWorkSymbol;
               if (sCommandStr.cLength < kCommand_length)
               {
                  sCommandStr.cLength++;
               }
               else
               {
                  // error!
                  eReadingState = eLookForStartTelegramm;
                  iError = 1;
                  #ifdef debugmode
                  //   printf("\r\n sCommandStr.cLength = %d", sCommandStr.cLength);
                  //   for (int i=0; i < sCommandStr.cLength; i++)
                  //   {
                  //      printf(":%d:", sCommandStr.cData[i]);
                  //   }
                  #endif
               }
            }
         break;

         default:
         break;

      }
   }


   if (!iError && bReceived)
   {
      if (iLocalWorkCounter >= kBuff_In_Out_length)
      {
         // Overflow
         return(FALSE);
      }

      if (sCommandStr.cLength > 2)
      {
         sCommandStr.cLength = sCommandStr.cLength - 2;
      }
      else
      {
         if (sCommandStr.cLength == 2)
         {
            // Empty command
            sCommandStr.cLength = 0;
            sCommandStr.cComm = 0;
         }
         else
         {
            // error!
            iError = 2;
         }
      }

      // Telegram successfully received
      (void)DataProcess(&sCommandStr);
      // Next command reception
      bReceived = TRUE;

      #ifdef debugmode
      //   printf("\r\nReceived Data Length = %d", sCommandStr.cLength);
      //   printf("\r\nReceived Command = %d\r\n", sCommandStr.cComm);
      //
      //   //printf("\r\nReceived Command = ");
      //   for (int i=0; i < sCommandStr.cLength; i++)
      //   {
      //      printf("%d,", sCommandStr.cData[i]);
      //   }
      //   printf("\r\n");
      #endif
   }
   else
   {
      #ifdef debugmode
         if (iError)
         {
            printf("\r\n ERROR!");
            printf("\r\n iError = %d\r\n", iError);
            printf("\r\n bReceived = %d\r\n", bReceived);
         }
      #endif
   }

   #ifdef debugmode
   //   if (pBufferReadPointer >= pBufferWritePointer)
   //   {
   //      printf("\r\n End telegram received!");
   //   }
   #endif


   return(bReceived);
}

/* --------------------------------------------------------------------------
 * char GetNextChar(void)
 * Function reads next char no matter which, according to the global pointer pBufferReadPointer
 * -------------------------------------------------------------------------- */
char GetNextChar(void)
{
   char cReturn;

   cReturn = *pBufferReadPointer;

   if (pBufferReadPointer < &received_data[kBuff_In_Out_length - 1])
   {

      pBufferReadPointer++;
   }
   else
   {
      pBufferReadPointer = &received_data[0];
   }

   #ifdef debugmode
   //   printf(" %d:", cReturn);
   #endif // debugmode

   return(cReturn);

}

uint16_t DataProcess(sComm_full_structure* psDataToProcess)
{
   uint32_t lOutValue, lMin, lMax;

   switch (psDataToProcess->cComm)
   {
      case keTset_input:
         printf("Command Tset\r\n");
         lOutValue = (uint32_t)psDataToProcess->cData[0] | (uint32_t)(psDataToProcess->cData[1] << 8);
         lMin = VarForIndication[keTset].lVarMin;
         lMax = VarForIndication[keTset].lVarMax;

         if (lOutValue > lMax)
         {
            lOutValue = lMax;
         }
         else
         {
            if (lOutValue < lMin)
            {
               lOutValue = lMin;
            }
         }

         lTemperatureSet = lOutValue;
         break;
      case keTstep_input:
         printf("Command Tstep\r\n");
         lOutValue = (uint32_t)psDataToProcess->cData[0] | (uint32_t)(psDataToProcess->cData[1] << 8);
         lMin = VarForIndication[keDeltaT].lVarMin;
         lMax = VarForIndication[keDeltaT].lVarMax;

         if (lOutValue > lMax)
         {
            lOutValue = lMax;
         }
         else
         {
            if (lOutValue < lMin)
            {
               lOutValue = lMin;
            }
         }

         lDelta_T = lOutValue;
         break;
      case ketime_step_input:
         printf("Command time step\r\n");
         lOutValue = (uint32_t)psDataToProcess->cData[0] | (uint32_t)(psDataToProcess->cData[1] << 8);

         lMin = VarForIndication[keDeltat].lVarMin;
         lMax = VarForIndication[keDeltat].lVarMax;

         if (lOutValue > lMax)
         {
            lOutValue = lMax;
         }
         else
         {
            if (lOutValue < lMin)
            {
               lOutValue = lMin;
            }
         }

         lDelta_t = lOutValue;
         break;
      case keKprop_input:
         printf("Command Kprop\r\n");
         lOutValue = (uint32_t)psDataToProcess->cData[0] | (uint32_t)(psDataToProcess->cData[1] << 8);
         lMin = VarForIndication[keDeltat].lVarMin;
         lMax = VarForIndication[keDeltat].lVarMax;

         if (lOutValue > lMax)
         {
            lOutValue = lMax;
         }
         else
         {
            if (lOutValue < lMin)
            {
               lOutValue = lMin;
            }
         }

         lKprop = lOutValue;
         break;
      case keKdiff_input:
         printf("Command Kdiff\r\n");
         lOutValue = (uint32_t)psDataToProcess->cData[0] | (uint32_t)(psDataToProcess->cData[1] << 8);
         lMin = VarForIndication[keDeltat].lVarMin;
         lMax = VarForIndication[keDeltat].lVarMax;

         if (lOutValue > lMax)
         {
            lOutValue = lMax;
         }
         else
         {
            if (lOutValue < lMin)
            {
               lOutValue = lMin;
            }
         }
         lKdiff = lOutValue;
         break;

      case keSet_ScanSelect:
         if (psDataToProcess->cData[0])
         {
            bScanOrSetMode = TRUE;
         }
         else
         {
            bScanOrSetMode = FALSE;
         }
         break;

      case keTemperatureUnitSwitch:
         if (psDataToProcess->cData[0])
         {
            bCelsiumOrKelvin = TRUE;
         }
         else
         {
            bCelsiumOrKelvin = FALSE;
         }
         break;

      case keSaveConfigs:
         saveSettings();
         break;

      case keADCCalibration:
         ADC_service();
         break;

      case keShowSensor:
         ShowSensor();
         break;

      case keExit:
         K45_Exit(0);
         break;

      default:
         //printf("Unknown Command");
         return(-1);
         break;
   }

   // the command fulfilled -> clear
   psDataToProcess->cComm = 0;
   psDataToProcess->cLength = 0;

   return(0);
}


/* --------------------------------------------------------------------------
 * int uart_close(void)
 * Stop communication and close Serial Port
 * --------------------------------------------------------------------------*/
int uart_close(void)
{
    close(fd_PC_Communication);
    return 0;
}

