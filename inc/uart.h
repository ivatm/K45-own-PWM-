/*
 * uart.h
 *
 *  Created on: 21 трав. 2021 р.
 *      Author: Oliva
 */

#ifndef INC_UART_H_
#define INC_UART_H_

#include <fcntl.h>   // Contains file controls like O_RDWR
#include <errno.h>   // Error integer and strerror() function
#include <termios.h> // Contains POSIX terminal control definitions
#include <unistd.h>  // write(), read(), close()

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// UART Definitions
// B0,    B50,   B75,   B110,   B134,   B150,   B200,    B300,    B600, B1200, B1800,
// B2400, B4800, B9600, B19200, B38400, B57600, B115200, B230400, B460800
#define kBoadRate_Default            B9600
#define kBuffInput_length            255
#define kBuff_In_Out_length          200
#define kCommand_length              11
#define kSensor_Data_Command_length  100

// The general terminal interface that
// is provided to control asynchronous communications ports
struct termios serial;

// Allocate memory for In/Out string.
uint8_t buffer_out[kBuff_In_Out_length];
uint8_t received_data[kBuff_In_Out_length];

// Allocate memory for read buffer, set size according to your needs
uint8_t buffer_in[kBuffInput_length];
uint8_t* pBufferWritePointer;
uint8_t* pBufferReadPointer;

uint16_t ByteSReceived;

// file descriptor of terminal
int fd_PC_Communication;

// Type define
typedef struct
{
   uint8_t cComm;                   // Command
   uint8_t cLength;                 // Number of byte
   uint8_t cData[kSensor_Data_Command_length];  // All received bytes. Data not longer then 10 byte
} sComm_full_structure;

typedef enum
{
   eLookForStartTelegramm = 0,
   eReadingCommand,
   eReadingTelegramm,
} CommunicationState_enum;

/* --------------------------------------------------------------------
 * List of all K45 to PC answers.
 * It is used in UART communication
 -------------------------------------------------------------------- */
typedef enum
{
   keSimpleTelegram     = 0,
   keSensorLineReceived = 1
} K45Commands_enum;

/* --------------------------------------------------------------------
 * UART Error codes
 -------------------------------------------------------------------- */
#define kErrTelLength 1
#define kErrCommand   2
#define kErrChecksum  3


#endif /* INC_UART_H_ */
