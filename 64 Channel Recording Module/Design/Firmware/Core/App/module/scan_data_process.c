/// @file       scan_data_process.c
/// @brief      Multi-mode data processing and transmission
/// @date       January 29, 2025

#include <stdio.h>
#include <string.h>

#include "gpio_control.h"
#include "scan_data_process.h"
#include "serial_interface.h"
#include "sampling_config.h"

#include "ObjDict.h"

//=============================================================================
// PRIVATE VARIABLES
//=============================================================================

extern volatile bool uart_tx_busy;

static uint32_t channel_sums[ ACTIVE_TOTAL_CHANNELS ] = { 0 };
static uint16_t channel_averages[ ACTIVE_TOTAL_CHANNELS ] = { 0 };
static int current_avg_count = 0;

#if CFG_SINGLE_CHANNEL_MODE
/** Discard first N scans so Intan CONVERT pipeline is filled */
static uint8_t pipeline_skip_remaining = INTAN_PIPELINE_SKIP_SCANS;
static uint16_t single_tx_sample = 0;

/* DEBUG: set to 0 (or delete this block) after reviewing raw samples.
 * Watch dbg_raw_log_done → 1, then inspect dbg_raw_log[0..255] once. */
#define DBG_RAW_LOG_ENABLE  0
#if DBG_RAW_LOG_ENABLE
#define DBG_RAW_LOG_N       2048
volatile uint16_t dbg_raw_log[DBG_RAW_LOG_N];
volatile uint16_t dbg_raw_log_i = 0;
volatile uint8_t  dbg_raw_log_done = 0;
#endif
#endif

//=============================================================================
// PUBLIC FUNCTIONS
//=============================================================================

void reset_scan_pipeline( void )
{
#if CFG_SINGLE_CHANNEL_MODE
	pipeline_skip_remaining = INTAN_PIPELINE_SKIP_SCANS;
#if DBG_RAW_LOG_ENABLE
	dbg_raw_log_i = 0;
	dbg_raw_log_done = 0;
#endif
#endif
}

/**
 * @brief Process a completed scan with variable channel support
 */
void process_scan( uint16_t* buffer_a, uint16_t* buffer_b, int n_channels )
{
    SamplesProcessedCount++;

#if CFG_SINGLE_CHANNEL_MODE
	(void)n_channels;

	/* Snapshot settled word immediately (before UART work / next TIM→SPI DMA). */
#if CFG_USE_MISO_A
	single_tx_sample = buffer_a[SPI_RESULT_WORD_INDEX];
	(void)buffer_b;
#else
	single_tx_sample = buffer_b[SPI_RESULT_WORD_INDEX];
	(void)buffer_a;
#endif

	if ( pipeline_skip_remaining > 0 )
	{
		pipeline_skip_remaining--;
		return;
	}

#if DBG_RAW_LOG_ENABLE
	if ( !dbg_raw_log_done )
	{
		dbg_raw_log[dbg_raw_log_i++] = single_tx_sample;
		if ( dbg_raw_log_i >= DBG_RAW_LOG_N )
		{
			dbg_raw_log_done = 1;
		}
	}
#endif

	if ( uart_tx_busy )
	{
		SamplesDroppedCount++;
		return;
	}

	uart_tx_busy = true;
	transmit_samples( &single_tx_sample, 1 );
	SamplesTransmittedCount++;
	return;
#else
    // Check UART availability before attempting transmission
    if ( uart_tx_busy )
    {
        SamplesDroppedCount++;
        return;
    }

	if(OversampleRatio == 1){
		uart_tx_busy = true;

		if(INTAN_BankBEnabled){
			transmit_samples_two( buffer_a, buffer_b, n_channels );
		}
		else{
			transmit_samples( buffer_a, n_channels );
		}

		SamplesTransmittedCount++;
	}
	else
	{
		if ( current_avg_count == 0 )
		{
			memset( channel_sums, 0, sizeof(channel_sums) );
		}

		for ( int i = 0; i < n_channels; i++ )
		{
			channel_sums[ i ] += buffer_a[ i ];
		}

		if(INTAN_BankBEnabled){
			for ( int i = 0; i < n_channels; i++ )
			{
				channel_sums[ i + n_channels ] += buffer_b[ i ];
			}
		}

		current_avg_count++;

		if ( current_avg_count >= OversampleRatio )
		{
			int total_channels = INTAN_ActiveChannels;

			for ( int i = 0; i < total_channels; i++ )
			{
				if((OversampleRatio & (OversampleRatio - 1)) == 0)
					channel_averages[ i ] = (uint16_t)( channel_sums[ i ] >> __builtin_ctz( OversampleRatio) );
				else
					channel_averages[ i ] = (uint16_t)( channel_sums[ i ] / OversampleRatio );
			}

			current_avg_count = 0;
			uart_tx_busy = true;
			transmit_samples( channel_averages, total_channels );
			SamplesTransmittedCount++;
		}
	}
#endif /* CFG_SINGLE_CHANNEL_MODE */
}

void reset_scan_statistics( void )
{
    SamplesProcessedCount = 0;
    SamplesTransmittedCount = 0;
    SamplesDroppedCount = 0;
}
