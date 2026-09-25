/// @file       serial_interface.c
/// @brief      UART transmission routines - IMPROVED VERSION
/// @date       January 29, 2025
///
/// IMPROVEMENTS:
/// - FIX #1: Removed triple transmission bug (now sends data once)
/// - FIX #2: Added DMA flow control with uart_tx_busy flag
/// - FIX #5: Increased baudrate to 12 Mbaud for higher throughput
/// - Removed all blocking HAL_UART_Transmit calls
/// - Better error handling

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "main.h"
#include "serial_interface.h"
#include "stm32u5xx_hal.h"

#include "ObjDict.h"

//=============================================================================
// PRIVATE VARIABLES
//=============================================================================

UART_HandleTypeDef huart1;

// UART transmission state flag (set by user, cleared by ISR callback)
volatile bool uart_tx_busy = false;

// Command reception buffer and state
#define CMD_BUFFER_SIZE 4
static uint8_t cmd_rx_buffer[CMD_BUFFER_SIZE];
static volatile bool transmission_enabled = false;
static volatile bool command_received = false;

// Command definitions (simple 4-byte commands)
static const uint8_t CMD_START[4] = {'S', 'T', 'R', 'T'};  // "STRT"
static const uint8_t CMD_STOP[4]  = {'S', 'T', 'O', 'P'};  // "STOP"
static const uint8_t  Single_Channel = 0;

// Binary stream frame counter (little-endian in header bytes [2:3])
static uint16_t uart_frame_counter = 0;

/**
 * @brief Build sync header + sample payload and kick DMA TX
 * @details Packet layout: AA 55 | frame_counter (LE) | N x uint16 samples (LE)
 *          Static TX buffer is safe because process_scan() only queues when
 *          uart_tx_busy is false (one in-flight DMA at a time).
 */
static HAL_StatusTypeDef transmit_binary_frame( const uint16_t* samples, int n_channels )
{
	static uint8_t tx_packet[ UART_FRAME_HEADER_BYTES + ( UART_MAX_CHANNELS * sizeof(uint16_t) ) ];

	if ( n_channels < 1 || n_channels > UART_MAX_CHANNELS )
	{
		return HAL_ERROR;
	}

	tx_packet[0] = UART_SYNC0;
	tx_packet[1] = UART_SYNC1;
	tx_packet[2] = (uint8_t)( uart_frame_counter & 0xFFu );
	tx_packet[3] = (uint8_t)( ( uart_frame_counter >> 8 ) & 0xFFu );
	uart_frame_counter++;

	memcpy( tx_packet + UART_FRAME_HEADER_BYTES,
	        samples,
	        (size_t)n_channels * sizeof(uint16_t) );

	return HAL_UART_Transmit_DMA( &huart1,
	                              tx_packet,
	                              (uint16_t)( UART_FRAME_HEADER_BYTES + (size_t)n_channels * sizeof(uint16_t) ) );
}

//=============================================================================
// PUBLIC FUNCTIONS
//=============================================================================

/**
 * @brief Transmit samples from contiguous buffer
 * @details Used for averaged data or when buffers are already combined
 */
void transmit_samples( uint16_t* sample_buffer, int n_channels )
{
	if(TransmitASCII)
	{
		char uart_buf[ 700 ];
		int total_len = 0;

		// Format as "$sample0 sample1 sample2 ... sampleN;"
		total_len += sprintf( uart_buf + total_len, "$%u", sample_buffer[0] );

		for ( int i = 1; i < n_channels; i++ )
		{
			total_len += sprintf( uart_buf + total_len, " %u", sample_buffer[i] );
		}

		total_len += sprintf( uart_buf + total_len, ";" );

		HAL_UART_Transmit_DMA( &huart1, (uint8_t*)uart_buf, total_len );

	}
	else
	{
		HAL_StatusTypeDef status = transmit_binary_frame( sample_buffer, n_channels );

		if ( status != HAL_OK )
		{
			// Transmission failed - UART may be busy or error occurred
			uart_tx_busy = false;  // Clear flag to allow retry
		}
	}
}

/**
 * @brief Transmit samples from two separate buffers
 * - Combine buffers into single framed packet
 * - Send ONCE via DMA (header + 128 sample bytes for 64 ch)
 */
void transmit_samples_two( uint16_t* buffer_a, uint16_t* buffer_b, int n_channels_each )
{
	if(TransmitASCII)
	{
		// ASCII mode: fall back to contiguous transmit
		    static uint16_t tmp_buffer[ 64 ];
		    memcpy( tmp_buffer, buffer_a, n_channels_each * sizeof(uint16_t) );
		    memcpy( tmp_buffer + n_channels_each, buffer_b, n_channels_each * sizeof(uint16_t) );
		    transmit_samples( tmp_buffer, n_channels_each * 2 );
	}
	else
	{
		static uint16_t sample_frame[ UART_MAX_CHANNELS ];
		int total_channels = 2 * n_channels_each;

		if ( total_channels > UART_MAX_CHANNELS )
		{
			uart_tx_busy = false;
			return;
		}

		// Combine both buffers into contiguous sample payload
		memcpy( sample_frame, buffer_a, (size_t)n_channels_each * sizeof(uint16_t) );
		memcpy( sample_frame + n_channels_each, buffer_b, (size_t)n_channels_each * sizeof(uint16_t) );

		HAL_StatusTypeDef status = transmit_binary_frame( sample_frame, total_channels );

		if ( status != HAL_OK )
		{
			// Transmission failed
			uart_tx_busy = false;  // Clear flag to allow retry
		}
	}
}

void transmit_samples_three( uint16_t* sample_buffer, int n_channels )
{
#if USE_ASCII
    char uart_buf[ 700 ];
    int uart_buf_len;
    int total_buffer_len;
#endif

#if USE_ASCII
    total_buffer_len = 0;

    uart_buf_len      = sprintf( uart_buf + total_buffer_len, "$%u", *( sample_buffer + 0 ) );
    total_buffer_len += uart_buf_len;

    for ( int i = 1; i < n_channels; i++ )
    {
        uart_buf_len      = sprintf( uart_buf + total_buffer_len, " %u", *( sample_buffer + i ) );
        total_buffer_len += uart_buf_len;
    }
    uart_buf_len      = sprintf( uart_buf + total_buffer_len, ";" );
    total_buffer_len += uart_buf_len;

    HAL_UART_Transmit_DMA( &huart1, (uint8_t*)uart_buf, total_buffer_len );
#else
    static uint8_t xmit_buffer[ 80 ];
    memcpy( xmit_buffer, sample_buffer, n_channels );
    HAL_UART_Transmit_DMA( &huart1, xmit_buffer, n_channels);

#endif
}

/**
 * @brief Check if UART is busy
 */
bool is_uart_busy( void )
{
    return uart_tx_busy;
}

/**
 * @brief Start listening for USART commands
 * @details Initiates interrupt-driven reception for START/STOP commands
 */
void start_command_listener( void )
{
    // Start receiving commands via UART interrupt
    HAL_UART_Receive_IT(&huart1, cmd_rx_buffer, CMD_BUFFER_SIZE);
}

/**
 * @brief Check if transmission should be enabled
 * @return true if START command received, false if STOP command received
 */
bool check_command_status( void )
{
    return transmission_enabled;
}

/**
 * @brief UART RX Complete Callback
 * @details Called when a command is received over UART
 * Processes START/STOP commands and restarts reception
 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART1)
    {
        // Check for START command
        if (memcmp(cmd_rx_buffer, CMD_START, CMD_BUFFER_SIZE) == 0)
        {
            transmission_enabled = true;
            command_received = true;
        }
        // Check for STOP command
        else if (memcmp(cmd_rx_buffer, CMD_STOP, CMD_BUFFER_SIZE) == 0)
        {
            transmission_enabled = false;
            command_received = true;
        }

        // Restart reception for next command
        HAL_UART_Receive_IT(&huart1, cmd_rx_buffer, CMD_BUFFER_SIZE);
    }
}

/**
 * @brief UART TX Complete Callback
 * @details Called by DMA interrupt when UART transmission finishes
 *
 * CRITICAL FIX #2: This callback is essential for UART flow control
 * Without it, uart_tx_busy flag is never cleared, causing all subsequent
 * transmissions to fail silently.
 *
 * @param huart Pointer to UART handle that completed transmission
 */
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART1)
    {
        // Clear busy flag - UART is now ready for next transmission
        uart_tx_busy = false;
    }
}


/**
 * @brief USART1 Initialization Function
 * @details Configures UART at 12 Mbaud with FIFO enabled
 *
 * IMPROVED: Baudrate increased from 3 Mbaud to 12 Mbaud
 */
void MX_USART1_UART_Init( void )
{
    huart1.Instance                    = USART1;
    huart1.Init.BaudRate               = UART_BAUDRATE;  // 10 Mbaud
    huart1.Init.WordLength             = UART_WORDLENGTH_8B;
    huart1.Init.StopBits               = UART_STOPBITS_1;
    huart1.Init.Parity                 = UART_PARITY_NONE;
    huart1.Init.Mode                   = UART_MODE_TX_RX;  // Enable both TX and RX
    huart1.Init.HwFlowCtl              = UART_HWCONTROL_NONE;
    huart1.Init.OverSampling           = UART_OVERSAMPLING_16;
    huart1.Init.OneBitSampling         = UART_ONE_BIT_SAMPLE_DISABLE;
    huart1.Init.ClockPrescaler         = UART_PRESCALER_DIV1;
    huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;

    if ( HAL_UART_Init( &huart1 ) != HAL_OK )
    {
        Error_Handler();
    }

    // Enable FIFO mode for better DMA efficiency at high baud rates
    if ( HAL_UARTEx_EnableFifoMode( &huart1 ) != HAL_OK )
    {
        Error_Handler();
    }

    // Set FIFO thresholds
    if ( HAL_UARTEx_SetTxFifoThreshold( &huart1, UART_TXFIFO_THRESHOLD_1_2 ) != HAL_OK )
    {
        Error_Handler();
    }

    if ( HAL_UARTEx_SetRxFifoThreshold( &huart1, UART_RXFIFO_THRESHOLD_1_2 ) != HAL_OK )
    {
        Error_Handler();
    }
}

void HAL_UART_MspInit(UART_HandleTypeDef *huart){
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

	PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART1;
	PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_SYSCLK;
	if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
	{
	  Error_Handler();
	}

	/* Peripheral clock enable */
	__HAL_RCC_USART1_CLK_ENABLE();

	__HAL_RCC_GPIOA_CLK_ENABLE();
	/**USART1 GPIO Configuration
	PA9     ------> USART1_TX
	PA10     ------> USART1_RX
	*/
	GPIO_InitStruct.Pin = GPIO_PIN_9|GPIO_PIN_10;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	GPIO_InitStruct.Alternate = GPIO_AF7_USART1;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* USART1 DMA Init */
    /* GPDMA1_REQUEST_USART1_TX Init */
    handle_GPDMA1_Channel8.Instance = GPDMA1_Channel8;
    handle_GPDMA1_Channel8.Init.Request = GPDMA1_REQUEST_USART1_TX;
    handle_GPDMA1_Channel8.Init.BlkHWRequest = DMA_BREQ_SINGLE_BURST;
    handle_GPDMA1_Channel8.Init.Direction = DMA_MEMORY_TO_PERIPH;
    handle_GPDMA1_Channel8.Init.SrcInc = DMA_SINC_INCREMENTED;
    handle_GPDMA1_Channel8.Init.DestInc = DMA_DINC_FIXED;
    handle_GPDMA1_Channel8.Init.SrcDataWidth = DMA_SRC_DATAWIDTH_BYTE;
    handle_GPDMA1_Channel8.Init.DestDataWidth = DMA_DEST_DATAWIDTH_BYTE;
    handle_GPDMA1_Channel8.Init.Priority = DMA_LOW_PRIORITY_LOW_WEIGHT;
    handle_GPDMA1_Channel8.Init.SrcBurstLength = 1;
    handle_GPDMA1_Channel8.Init.DestBurstLength = 1;
    handle_GPDMA1_Channel8.Init.TransferAllocatedPort = DMA_SRC_ALLOCATED_PORT0|DMA_DEST_ALLOCATED_PORT0;
    handle_GPDMA1_Channel8.Init.TransferEventMode = DMA_TCEM_BLOCK_TRANSFER;
    handle_GPDMA1_Channel8.Init.Mode = DMA_NORMAL;
    if (HAL_DMA_Init(&handle_GPDMA1_Channel8) != HAL_OK)
    {
      Error_Handler();
    }

    __HAL_LINKDMA(huart, hdmatx, handle_GPDMA1_Channel8);

    if (HAL_DMA_ConfigChannelAttributes(&handle_GPDMA1_Channel8, DMA_CHANNEL_NPRIV) != HAL_OK)
    {
      Error_Handler();
    }

    /* USART1 interrupt Init */
    HAL_NVIC_SetPriority(USART1_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(USART1_IRQn);
}
