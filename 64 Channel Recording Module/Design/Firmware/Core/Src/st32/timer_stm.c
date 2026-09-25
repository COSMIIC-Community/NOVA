/**
 * @file timer_stm.c
 *
 *  @date Created on: Oct 9, 2024
 *  @author Jerry Ukwela (jeu6@case.edu)
 *  @ingroup CANFestival
 * 	@ingroup FESCAN
 */


#include "timer.h"
#include "stm32u5xx.h"
#include "main.h"
#include "sampling.h"

/************************** Module variables **********************************/
// Store the last timer value to calculate the elapsed time
static volatile TIMEVAL last_time_set = 0;//TIMEVAL_MAX;
static volatile TIMEVAL elapsed_time = 0;

/**
  * @brief  Set the timer peripheral for the next interrupt
  */
void setTimer(TIMEVAL value){
	uint32_t timer = TIM6->CNT;
	elapsed_time += timer - last_time_set;

	last_time_set = 65535-value;
	TIM6->CNT = last_time_set;

	TIM6->CR1 |= 1; //Re-enable timer
}

void initTimer(void)
{

}

/**
  * @brief  Checks if a timer has timed out
  */
UNS8 isTimedOut( UNS32 *tRef, UNS32 tAlarm )
{

	if( (HAL_GetTick() - *tRef) > tAlarm )
	{
		return 1;
	}

	else
		return 0;
}

/**
  * @brief  Resets a timed out timer
  */
void resetTimeOut( UNS32 *tRef )
{
	*tRef = HAL_GetTick();
}

/**
 * @brief Return the elapsed time to tell the Stack how much time is spent since last call.
 * @return value TIMEVAL (unsigned long) the elapsed time
 */
TIMEVAL getElapsedTime(void)
{
	TIMEVAL timer = TIM6->CNT;

    if (timer > last_time_set)                    // In case the timer value is higher than the last time.
	  return (timer - last_time_set);             // Calculate the time difference
    else if (timer < last_time_set)
	  return (last_time_set - timer);             // Calculate the time difference
    else
	  return TIMEVAL_MAX;
}

/**
 * @brief Timer period elapsed callback
 * @details Called when TIM2 reaches period (at sample rate)
 *          Triggers new SPI DMA transfers
 *          Called when TIM6 reaches period
 *          Triggers CanFestival TimeDispatch
 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	if (htim->Instance == TIM6) {
		last_time_set = 0;
		elapsed_time = 0;
		CLEAR_BIT(TIM6->SR, TIM_SR_UIF); //Clear pending interrupt
		TimeDispatch();
	}
	if ( htim->Instance == TIM2 )
	{
		// BREAKPOINT HERE: Verify TIM2 interrupt is firing
		// Trigger new conversion cycle
		start_spi_transfers();
		// BREAKPOINT HERE: Verify start_spi_transfers() returned
	}
}
