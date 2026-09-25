/**
 * @file app.c
 *
 *  Created on: @date Oct 14, 2024
 *      @author Jerry Ukwela (jeu6@case.edu)
 *      @brief 	Module Applicaiton Code
 *  @defgroup RM Remote Module
 *  @{
 *  @defgroup application Module Application
 *  @brief Module Application Code
 *  @}
 *  @defgroup FESCAN FESCAN
 */

//STM Remote Module
#include "ObjDict.h"
#include "states.h"
#include "app.h"
#include "main.h"

#include "can_stm.h"
#include "sampling.h"
#include "serial_interface.h"

//Application Specific
volatile UNS8 f_syncPulse = 0;

//=============================================================================
// INTAN RHD2164 COMMAND DEFINITIONS
//=============================================================================

// Read commands
#define INTAN_READ_REG(reg)     ((uint16_t)(0xC000 | ((reg & 0x3F) << 8)))
const uint16_t READREG_I         = 0b1110100100000000;    // reg 40-41 have INTAN chip info
const uint16_t MISOABMARKER      = 0b1111101100000000;    // reg 59 is MISO A/B marker
const uint16_t READREG63         = 0b1111111100000000;    // reg 63 dummy register

// Write commands - format: 1 0 | r5 r4 r3 r2 r1 r0 | d7 d6 d5 d4 d3 d2 d1 d0
#define INTAN_WRITE_REG(reg, data)  ((uint16_t)(0x8000 | ((reg & 0x3F) << 8) | (data & 0xFF)))

// Register configurations (see RHD2164 datasheet for details)
const uint16_t REG0CONFIG   = 0b1000000011011110;    // reg 0: ADC configuration, fast settle
const uint16_t REG1CONFIG   = 0b1000000101000010;    // reg 1: ADC buffer bias current
const uint16_t REG2CONFIG   = 0b1000001000000100;    // reg 2: MUX bias current, temp sensor OFF
const uint16_t REG3CONFIG   = 0b1000001100000000;    // reg 3: MUX load, temp S1, temp S2
const uint16_t REG4CONFIG   = 0b1000010010000000;    // reg 4: Impedance check DAC
const uint16_t REG5CONFIG   = 0b1000010100000000;    // reg 5: Impedance check control (OFF)
const uint16_t REG6CONFIG   = 0b1000011010000000;    // reg 6: Impedance check DAC
const uint16_t REG7CONFIG   = 0b1000011100000000;    // reg 7: Impedance check control
const uint16_t REG8CONFIG   = 0b1000100000011000;    // reg 8: Upper bandwidth = 7.5 kHz
const uint16_t REG9CONFIG   = 0b1000100110000000;    // reg 9: Upper bandwidth
const uint16_t REG10CONFIG  = 0b1000101000010111;    // reg 10: Lower bandwidth
const uint16_t REG11CONFIG  = 0b1000101110000000;    // reg 11: Lower bandwidth
const uint16_t REG12CONFIG  = 0b1000110000101100;    // reg 12: Lower bandwidth = 1 Hz
const uint16_t REG13CONFIG  = 0b1000110110000110;    // reg 13: Lower bandwidth
const uint16_t REG14CONFIG  = 0b1000111011111111;    // reg 14: Power up channels 0-7
const uint16_t REG15CONFIG  = 0b1000111111111111;    // reg 15: Power up channels 8-15
const uint16_t REG16CONFIG  = 0b1001000011111111;    // reg 16: Power up channels 16-23
const uint16_t REG17CONFIG  = 0b1001000111111111;    // reg 17: Power up channels 24-31
const uint16_t REG18CONFIG  = 0b1001001011111111;    // reg 18: Power up channels 32-39
const uint16_t REG19CONFIG  = 0b1001001111111111;    // reg 19: Power up channels 40-47
const uint16_t REG20CONFIG  = 0b1001010011111111;    // reg 20: Power up channels 48-55
const uint16_t REG21CONFIG  = 0b1001010111111111;    // reg 21: Power up channels 56-63

// Command codes
const uint16_t CALIBRATE    = 0b0101010100000000;    // Calibrate ADC
const uint16_t CLEAR        = 0b0110101000000000;    // Clear calibration
const uint16_t CH0CONVERT   = 0b0000000000000000;    // Convert channel 0
const uint16_t CH12CONVERT  = 0b0000110000000000;    // Convert channel 12
const uint16_t CH13CONVERT  = 0b0000110100000001;    // Convert channel 13


/**
  * @brief  Function called at the initialization of program
  * @note	Object Dictionary has already been restored by this point, module type, node, and serial number have also been set
  * @param  None
  * @retval None
  * @ingroup application
  */
void initAppTask( void )
{
	MX_TIM2_Init();
	MX_USART1_UART_Init();

	RegisterSetODentryCallBack(&ObjDict_Data, 0x3A00, 0x01, &update_sampling_parameters); // OD Callback for Sampling Rate

    // Turn on power line going to Intan chip
    HAL_GPIO_WritePin( DVDD_EMG_EN_GPIO_Port, DVDD_EMG_EN_Pin, GPIO_PIN_SET );

    // Wait 500 ms for Intan power stabilization
    HAL_Delay( 500 );

    // Initialize and configure Intan RHD2164 chip
    setup_intan();

    // Start listening for USART commands (START/STOP)
    // start_command_listener();

    // Note: Sampling timer will be started when START command is received
    // To auto-start on boot, uncomment the line below:
    start_sampling_timer();
}

/**
  * @brief  Main application loop called from main.c
  * @note	main.c sleep mode causes this function to be called once per SysTick (every 1ms)
  * @param  None
  * @retval None
  * @ingroup application
  */
void updateAppTask()
{
	//This function gets called in the main.c while loop
	scan_monitor_process();
}

/**
 * @brief Initialize Intan RHD2164 chip
 * @details Configures all registers and calibrates the ADC
 * @note Must be called after Intan power is stable (500ms after enable)
 */
void setup_intan( void )
{
    // Perform two dummy reads to clear any startup garbage
    HAL_SPI_Transmit( &hspi1, (uint8_t*)&READREG63, 2, 100 );
    HAL_SPI_Transmit( &hspi1, (uint8_t*)&READREG63, 2, 100 );

    // Configure all registers (R0-R21)
    HAL_SPI_Transmit( &hspi1, (uint8_t*)&REG0CONFIG, 2, 100 );
    HAL_SPI_Transmit( &hspi1, (uint8_t*)&REG1CONFIG, 2, 100 );
    HAL_SPI_Transmit( &hspi1, (uint8_t*)&REG2CONFIG, 2, 100 );
    HAL_SPI_Transmit( &hspi1, (uint8_t*)&REG3CONFIG, 2, 100 );
    HAL_SPI_Transmit( &hspi1, (uint8_t*)&REG4CONFIG, 2, 100 );
    HAL_SPI_Transmit( &hspi1, (uint8_t*)&REG5CONFIG, 2, 100 );
    HAL_SPI_Transmit( &hspi1, (uint8_t*)&REG6CONFIG, 2, 100 );
    HAL_SPI_Transmit( &hspi1, (uint8_t*)&REG7CONFIG, 2, 100 );
    HAL_SPI_Transmit( &hspi1, (uint8_t*)&REG8CONFIG, 2, 100 );
    HAL_SPI_Transmit( &hspi1, (uint8_t*)&REG9CONFIG, 2, 100 );
    HAL_SPI_Transmit( &hspi1, (uint8_t*)&REG10CONFIG, 2, 100 );
    HAL_SPI_Transmit( &hspi1, (uint8_t*)&REG11CONFIG, 2, 100 );
    HAL_SPI_Transmit( &hspi1, (uint8_t*)&REG12CONFIG, 2, 100 );
    HAL_SPI_Transmit( &hspi1, (uint8_t*)&REG13CONFIG, 2, 100 );
    HAL_SPI_Transmit( &hspi1, (uint8_t*)&REG14CONFIG, 2, 100 );
    HAL_SPI_Transmit( &hspi1, (uint8_t*)&REG15CONFIG, 2, 100 );
    HAL_SPI_Transmit( &hspi1, (uint8_t*)&REG16CONFIG, 2, 100 );
    HAL_SPI_Transmit( &hspi1, (uint8_t*)&REG17CONFIG, 2, 100 );
    HAL_SPI_Transmit( &hspi1, (uint8_t*)&REG18CONFIG, 2, 100 );
    HAL_SPI_Transmit( &hspi1, (uint8_t*)&REG19CONFIG, 2, 100 );
    HAL_SPI_Transmit( &hspi1, (uint8_t*)&REG20CONFIG, 2, 100 );
    HAL_SPI_Transmit( &hspi1, (uint8_t*)&REG21CONFIG, 2, 100 );

    // Run ADC calibration
    HAL_SPI_Transmit( &hspi1, (uint8_t*)&CALIBRATE, 2, 100 );

    // Perform 9 dummy reads to allow calibration to complete
    for (int i = 0; i < 9; i++)
    {
        HAL_SPI_Transmit( &hspi1, (uint8_t*)&READREG63, 2, 100 );
    }

    // Prime the conversion pipeline with two channel 0 conversions
    HAL_SPI_Transmit( &hspi1, (uint8_t*)&CH0CONVERT, 2, 100 );
    HAL_SPI_Transmit( &hspi1, (uint8_t*)&CH0CONVERT, 2, 100 );
}

/**
 * @brief Lets application process SYNC message if necessary
 * @param *m 8 byte message sent with SYNC objects from PM
 * @ingroup application
 * @note Changes made to the Object Dictionary in this function will be present in the response to the network
 */
void processSYNCMessageForApp(Message* m)
{
	// syncPulse is included as an example of setting a flag
	// to be used in the app task and is not required for functionality
	f_syncPulse = 1;
}

/**
 * @brief This function is called when the module recieves an NMT to enter low power mode
 * @param none
 * @ingroup application
 */
void sleepApplication(void)
{

}

/**
  * @brief  Reinitializes application values when necessary
  * @note	Runs when node is stopped, entering waiting mode, entering stop stim mode
  * @param  None
  * @retval None
  * @ingroup application
  */
void InitAppTaskValues(void)
{
	f_syncPulse = 0;
}

/**
 * @brief Lets application process NMT_Start_Nodes (unused)
 * @param *d  Pointer to the CAN data structure
 * @param *m 3 byte message sent with NMT objects from PM
 * @ingroup CANFestival FESCAN
 */
void StartNodesFunc(CO_Data* d, Message *m)
{
  if ( d->nodeState == Hibernate)
    setState(d,Waiting);
  else
    setState(d,Unknown_state);
}

/**
 * @brief Lets application process NMT_Stop_Nodes
 * @param *d  Pointer to the CAN data structure
 * @param *m 3 byte message sent with NMT objects from PM
 * @ingroup CANFestival FESCAN
 */
void StopNodesFunc(CO_Data* d, Message *m)
{
  //no restrictions on state entry
  setState(d,Stopped);
  InitAppTaskValues();
}

/**
 * @brief Lets application process NMT_Enter_Waiting
 * @param *d  Pointer to the CAN data structure
 * @param *m 3 byte message sent with NMT objects from PM
 * @ingroup CANFestival FESCAN
 */
void EnterWaitingFunc(CO_Data* d, Message *m)
{
  //no restrictions on state entry
  setState(d,Waiting);
  StopWatchDog( d );
  InitAppTaskValues();
}

/**
 * @brief Lets application process NMT_Enter_Patient_Operation
 * @param *d  Pointer to the CAN data structure
 * @param *m 3 byte message sent with NMT objects from PM
 * @ingroup CANFestival FESCAN
 */
void EnterPatientOperationFunc(CO_Data* d, Message *m)
{
  if (d->nodeState == Waiting && (*m).data[1] == 0) //must be NMT broadcast and in waiting
  {
    setState(d, Mode_Patient_Control);
  }
}

/**
 * @brief Lets application process NMT_Enter_X_Manual
 * @param *d  Pointer to the CAN data structure
 * @param *m 3 byte message sent with NMT objects from PM
 * @ingroup CANFestival FESCAN
 */
void EnterXManualFunc(CO_Data* d, Message *m)
{
  if (d->nodeState == Waiting) //must be in waiting
  {
    setState(d, Mode_X_Manual);
    StartWatchDog(d, 10000);
  }
}

/**
 * @brief Lets application process NMT_Enter_Y_Manual
 * @param *d  Pointer to the CAN data structure
 * @param *m 3 byte message sent with NMT objects from PM
 * @ingroup CANFestival FESCAN
 */
void EnterYManualFunc(CO_Data* d, Message *m)
{
  if (d->nodeState == Waiting) //must be in waiting
  {
    setState(d, Mode_Y_Manual);
    StartWatchDog(d, 10000);
  }
}

/**
 * @brief Lets application process NMT_Enter_Stop_Stim
 * @param *d  Pointer to the CAN data structure
 * @param *m 3 byte message sent with NMT objects from PM
 * @ingroup CANFestival FESCAN
 */
void EnterStopStimFunc(CO_Data* d, Message *m)
{
  //no restrictions on state entry
  setState(d, Stopped);
  InitAppTaskValues();
}

/**
 * @brief Lets application process NMT_Enter_Patient_Manual
 * @param *d  Pointer to the CAN data structure
 * @param *m 3 byte message sent with NMT objects from PM
 * @ingroup CANFestival FESCAN
 */
void EnterPatientManualFunc(CO_Data* d, Message *m)
{
  if (d->nodeState == Waiting && (*m).data[1] == 0) //must be NMT broadcast and in waiting
  {
    setState(d, Mode_Patient_Manual);
    StartWatchDog(d, 10000);
  }
}

/**
 * @brief Lets application process NMT_Produce_X_Manual
 * @param *d  Pointer to the CAN data structure
 * @param *m 3 byte message sent with NMT objects from PM
 * @ingroup CANFestival FESCAN
 */
void EnterProduceXManualFunc(CO_Data* d, Message *m)
{
  if (d->nodeState == Waiting && (*m).data[1] == 0) //must be NMT broadcast and in waiting
  {
     setState(d, Mode_Produce_X_Manual);
  }
}

/**
 * @brief Lets application process NMT_Enter_Record_X
 * @param *d  Pointer to the CAN data structure
 * @param *m 3 byte message sent with NMT objects from PM
 * @ingroup CANFestival FESCAN
 */
void EnterRecordXFunc(CO_Data* d, Message *m)
{
  if (d->nodeState == Waiting)  //must be in waiting
  {
    setState(d, Mode_Record_X);
  }
}
