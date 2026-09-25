/**
 * @file timer.h
 *
 *  Created on: Oct 9, 2024
 *      Author: jenej
 */

#ifndef INC_TIMER_H_
#define INC_TIMER_H_


#include "timerscfg.h"
#include "applicfg.h"

#define TIMER_HANDLE INTEGER16
#define TASK_HANDLE  INTEGER16

#include <can_stm.h>

/* --------- types and constants definitions --------- */
#define TIMER_FREE 0        // timer is available for use
#define TIMER_ARMED 1       // row is active
#define TIMER_TRIG 2        // timer has been triggered
#define TIMER_TRIG_PERIOD 3

#define TIMER_NONE -1

typedef void (*TimerCallback_t)(CO_Data* d, UNS32 id);

/**
 * @brief timer struct
 */
struct struct_s_timer_entry {
	UNS8 state;
	CO_Data* d;
	TimerCallback_t callback; /*!< The callback func. */
	UNS32 id; /*!< The callback func. */
	TIMEVAL val;  /*!< this is the current timer value */
	TIMEVAL interval; /*!< Periodicity */
};

typedef struct struct_s_timer_entry s_timer_entry;

/****************Variables****************************/
extern unsigned char osc_value;

/* ---------  prototypes --------- */
TIMER_HANDLE SetAlarm(CO_Data* d, UNS32 id, TimerCallback_t callback, TIMEVAL value, TIMEVAL period);
TIMER_HANDLE DelAlarm(TIMER_HANDLE handle);
void TimeDispatch(void);

//for timer_stm.c
void setTimer(TIMEVAL value);
TIMEVAL getElapsedTime(void);

UNS8 isTimedOut( UNS32 *tRef, UNS32 tAlarm );
void resetTimeOut( UNS32 *tRef );

#endif /* INC_TIMER_H_ */
