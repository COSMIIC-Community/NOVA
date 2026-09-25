/**
 * @file timer_driver.h
 *
 *  Created on: Oct 9, 2024
 *      Author: jenej
 */

#ifndef INC_TIMER_DRIVER_H_
#define INC_TIMER_DRIVER_H_

#include "timerscfg.h"
#include "timer.h"

// For use from CAN driver


/**
 * @ingroup timer
 * @brief Acquire mutex
 */
void EnterMutex(void);

/**
 * @ingroup timer
 * @brief Release mutex
 */
void LeaveMutex(void);

void WaitReceiveTaskEnd(TASK_HANDLE*);

/**
 * @ingroup timer
 * @brief Initialize Timer
 */
void TimerInit(void);

/**
 * @ingroup timer
 * @brief Cleanup Timer
 */
void TimerCleanup(void);

/**
 * @ingroup timer
 * @brief Start the timer task
 * @param Callback A callback function
 */
void StartTimerLoop(TimerCallback_t Callback);

/**
 * @ingroup timer
 * @brief Stop the timer task
 * @param Callback A callback function
 */
void StopTimerLoop(TimerCallback_t Callback);

/**
 * @brief Stop the timer task
 * @param port CanFestival file descriptor
 * @param *handle handle of receive loop thread
 * @param *ReceiveLoopPtr Pointer on the receive loop function
 */
void CreateReceiveTask(CAN_PORT port, TASK_HANDLE* handle, void* ReceiveLoopPtr);

#endif /* INC_TIMER_DRIVER_H_ */
