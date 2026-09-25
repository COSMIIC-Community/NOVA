/// @file       sampling.c
/// @brief      Multi-mode sampling with variable channel support
/// @date       January 29, 2025

#include <stdbool.h>
#include <stdint.h>

#include "stm32u5xx_hal.h"

#include "gpio_control.h"
#include "main.h"
#include "sampling.h"
#include "scan_data_process.h"
#include "sampling_config.h"

#include "ObjDict.h"


extern SPI_HandleTypeDef hspi1;
extern SPI_HandleTypeDef hspi2;
extern SPI_HandleTypeDef hspi3;

TIM_HandleTypeDef htim2;

// Flags indicating DMA completion
static bool miso_a_ready = false;
static bool miso_b_ready = false;
static bool scan_complete = false;

// Performance counters
static uint32_t total_scans_completed = 0;
static bool tx_buffer_initialized = false;

// Debug counters
static uint32_t debug_spi2_callbacks = 0;
static uint32_t debug_spi3_callbacks = 0;
static uint32_t debug_transfers_started = 0;

/**
 * @brief Generate Intan CONVERT commands for selected channels
 * @details Creates command buffer with CONVERT commands only for active channels.
 * @param cmd_buffer Output buffer for commands
 * @param start_channel First channel to convert (0-31)
 * @param count Number of channels to convert
 */
static void generate_intan_commands(uint16_t* cmd_buffer, uint8_t start_channel, uint8_t count)
{
    for (uint8_t i = 0; i < count; i++)
    {
        uint8_t channel = (uint8_t)(start_channel + i);
        // Intan CONVERT command format: 00 CCCCCC 00000000
        cmd_buffer[i] = (uint16_t)(channel << 8);
    }
}

//=============================================================================
// PUBLIC FUNCTIONS
//=============================================================================

/**
 * @brief Start SPI DMA transfers for selected channels
 */
void start_spi_transfers( void )
{
    static uint16_t tx_command_buffer[ SPI_WORDS_PER_SCAN ] = { 0 };

    // Streams not used this mode are treated as already complete
    miso_a_ready = (CFG_USE_MISO_A == 0);
    miso_b_ready = (CFG_USE_MISO_B == 0);

    if ( !tx_buffer_initialized )
    {
#if CFG_SINGLE_CHANNEL_MODE
        /* Same CONVERT(ch) repeated — do NOT use generate_intan_commands()
         * (that would emit CONVERT(ch), CONVERT(ch+1), CONVERT(ch+2)). */
        {
            const uint16_t cmd = (uint16_t)((uint8_t)CFG_CHIP_CHANNEL << 8);
            for ( uint8_t i = 0; i < SPI_WORDS_PER_SCAN; i++ )
            {
                tx_command_buffer[i] = cmd;
            }
        }
#else
        generate_intan_commands(tx_command_buffer, START_CHANNEL_A, SPI_WORDS_PER_SCAN);
#endif
        tx_buffer_initialized = true;
    }

    debug_transfers_started++;

    HAL_StatusTypeDef status;

#if CFG_USE_MISO_A
    status = HAL_SPI_Receive_DMA( &hspi2,
                                   (uint8_t*)INTAN_BufferA,
                                   SPI_WORDS_PER_SCAN );
    if (status != HAL_OK)
    {
        Error_Handler();
    }
#endif

#if CFG_USE_MISO_B
    status = HAL_SPI_Receive_DMA( &hspi3,
                                   (uint8_t*)INTAN_BufferB,
                                   SPI_WORDS_PER_SCAN );
    if (status != HAL_OK)
    {
        Error_Handler();
    }
#endif

    status = HAL_SPI_Transmit_DMA( &hspi1,
                                    (uint8_t*)tx_command_buffer,
                                    SPI_WORDS_PER_SCAN );
    if (status != HAL_OK)
    {
        Error_Handler();
    }
}

/**
 * @brief Monitor for completed scans and trigger processing
 */
void scan_monitor_process( void )
{
    if ( scan_complete )
    {
        scan_complete = false;
        total_scans_completed++;

        if(total_scans_completed % 500 > 250)
        	HAL_GPIO_TogglePin(PC4_LED_GPIO_Port, PC4_LED_Pin);

        process_scan( INTAN_BufferA, INTAN_BufferB, SPI_WORDS_PER_SCAN );
    }
}

uint32_t get_sample_rate_hz( void )
{
    return CFG_SAMPLE_RATE_HZ;
}

uint32_t get_active_channel_count( void )
{
    return ACTIVE_TOTAL_CHANNELS;
}

void get_spi_debug_info( void )
{
    volatile uint32_t transfers = debug_transfers_started;
    volatile uint32_t spi2_cb = debug_spi2_callbacks;
    volatile uint32_t spi3_cb = debug_spi3_callbacks;
    volatile HAL_SPI_StateTypeDef spi1_state = hspi1.State;
    volatile HAL_SPI_StateTypeDef spi2_state = hspi2.State;
    volatile HAL_SPI_StateTypeDef spi3_state = hspi3.State;
    volatile uint32_t spi1_error = hspi1.ErrorCode;
    volatile uint32_t spi2_error = hspi2.ErrorCode;
    volatile uint32_t spi3_error = hspi3.ErrorCode;

    (void)transfers;
    (void)spi2_cb;
    (void)spi3_cb;
    (void)spi1_state;
    (void)spi2_state;
    (void)spi3_state;
    (void)spi1_error;
    (void)spi2_error;
    (void)spi3_error;
}

void start_sampling_timer( void )
{
    reset_scan_pipeline();
    if ( HAL_TIM_Base_Start_IT( &htim2 ) != HAL_OK )
    {
        Error_Handler();
    }
}

void pause_sampling_timer( void )
{
    if ( HAL_TIM_Base_Stop_IT( &htim2 ) != HAL_OK )
    {
        Error_Handler();
    }
}

void stop_data_acquisition( void )
{
    HAL_TIM_Base_Stop_IT( &htim2 );

    HAL_SPI_Abort( &hspi1 );
    HAL_SPI_Abort( &hspi2 );
    HAL_SPI_Abort( &hspi3 );

    miso_a_ready = false;
    miso_b_ready = false;
    scan_complete = false;
}

void restart_data_acquisition( void )
{
    miso_a_ready = false;
    miso_b_ready = false;
    scan_complete = false;
    tx_buffer_initialized = false;
    reset_scan_pipeline();

    if ( HAL_TIM_Base_Start_IT( &htim2 ) != HAL_OK )
    {
        Error_Handler();
    }
}

//=============================================================================
// CALLBACKS
//=============================================================================

void HAL_SPI_RxCpltCallback( SPI_HandleTypeDef* hspi )
{
    if ( hspi->Instance == SPI2 )
    {
        debug_spi2_callbacks++;
        miso_a_ready = true;
    }
    else if ( hspi->Instance == SPI3 )
    {
        debug_spi3_callbacks++;
        miso_b_ready = true;
    }

    if ( miso_a_ready && miso_b_ready )
    {
        miso_a_ready = false;
        miso_b_ready = false;
        scan_complete = true;
    }
}

UNS32 update_sampling_parameters(CO_Data* d, const indextable * pIndexTable, UNS8 bSubindex){
	switch(bSubindex){
	case 1:
		if(INTAN_SampleRate > 1 && INTAN_SampleRate < 50000)
		{
		}
		else
		{
			INTAN_SampleRate = 1000;
		}

		TIM2->ARR = 1000000/INTAN_SampleRate;
		break;
	case 2:
		tx_buffer_initialized = false;
		break;
	default:
		break;
	}

	return 0;
}

/**
 * @brief TIM2 Initialization Function
 * Clock: 160 MHz / ((Prescaler+1)×(Period+1))
 */
void MX_TIM2_Init( void )
{
    TIM_ClockConfigTypeDef sClockSourceConfig = { 0 };
    TIM_SlaveConfigTypeDef sSlaveConfig       = { 0 };
    TIM_MasterConfigTypeDef sMasterConfig     = { 0 };

    htim2.Instance = TIM2;
    __HAL_RCC_TIM2_CLK_ENABLE();

    htim2.Init.Prescaler         = TIMER_PRESCALER;
    htim2.Init.Period            = TIMER_PERIOD;
    htim2.Init.CounterMode       = TIM_COUNTERMODE_UP;
    htim2.Init.ClockDivision     = TIM_CLOCKDIVISION_DIV1;
    htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;

    if ( HAL_TIM_Base_Init( &htim2 ) != HAL_OK )
    {
        Error_Handler();
    }

    sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
    if ( HAL_TIM_ConfigClockSource( &htim2, &sClockSourceConfig ) != HAL_OK )
    {
        Error_Handler();
    }

    sSlaveConfig.SlaveMode    = TIM_SLAVEMODE_DISABLE;
    sSlaveConfig.InputTrigger = TIM_TS_ITR2;
    if ( HAL_TIM_SlaveConfigSynchro( &htim2, &sSlaveConfig ) != HAL_OK )
    {
        Error_Handler();
    }

    sMasterConfig.MasterOutputTrigger  = TIM_TRGO_RESET;
    sMasterConfig.MasterOutputTrigger2 = TIM_TRGO2_RESET;
    sMasterConfig.MasterSlaveMode      = TIM_MASTERSLAVEMODE_DISABLE;
    if ( HAL_TIMEx_MasterConfigSynchronization( &htim2, &sMasterConfig ) != HAL_OK )
    {
        Error_Handler();
    }

    HAL_NVIC_SetPriority( TIM2_IRQn, 0, 0 );
    HAL_NVIC_EnableIRQ( TIM2_IRQn );
}
