//

#include <ads1256.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>

#include <string.h>
#include <sys/timeb.h>

#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

#include <i2c_lcd.h>

#include "ADCHeader.h"

#include <pthread.h>
#include <stdio.h>

#include "defines.h"

#include "modulSPI.h"

#include "user_types.h"
#include "globalvarheads.h"
//#include "uart.h"

#include "pigpio.h"

///////////////////////////////////////////
#ifdef RASPBERRY_PWM
extern int PWM_init(void);
#endif

// Local defines


// Declaration for threads of the system
pthread_t ADC_Thread;              // the ADC data Flow thread
pthread_t TemperatureRegulator;    // Temperature control calculations
pthread_t Interface;               // Indication/Keyboard
pthread_t Keypad_Thread;           // Keyboard
pthread_t PowerEquipment;          // Communication with Co-Processor plate / Or direct PWM's outputs
pthread_t ServiceMeasurements;     // Additional measurements
pthread_t UARTCommThread;          // Communication with PC via Serial Protocol


// Extern procedures -------------------------------------------------------------
extern void iniTemperaturController(void);
extern boolean periferal_SPI1_Init(void);
extern void CalculRegulator(void);

extern void executeModulControl(void);
extern void updateCurrentVoltages(void);

extern void KeyboardProcess(void);
extern void AutoSettingTemperature(void);

extern void lcd_Init(void);
extern void lcd_update(void);
extern void HelloShow(void);

extern char init_keypad(void);

extern int uart_init(void);
extern int uart_read(void);
extern boolean uart_data_receive(void);
extern int uart_send(void);
extern int copyFile(void);
extern int BashCopyFile(void);

extern void ADC_Calibration(void);

// Additional measurements
extern void FrequencyMeasurement(void);
extern void ControlVoltagesCheck(void);
extern int PigpioInit(void);
extern void CurentStatusUpdate(void);

// protection
uint64_t getSerialID(void);
extern void K45_Exit(uint16_t iReason);

// Local procedures
uint16_t K45GlobalInit(void);

void* Measurements(void *parm);
void* PowerEquipment_service(void *parm);

uint16_t SetThreadPriority(pthread_t pthread_id, uint16_t priority);


/* ===============================================================================
 * The thread for Output user interfaces.
 *
 * =============================================================================== */
void* Interface_Process()
{
   static uint16_t iLocalCounter;

   lcd_Init();
   delay(500);

   LCDI2C_backlight();

   HelloShow();
// ToDo: Not delay but counter of program cycles!
   delay(3000);

   while(fSystemThreadControl.s.bInterfaceOn)
   {

      // Cycle counter
      iLocalCounter++;

      lcd_update();

      delay(kIndicationTimeUpdate);
   }
   /* the function must return something - NULL will do */
   return NULL;
}

/* ===============================================================================
 * The thread for additional service measurements
 *
 * =============================================================================== */
void* ServiceMeasurements_Process()
{

   while(fSystemThreadControl.s.bTemperatureRegulatorOn)
   {

      // Measure frequency
      FrequencyMeasurement();

      // Clear errors
      ControlVoltagesCheck();

      CurentStatusUpdate();

      delay(kIndicationTimeUpdate);
   }

   gpioTerminate();

   return NULL;
}


/******************************************************************************
function:  The thread for Output user interfaces.
parameter:
Info:
******************************************************************************/
void* Keypad_service()
{
   while(fSystemThreadControl.s.bKeypad_ThreadOn)
   {
      KeyboardProcess();
      delay(500);
   }
/* the function must return something - NULL will do */
return NULL;
}

/* Variables for controlling the measuring and temperature regulating
   If the flag is set, the ADC-conversion is started and the flag again goes to FALSE
*/
boolean         ADC_needed_flag,         // Flag
                ADC_ready_flag;
pthread_cond_t  ADC_start_cv,            // Conditional variable
                ADC_stop_cv;
pthread_mutex_t ADC_thread_flag_mutex;   // Thread mutex

/******************************************************************************
function:  Temperature Regulator main engine
parameter:
Info:
******************************************************************************/
void* TemperatureRegulator_service()
{

   int retVal;
   pthread_attr_t attr;
   struct sched_param schedParam;

   retVal = pthread_attr_init(&attr);
   if (retVal)
   {
       fprintf(stderr, "pthread_attr_init error %d\n", retVal);
       exit(1);
   }

   retVal = pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
   if (retVal)
   {
       fprintf(stderr, "pthread_attr_setinheritsched error %d\n", retVal);
       exit(1);
   }

   retVal = pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
   if (retVal)
   {
       fprintf(stderr, "pthread_attr_setschedpolicy error %d\n", retVal);
       exit(1);
   }

   schedParam.sched_priority = 55;
   retVal = pthread_attr_setschedparam(&attr, &schedParam);
   if (retVal)
   {
       fprintf(stderr, "pthread_attr_setschedparam error %d\n", retVal);
       exit(1);
   }

   pthread_create(&ADC_Thread,   &attr, Measurements, NULL);

   iniTemperaturController();

   // First initialization to FALSE that means "no calculation needed"
   sSensorData.bMeasurementReady = FALSE;

   while(fSystemThreadControl.s.bTemperatureRegulatorOn)
   {
      pthread_join( ADC_Thread, NULL);

      // The new state preparation
      CalculRegulator();
      //executeModulControl();
      //updateCurrentVoltages();
      pthread_create(&ADC_Thread,   &attr, Measurements, NULL);
      pthread_join( PowerEquipment, NULL);
      pthread_create(&PowerEquipment, &attr, PowerEquipment_service, NULL);

      #ifdef SCANNING
         AutoSettingTemperature();
      #endif

      delay(mNormTime(kTemperaturControlPeriod));
   }
   /* the function must return something - NULL will do */
   return NULL;
}

/******************************************************************************
function:  The thread for measurements
parameter:
Info:
******************************************************************************/
void* Measurements(void *parm)
{
   if (bADCCalibrationProcess)
   {
      // In case of Calibration Flag is On, the according procedure
      ADC_Calibration();
      // The Flag goes Off again, since the calibration is fulfilled
      bADCCalibrationProcess = FALSE;
   }
   else
   {
      // If no calibration necessary, simple measurement
      updateCurrentVoltages();
   }

   /* the function must return something - NULL will do */
   pthread_exit(NULL);
}

/******************************************************************************
function:  The thread for power switches.
parameter:
Info:
******************************************************************************/
void* PowerEquipment_service(void *parm)
{
   executeModulControl();

   return(NULL);
}


/******************************************************************************
function:  The thread for communication with PC
parameter:
Info:
******************************************************************************/
void* UARTCommThread_service()
{

   if (bUART_Active)
   {
      while(fSystemThreadControl.s.bUARTCommThreadOn)
      {

         if (!uart_read())
         {
            #ifdef debugmode
              // printf(" Error on reception\r\n");
            #endif
         }
         else
         {
            // Its ok
         }

         uart_data_receive();
         uart_send();


         //(void)copyFile();
         BashCopyFile();

         //
         delay(kReceptionCheck);
      }
   }

   /* the function must return something - NULL will do */
   return(NULL);
}

/******************************************************************************
function:  uint16_t Inits(void)
parameter:
Info:
******************************************************************************/
uint16_t Inits(void)
{

   pthread_t  pt_id;
   int Err;

   if (K45GlobalInit())
   {
      printf("Something goes wrong on init...\n");
      return(FALSE);
   }

   // Threads are active (possible to create)
   fSystemThreadControl.cAllThreadStates = 0xFF;

   // Low Priority & low speed
   pt_id = pthread_create(&Interface,    NULL, Interface_Process, NULL);
   delay(500);
   if (pt_id != 0)
   {
      Err = SetThreadPriority(pt_id, 10);

      if (Err != 0)
      {
         printf("\nError on Interface priority setting: %d, %d!\n\r", (int)pt_id, Err);
      }
   }

   pt_id = pthread_create(&ServiceMeasurements,    NULL, ServiceMeasurements_Process, NULL);
   delay(500);
   if (pt_id != 0)
   {
      #ifdef debugmode
         printf("ServiceMeasurements  created\n");
      #endif
      Err = SetThreadPriority(pt_id, 10);

      if (Err != 0)
      {
         printf("\nError on Interface priority setting: %d, %d!\n\r", (int)pt_id, Err);
      }
   }

   // Lowest Priority but highest speed
   pt_id = pthread_create(&UARTCommThread, NULL, UARTCommThread_service, NULL);
   delay(5);
   if (pt_id != 0)
   {
      Err = SetThreadPriority(pt_id, 5);

      if (Err != 0)
      {
         printf("\nError on UARTCommThread priority setting: %d, %d!\n\r", (int)pt_id, Err);
      }
   }

   pt_id = pthread_create(&Keypad_Thread,  NULL, Keypad_service,         NULL);
   delay(5);
   if (pt_id != 0)
   {
      Err = SetThreadPriority(pt_id, 1);

      if (Err != 0)
      {
         printf("\nError on Keypad_Thread priority setting: %d, %d!\n\r", (int)pt_id, Err);
      }
   }

   // Critical reaction time -> consider  - FIFO mode
   pt_id = pthread_create(&TemperatureRegulator, NULL, TemperatureRegulator_service, NULL);
   delay(5);
   if (pt_id != 0)
   {
      Err = SetThreadPriority(pt_id, 50);

      if (Err != 0)
      {
         printf("\nError on TemperatureRegulator priority setting: %d, %d!\n\r", (int)pt_id, Err);
      }
   }

   // Additional measurements
   if (PigpioInit())
   {
      printf("PigpioInit not init!\n");
   }
   else
   {
   #ifdef debugmode
      printf("Pigpio inited\n\r");
   #endif
   }

   return(TRUE);
}

uint16_t SetThreadPriority(pthread_t pthread_id, uint16_t priority)
{
   uint16_t result;

   struct sched_param param;
   int policy = SCHED_FIFO; //kind of policy desired, either SCHED_FIFO or SCHED_RR, otherwise Linux uses SCHED_OTHER

   result =pthread_getschedparam(pthread_id, &policy, &param);

   printf("pthread_id: %d\n\r", (int)pthread_id);
   printf("param.sched_priority: %d\n\r", param.sched_priority);

   /* set the priority; others are unchanged */
   param.sched_priority = priority;
   /* setting the new scheduling param */
   result = pthread_setschedparam(pthread_id, policy, &param);

   return(result);
}

/******************************************************************************
function:  uint16_t K45GlobalInit(void)
parameter:
Info:
******************************************************************************/
uint16_t K45GlobalInit(void)
{
   uint16_t Result;
   boolean PowerModulconfigured; // Common SPI interface for Indicator and RTmodule

   //
   DEV_ModuleInit();

   Result = FALSE;

#if !gdb_DEBUG_config
   if (ADC_Init())
#endif
   {
      #ifdef debugmode
         printf("ADS1256 inited\n");
      #endif
   }
#if !gdb_DEBUG_config
   else
   {
      #ifdef debugmode
         printf("ADS1256 wrong\n");
      #endif
      Result = TRUE;
   }
#endif

   #ifdef RASPBERRY_PWM
      PowerModulconfigured = PWM_init();
   #else
      PowerModulconfigured = periferal_SPI1_Init();
   #endif

   if (PowerModulconfigured)
   {
      // still Result = FALSE;
   }
   else
   {
      Result = TRUE;
      #ifdef debugmode
         printf("PowerModul can`t configure ...\n");
      #endif
   }


   // Initialisation LCD via i2c connection
   lcd_Init();

   //Matrix keypad
   init_keypad();

   // UART initialisation
   if (uart_init())
   {
      #ifdef debugmode
         printf("UART wrong\n");
      #endif
      bUART_Active = FALSE;
      Result = TRUE;
   }
   else
   {
      #ifdef debugmode
         printf("UART inited\n");
      #endif
      bUART_Active = TRUE;
      // still Result = FALSE;
   }

   // FALSE - Allright
   // TRUE - something wrong
   return Result;
}

// Input Parameter - a name of the application
int main(int argc, char *argv[])
{
   uint16_t iLocalVar;
   char* ptr;

   if (argc == 2 )
   {

      for (iLocalVar = 0, ptr= argv[1]; (iLocalVar < 10) && (*ptr != '.');iLocalVar++, ptr++)
      {
         myNameApplication[iLocalVar] = *ptr;
      }

      // Rest default symbols delete
      for (; (iLocalVar < 10); iLocalVar++)
      {
         myNameApplication[iLocalVar] = ' ';
      }
      #ifdef debugmode
      printf("%s\r\n", &myNameApplication[0]);
      #endif
   }
   else
   {
      // No parameters or wrong
      // myNameApplication - is initialised yet
   }

   if (Inits())
   {
      #ifdef debugmode
      printf("%Lx\n", getSerialID());
         printf(" Inited successful\r\n");
      #else
         #ifdef PROTECTION
            if (HW_ID != getSerialID())
            {
               printf(" Wrong HW\r\n");
               K45_Exit(0);
            }
         #endif
      #endif
   }

   // All Threads stop
   pthread_join( TemperatureRegulator, NULL);
   pthread_join( Interface,            NULL);
   pthread_join( PowerEquipment,       NULL);
   pthread_join( ServiceMeasurements,  NULL);
   pthread_join( UARTCommThread,       NULL);

   return 0;
}
