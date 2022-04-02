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
#define PIN27    0
#define PIN29    5
#define PIN31    6
#define PIN33    13
#define PIN37    26

#define PIN24    8
#define PIN26    7
#define PIN28    1
#define PIN32    12

//Define the size of the keypad
#define ROWS 5
#define COLS 4

//define the column positions to pins
  uint8_t cols[COLS]={PIN24,PIN26,PIN28,PIN32};
//define the row position to pins
  uint8_t row[ROWS]={PIN27,PIN29,PIN31,PIN33, PIN37};
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
