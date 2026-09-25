/// @file       scan_data_process_IMPROVED.h
/// @brief      Multi-mode data processing and transmission - IMPROVED VERSION
/// @date       January 29, 2025
///
/// IMPROVEMENTS:
/// - Integrated with sampling_config.h for variable channel support
/// - Dynamic buffer sizing based on active channel count
/// - Flow control with UART busy checking
/// - Statistics tracking for debugging and performance monitoring

#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "sampling_config.h"

//=============================================================================
// PUBLIC FUNCTIONS
//=============================================================================

/**
 * @brief Process a completed scan from active MISO streams
 * @details Combines data from both MISO streams (if active) and transmits via UART.
 *          Implements flow control to prevent data loss when UART is busy.
 *          Buffer sizes automatically adjust based on sampling_config.h mode.
 *
 * @param buffer_a Pointer to MISO A channel data
 * @param buffer_b Pointer to MISO B channel data (may be unused in single-channel mode)
 * @param n_channels Number of channels per stream (ACTIVE_CHANNELS_PER_STREAM)
 */
void process_scan( uint16_t* buffer_a, uint16_t* buffer_b, int n_channels );

/**
 * @brief Reset statistics counters to zero
 */
void reset_scan_statistics( void );

/**
 * @brief Reset Intan CONVERT pipeline skip / debug log (single-channel mode)
 * @details Call when starting/restarting acquisition.
 */
void reset_scan_pipeline( void );
