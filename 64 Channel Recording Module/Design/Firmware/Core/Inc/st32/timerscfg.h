/*
 * timerscfg.h
 *
 *  Created on: Oct 9, 2024
 *      Author: jenej
 */

#ifndef INC_ST32_TIMERSCFG_H_
#define INC_ST32_TIMERSCFG_H_

// Whatever your microcontroller, the timer wont work if
// TIMEVAL is not at least on 32 bits
#define TIMEVAL UNS32

// The timer of the STM32 counts from 0 to 0xFFFF (it can be
// shortened setting counter ARR register eg. to get 2ms instead of 2.048ms)
#define TIMEVAL_MAX 0xFFFF

#define MS_TO_TIMEVAL(ms) ((ms) * 100UL) //JML added UL otherwise may overflow
#define US_TO_TIMEVAL(us) ((us)>>3)

#endif /* INC_ST32_TIMERSCFG_H_ */
