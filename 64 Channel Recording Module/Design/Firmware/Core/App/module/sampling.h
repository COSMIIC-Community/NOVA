/// @file       sampling_IMPROVED.h
/// @brief      Multi-mode sampling control with variable channel support
/// @date       January 29, 2025
///

#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "sampling_config.h"  // Centralized configuration

//=============================================================================
// EXTERNAL HANDLES
//=============================================================================

extern TIM_HandleTypeDef htim2;

//=============================================================================
// PUBLIC FUNCTIONS
//=============================================================================

/**
 * @brief Initialize TIM2 for sampling rate generation
 * @details Configures timer based on compile-time sampling mode selection:
 *          - SINGLE_CHANNEL_MODE:  1 channel  @ 300 kHz
 *          - EIGHT_CHANNEL_MODE:   8 channels @ 37.5 kHz
 *          - MEDIUM_CHANNEL_MODE: 32 channels @ 9.4 kHz
 *          - FULL_CHANNEL_MODE:   64 channels @ 4.7 kHz
 *
 *          Timer prescaler and period are automatically configured from
 *          sampling_config.h based on 160 MHz CPU clock.
 */
void MX_TIM2_Init( void );

/**
 * @brief Start the sampling timer in interrupt mode
 * @details Timer interrupts will trigger SPI DMA transfers at configured rate.
 *          Frequency depends on selected channel mode in sampling_config.h.
 */
void start_sampling_timer( void );

/**
 * @brief Pause the sampling timer
 * @details Does not pause DMA transfers already in progress
 */
void pause_sampling_timer( void );

/**
 * @brief Stop data acquisition completely
 * @details Stops the sampling timer and aborts any ongoing SPI/DMA transfers.
 *          Safe to call multiple times. Use this when stopping transmission.
 */
void stop_data_acquisition( void );

/**
 * @brief Restart data acquisition
 * @details Restarts the sampling timer and SPI/DMA transfers.
 *          Safe to call after stop_data_acquisition(). Use when resuming transmission.
 */
void restart_data_acquisition( void );

/**
 * @brief Start SPI DMA transfers for selected channels
 * @details Initiates DMA reception on active MISO streams and transmits
 *          only the necessary commands to read selected channels.
 */
void start_spi_transfers( void );

/**
 * @brief Monitor and process completed scans
 * @details Call this repeatedly from main loop. When a complete scan is ready,
 *          data from both MISO streams will be combined and transmitted via UART.
 *          Buffer sizes automatically adjust based on active channel count.
 */
void scan_monitor_process( void );

/**
 * @brief Get the configured sample rate in Hz
 * @return Sample rate in samples per second (per channel)
 */
uint32_t get_sample_rate_hz( void );

/**
 * @brief Get the total number of active channels
 * @return Number of active channels (1, 8, 32, or 64)
 */
uint32_t get_active_channel_count( void );

/**
 * @brief Get the current operating mode name
 * @return String describing current mode (e.g., "SINGLE (1ch @ 300kHz)")
 */
const char* get_mode_name( void );

/**
 * @brief Callback from the Object Dictionary whenever the Configuration Data index is
 * @return Success on 0
 */
UNS32 update_sampling_parameters(CO_Data* d, const indextable * pIndexTable, UNS8 bSubindex);
