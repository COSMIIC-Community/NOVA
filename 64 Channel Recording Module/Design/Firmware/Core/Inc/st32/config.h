/**
 * @file config.h
 *
 *  Created on: Oct 9, 2024
 *      Author: jenej
 */

#ifndef INC_ST32_CONFIG_H_
#define INC_ST32_CONFIG_H_

// ============================================================================
// CAN Configuration
// ============================================================================
// ENABLE_CAN_TRANSMISSION: Controls CAN message transmission at compile time
//
// Set to 1: Normal operation with CAN transmission enabled
// Set to 0: UART-only mode - CAN transmission disabled (zero runtime overhead)
//
// Benefits of disabling (set to 0):
//   - Eliminates ALL CAN transmission overhead (compile-time optimization)
//   - Reduces power consumption (no CAN peripheral activity)
//   - Maximizes UART throughput (CPU freed from CAN processing)
//   - CAN reception still works (can receive configuration messages)
//
// Usage: Simply change the value below and rebuild
#define ENABLE_CAN_TRANSMISSION    1

#define CAN_BAUDRATE    100

// Needed defines by Canfestival lib
#define MAX_CAN_BUS_ID 1
#define SDO_MAX_LENGTH_TRANSFERT 50
#define SDO_MAX_SIMULTANEOUS_TRANSFERTS 1
#define NMT_MAX_NODE_ID 127
#define SDO_TIMEOUT_MS 1000U
#define MAX_NB_TIMER 8

// CANOPEN_BIG_ENDIAN is not defined
#define CANOPEN_LITTLE_ENDIAN 1

#define US_TO_TIMEVAL_FACTOR 8

#define REPEAT_SDO_MAX_SIMULTANEOUS_TRANSFERTS_TIMES(repeat)\
repeat

#define REPEAT_NMT_MAX_NODE_ID_TIMES(repeat)\
repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat  /*3 rows of 32 + 1 row of 31*/\
repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat \
repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat \
repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat repeat \

#define EMCY_MAX_ERRORS 8

#define REPEAT_EMCY_MAX_ERRORS_TIMES(repeat)\
repeat repeat repeat repeat repeat repeat repeat repeat

#endif /* INC_ST32_CONFIG_H_ */
