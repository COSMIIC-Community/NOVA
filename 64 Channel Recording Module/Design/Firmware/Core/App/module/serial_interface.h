/// @file       serial_interface.h
/// @brief      UART initialization and transmission routines - IMPROVED VERSION
/// @date       January 29, 2025
///
/// IMPROVEMENTS:
/// - Increased baudrate to 12 Mbaud
/// - Proper flow control with DMA busy flag
/// - Better documentation

#pragma once

#include <stdint.h>
#include <stdbool.h>

//=============================================================================
// CONFIGURATION
//=============================================================================

// UART baudrate (adjusted for 160 MHz CPU clock)
#ifndef UART_BAUDRATE
#define UART_BAUDRATE 3000000
#endif

// Binary frame header: AA 55 | uint16 LE frame counter | N x uint16 LE samples
#define UART_SYNC0              0xAAu
#define UART_SYNC1              0x55u
#define UART_FRAME_HEADER_BYTES 4
#define UART_MAX_CHANNELS       64

//// Output format selection
//#ifndef USE_ASCII
//#define USE_ASCII 0
//#endif
//
//#ifndef USE_BINARY
//#define USE_BINARY 1
//#endif

//=============================================================================
// EXTERNAL HANDLES
//=============================================================================

extern UART_HandleTypeDef huart1;

//=============================================================================
// PUBLIC FUNCTIONS
//=============================================================================

/**
 * @brief Initialize USART1 for data transmission
 * @details Configures UART at 12 Mbaud with FIFO mode enabled
 */
void MX_USART1_UART_Init( void );

/**
 * @brief Transmit samples from contiguous buffer
 * @param sample_buffer Pointer to sample array
 * @param n_channels Number of channels (samples) in buffer
 * @note Used when data is already in a single contiguous buffer
 */
void transmit_samples( uint16_t* sample_buffer, int n_channels );

/**
 * @brief Transmit samples from two separate buffers (zero-copy)
 * @details IMPROVED: Fixed to send data only once (was sending 3 times)
 *
 * @param buffer_a Pointer to first buffer (MISO A data)
 * @param buffer_b Pointer to second buffer (MISO B data)
 * @param n_channels_each Number of channels in each buffer
 */
void transmit_samples_two( uint16_t* buffer_a, uint16_t* buffer_b, int n_channels_each );

void transmit_samples_three( uint16_t* sample_buffer, int n_channels );
/**
 * @brief Check if UART transmission is in progress
 * @return true if UART is busy transmitting
 */
bool is_uart_busy( void );

//=============================================================================
// COMMAND INTERFACE
//=============================================================================

/**
 * @brief Start listening for USART commands
 * @details Initiates interrupt-driven reception for START/STOP commands
 */
void start_command_listener( void );

/**
 * @brief Check if a valid command has been received
 * @return true if START command received, false if STOP command received
 * @note Returns current state if no command pending
 */
bool check_command_status( void );
