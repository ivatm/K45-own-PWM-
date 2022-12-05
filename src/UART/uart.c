
#include "uart.h"
//#include "indicator.h"
#include "globalvarheads.h"
#include <pthread.h>

extern int32_t getMicroVoltsADC(uint32_t ADCCode);
extern uint16_t saveSettings(void);
extern void ADC_service(void);
extern void ShowSensor(void);
extern void K45_Exit(uint16_t iReason);

// Sensor data receive
extern boolean appendSensReceivedLine(uint16_t ReceivedLineNumber, char* pcLine);
extern int completeSensDataFile(void);


// Local variables -----------------------------------------------------------------
// static pthread_mutex_t uart_lock = PTHREAD_MUTEX_INITIALIZER;


// Local functions -----------------------------------------------------
uint8_t GetNextChar(void);
uint16_t DataProcess(sComm_full_structure* sDataToProcess);
uint8_t getCheckSum(uint8_t* pcBuff, uint16_t wLength);
boolean checkCheckSum(uint8_t* pcBuff, uint16_t wLength, uint8_t cCheckSum);
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
    * Let�s explore the different combinations:
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
   int i;

   //Transmit only when received
   if (fModulStatusByte2.sStatus.bUARTMsgReceived)
   {
      if (fModulStatusByte2.sStatus.bUARTSensorReception)
      {
         iLocalIndex = 0;
         buffer_out[iLocalIndex++]='b';
         buffer_out[iLocalIndex++]='e';
         buffer_out[iLocalIndex++]='g';

         buffer_out[iLocalIndex++] = keSensorLineReceived;

         uint8_t cLocalWork = getCheckSum(&buffer_out[0], iLocalIndex);
         // Add checksum
         buffer_out[iLocalIndex++] = cLocalWork;

         buffer_out[iLocalIndex++]='e';
         buffer_out[iLocalIndex++]='n';
         buffer_out[iLocalIndex++]='d';

         fModulStatusByte2.sStatus.bUARTSensorReception = FALSE;
      }
      else
      {
         iLocalIndex = 0;
         buffer_out[iLocalIndex++]='b';
         buffer_out[iLocalIndex++]='e';
         buffer_out[iLocalIndex++]='g';

         buffer_out[iLocalIndex++] = keSimpleTelegram;

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

//                  case keKint:
//                     pcPrefix = "Kin";
//                     bPointShifted = FALSE;
//                     break;

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
                  lValueToTransmit          = lValueToTransmit >> 8;

                  buffer_out[iLocalIndex++] = lValueToTransmit;
                  lValueToTransmit          = lValueToTransmit >> 8;

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
         buffer_out[iLocalIndex++] = fModulStatusByte2.cStatusByte;

         // States of Co-Processor modul
         buffer_out[iLocalIndex++] = fModulStatusByte1.cStatusByte;

         uint8_t cLocalWork = getCheckSum(&buffer_out[0], iLocalIndex);
         // Add checksum
         buffer_out[iLocalIndex++] = cLocalWork;

         buffer_out[iLocalIndex++]='e';
         buffer_out[iLocalIndex++]='n';
         buffer_out[iLocalIndex++]='d';

      }

      if (iLocalIndex > kBuff_In_Out_length)
      {
         printf("Error on transmiting!");
         //exit
         return(-1);
      }
      else
      {
         int wcount = write(fd_PC_Communication, buffer_out, iLocalIndex);

         if (wcount < 0)
         {
            return(-2);
         }
      }

      #ifdef debugmode
         printf("\r\n -> Transmitted Data Length = %d", iLocalIndex);
         printf("\r\n -> Transmitted Command = %d\r\n", buffer_out[3]);

         printf("\r\n -> Transmitted Command = ");
         for (i = 0; i < iLocalIndex; i++)
         {
            printf("%d,", buffer_out[i]);
         }
         printf("\r\n");
      #endif

      fModulStatusByte2.sStatus.bUARTMsgReceived = FALSE;

   }

   return(0);
}

int uart_addBuffer(uint8_t* str)
{

   // Attempt to send and receive
#ifdef debugmode
    //printf("UART Sending: %s", str);
#endif

   int wcount = write(fd_PC_Communication, (char*)str, strlen((char*)str));
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

int uart_send_responce(uint8_t* buffer)
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
   uint8_t* pReceivedDataPoiner;
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

      return 0;
   }

}

boolean uart_data_receive(void)
{
   static sComm_full_structure    sCommandStr;
   static CommunicationState_enum eReadingState;
   static uint8_t Sym1, Sym2, Sym3;
   static uint16_t iSupposedTelegramLength;
   uint8_t cWorkSymbol;
   uint16_t iError;
   uint16_t i;

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
               sCommandStr.cLength = 3;

               sCommandStr.cData[0] = 'b';
               sCommandStr.cData[1] = 'e';
               sCommandStr.cData[2] = 'g';

            }
            else
            {
               // Still looking for start telegramm
            }

         break;

         case eReadingCommand:

            sCommandStr.cComm = cWorkSymbol;
            sCommandStr.cData[sCommandStr.cLength++] = cWorkSymbol;

            // If sensor data are to be transmitted the expected length is other
            if ((cWorkSymbol == keSendSensor) || (cWorkSymbol == keSensorComplete))
            {
               iSupposedTelegramLength = kSensor_Data_Command_length;
            }
            else
            {
               iSupposedTelegramLength = kCommand_length;
            }


            eReadingState = eReadingTelegramm;

         break;

         case eReadingTelegramm:
            Sym1 = Sym2;
            Sym2 = Sym3;
            Sym3 = cWorkSymbol;

            if ((Sym1 == 'e') && (Sym2 == 'n') && (Sym3 == 'd'))
            {
               sCommandStr.cData[sCommandStr.cLength++] = cWorkSymbol;

               if (checkCheckSum(&sCommandStr.cData[0], sCommandStr.cLength-4, sCommandStr.cData[sCommandStr.cLength-4]))
               {
                  eReadingState = eLookForStartTelegramm;

                  bReceived = TRUE;
                  // Telegram successfully received
                  if (DataProcess(&sCommandStr))
                  {
                     fModulStatusByte2.sStatus.bUARTMsgReceived = FALSE;
                  }
                  else
                  {
                     fModulStatusByte2.sStatus.bUARTMsgReceived = TRUE;
                  }
               }
               else
               {
                  // Wrong checksum
                  eReadingState = eLookForStartTelegramm;
                  iError = kErrChecksum;
               }

            }
            else
            {
               sCommandStr.cData[sCommandStr.cLength] = cWorkSymbol;
               if (sCommandStr.cLength < iSupposedTelegramLength)
               {
                  // still here in state looking for end
                  sCommandStr.cLength++;
               }
               else
               {
                  // error!
                  iError = kErrTelLength;
                  printf("\n\r Cur.length =  %d, but expected length = %d", sCommandStr.cLength, iSupposedTelegramLength);

                  eReadingState = eLookForStartTelegramm;
               }
            }
         break;

         default:
         break;

      }
   }

   if (!iError && bReceived)
   {
      #ifdef debugmode
         printf("\r\n <- Ok Received Data Length = %d", sCommandStr.cLength);
         printf("\r\n <- Ok Received Command = %d\r\n", sCommandStr.cComm);

         printf("\r\n <- Command = ");
         for (i = 0; i < sCommandStr.cLength; i++)
         {
            printf(" %d,", sCommandStr.cData[i]);
         }
         printf("\r\n");
      #endif

      // Data clear
      sCommandStr.cComm = 0;
      sCommandStr.cLength = 0;
   }
   else
   {
      if (iError)
      {
         fModulStatusByte2.sStatus.bUARTMsgReceived = FALSE;
         #ifdef debugmode
         printf("\r\n ERROR!");
         printf("\r\n iError = %d\r\n", iError);

         printf("\r\n <- Error Received Data Length = %d", sCommandStr.cLength);
         printf("\r\n <- Error Received Command = %d\r\n", sCommandStr.cComm);

         printf("\r\n <- Command = ");
         for (i = 0; i < sCommandStr.cLength; i++)
         {
            printf(" %d,", sCommandStr.cData[i]);
         }
         printf("\r\n");
         #endif
      }
   }

   return(bReceived);
}

/* --------------------------------------------------------------------------
 * uint8_t GetNextChar(void)
 * Function reads next char no matter which, according to the global pointer pBufferReadPointer
 * -------------------------------------------------------------------------- */
uint8_t GetNextChar(void)
{
   uint8_t cReturn;

   cReturn = *pBufferReadPointer;

   if (pBufferReadPointer < &received_data[kBuff_In_Out_length - 1])
   {

      pBufferReadPointer++;
   }
   else
   {
      pBufferReadPointer = &received_data[0];
   }

   return(cReturn);

}

uint16_t DataProcess(sComm_full_structure* psDataToProcess)
{
   #define kDataShift 4
   uint32_t lOutValue, lMin, lMax;
   uint16_t iLineNumber;
   char* pStringPointer;

   switch (psDataToProcess->cComm)
   {
      case keTset_input:
         printf("Command Tset\r\n");
         lOutValue = (uint32_t)psDataToProcess->cData[kDataShift +0] | (uint32_t)(psDataToProcess->cData[kDataShift +1] << 8);
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
         lOutValue = (uint32_t)psDataToProcess->cData[kDataShift +0] | (uint32_t)(psDataToProcess->cData[kDataShift +1] << 8);
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
         lOutValue = (uint32_t)psDataToProcess->cData[kDataShift +0] | (uint32_t)(psDataToProcess->cData[kDataShift +1] << 8);

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
         lOutValue = (uint32_t)psDataToProcess->cData[kDataShift +0] | (uint32_t)(psDataToProcess->cData[kDataShift +1] << 8);
         lMin = VarForIndication[keKprop].lVarMin;
         lMax = VarForIndication[keKprop].lVarMax;

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
         lOutValue = (uint32_t)psDataToProcess->cData[kDataShift +0] | (uint32_t)(psDataToProcess->cData[kDataShift +1] << 8);
         lMin = VarForIndication[keKdiff].lVarMin;
         lMax = VarForIndication[keKdiff].lVarMax;

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
         if (psDataToProcess->cData[kDataShift +0])
         {
            bScanOrSetMode = TRUE;
         }
         else
         {
            bScanOrSetMode = FALSE;
         }
         break;

      case keTemperatureUnitSwitch:
            bCelsiumOrKelvin = !bCelsiumOrKelvin;
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

      case keSendSensor:
         // Line number: two bytes after command
         iLineNumber = (psDataToProcess->cData[kDataShift+0] << 8) | psDataToProcess->cData[kDataShift+1];
         // The line to be saved goes after LineNumber and ends with '\n'.
         // Here just transfer the address of first byte
         pStringPointer = (char*) &psDataToProcess->cData[kDataShift + 2];
         printf(pStringPointer);

         // If Previous File not saved yet, the Line should be ignored
         if (!fModulStatusByte2.sStatus.bSensorDataFileReceived)
         {
            (void)appendSensReceivedLine(iLineNumber, pStringPointer);
            fModulStatusByte2.sStatus.bUARTSensorReception = TRUE;
         }

         break;

      case keSensorComplete:
         printf("\n\r  Sensor completion\n\r");
         fModulStatusByte2.sStatus.bSensorDataFileReceived = TRUE;
         (void)completeSensDataFile();
         break;

      case keNOP:
         // Nothing to do
         break;

      default:
         //printf("Unknown Command");
         return(-1);
         break;
   }

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

/* --------------------------------------------------------------------------
 * uint8_t getCheckSum(uint8_t* pcBuff, uint16_t wLength)
 * Gets sum on 1 Byte basis. The last 1 byte for the sum itself)
 * --------------------------------------------------------------------------*/
uint8_t getCheckSum(uint8_t* pcBuff, uint16_t wLength)
{
   uint8_t cResult;
   uint16_t wIndex;

   cResult = 0;

   for (wIndex = 0; wIndex < wLength; wIndex++, pcBuff++)
   {
      cResult += *pcBuff;
   }

   return(cResult);
}

/* --------------------------------------------------------------------------
 * boolean checkCheckSum(uint8_t* pcBuff, uint16_t wLength, uint8_t cCheckSum)
 * Gets sum on 1 Byte basis. The last 1 byte for the sum itself)
 * --------------------------------------------------------------------------*/
boolean checkCheckSum(uint8_t* pcBuff, uint16_t wLength, uint8_t cCheckSum)
{
   uint8_t cLocalValue;

   cLocalValue = getCheckSum(pcBuff, wLength);

   printf("\n\r cLocalValue =  %d, but received cCheckSum = %d, with length %d\n\r", cLocalValue, cCheckSum, wLength);
   if (cLocalValue == cCheckSum)
   {
      return(TRUE);
   }
   else
   {
      return(FALSE);
   }
}

