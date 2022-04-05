/*
 * keypad.h : Raspberry PI (bcm2835) compatible switching keypad library
 *
 *  Created on: Oct 8, 2013
 *      Author: X. Zubizarreta
 */
#include <bcm2835.h>
 #include <stdio.h>
#include "user_types.h"

//Define the conencted pins
#define PIN_ROW1    0
#define PIN_ROW2    5
#define PIN_ROW3    6
#ifdef RASPBERRY_PWM
#define PIN_ROW4    19
#else
#define PIN_ROW4    13
#endif
#define PIN_ROW5    26

#define PIN_COL1    8
#define PIN_COL2    7
#define PIN_COL3    1
#ifdef RASPBERRY_PWM
#define PIN_COL4    25
#else
#define PIN_COL4    12
#endif
//Define the size of the keypad
#define ROWS 5
#define COLS 4

//define the column positions to pins
  uint8_t cols[COLS]={PIN_COL1,PIN_COL2,PIN_COL3,PIN_COL4};
//define the row position to pins
  uint8_t row[ROWS]={PIN_ROW1,PIN_ROW2,PIN_ROW3,PIN_ROW4, PIN_ROW5};
//setup the uint8_tacter mapping
  uint8_t map[ROWS][COLS]=
          {{'E','>','0','<'},
           {'c','9','8','7'},
           {'_','6','5','4'},
           {'^','3','2','1'},
           {'*','#','F','f'}};

  #define antibounce 50

//Initializes the driver for the GPIO and configures the keypad
uint8_t init_keypad(void);
//Polls for a key (returns 0 if none is currently pushed)
uint8_t get_key(void);
//Waits until a key is pressed
Buttons_enum wait_key(void);

KeyPadCommands_enum GetCommand(Buttons_enum eKey);
void ProcessCommand(KeyPadCommands_enum eCommand);
