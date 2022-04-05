/*
 * modulSPI.h
 *
 *  Header for communication to additional RealTime module (RTmodule) operations.
 *
 *  In Fact it is Arduino "mini"
 *  Created on: 23 זמגע. 2020 נ.
 *      Author: Oliva
 */

#ifndef INC_MODULSPI_H_
#define INC_MODULSPI_H_
#include "user_types.h"

#define MODUL_ISP_PIN      25

 #define kMsgLength 10

/* Transmit message structure
ByteNumber  | Byte 0   |  Byte 1       | Byte 2       |  Byte 3      | Byte 4       |  Byte 5     | Byte 6       |  Byte 7     | Byte 8   | Byte 9   |
ByteValue   |  '>'     |   nn          | Value Byte 0 | Value Byte 1 |   dd         |   dd        |   dd         |   dd        |   chck   |   '<'    |
ByteMeaning | StartSym |  Flags        | Heater Command              | Cooler Command             |  Valve Command             | CheckSum | EndSym   |
 */

typedef struct
{
   uint8_t bFree           :7;
   uint8_t bStartSetting   :1;
} sFlagCommand_Struct;

typedef union
{
   sFlagCommand_Struct sFlagCommand;
   uint8_t             cFlagsByte;
} fFlagCommandUnion;

#pragma pack(push, 1)
typedef struct
{
   char               startSymbol;
   fFlagCommandUnion  FlagCommand;
   uint16_t           HeaterCommand;
   uint16_t           CoolerCommand;
   uint16_t           ValveCommand;
   uint8_t            checkSum;
   char               stopSymbol;
} sExecutive_TxObj_Struct;
#pragma pack(pop)

typedef union
{
   sExecutive_TxObj_Struct TxObj;
   uint8_t cTxData[kMsgLength];
}  fExecutive_TxObj_Union;

/* Received on master side message structure
ByteNumber  | Byte 0   |  Byte 1       | Byte 2       |  Byte 3      | Byte 4       |  Byte 5     | Byte 6       |  Byte 7     | Byte 8   | Byte 9   |
ByteValue   |  '>'     |   nn          | Value Byte 0 | Value Byte 1 |   dd         |   dd        |   dd         |   dd        |   chck   |   '<'    |
ByteMeaning | StartSym |  Status       | Heater Voltage              | Cooler Voltage             |  Control diode Voltage     | CheckSum | EndSym   |
 */

typedef struct
{
   uint8_t bSetMode             :1;      // Setmode of co-processor
   uint8_t bHeaterError3        :1;      // HeaterVoltage
   uint8_t bCoolerError3        :1;      // CoolerVoltage
   uint8_t bControlDiodeError3  :1;      // ControlDiodeVoltage
   uint8_t bNotFoundErr         :1;      // co-processor plate not found
   uint8_t bFree                :3;
} sStatus_Struct;

typedef union
{
   sStatus_Struct sStatus;
   uint8_t        cStatusByte;
} fStatusUnion;

#pragma pack(push, 1)
typedef struct
{
   uint8_t      free;
   char         startSymbol;
   fStatusUnion Status;
   uint16_t     HeaterVoltage;
   uint16_t     CoolerVoltage;
   uint16_t     ControlDiodeVoltage;
   uint8_t      checkSum;
} sExecutive_RxObj_Struct;
#pragma pack(pop)

typedef union
{
   sExecutive_RxObj_Struct RxObj;
   uint8_t cRxData[kMsgLength];
}  fExecutive_RxObj_Union;


// Global Variables
extern uint8_t RecBuf[kMsgLength];       // Reception array
extern uint8_t TransmBuf[kMsgLength];    // Transmission array

// Headers of Procedures
extern void    Module_GPIOConfig(void);
extern uint8_t transferAndWait(const uint8_t what);
extern void    transferMsG(uint8_t* pMsgRec , uint8_t* pMsgTr);


#endif /* INC_MODULSPI_H_ */
