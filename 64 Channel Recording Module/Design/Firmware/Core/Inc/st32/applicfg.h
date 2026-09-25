/**
 * @file applicfg.h
 *
 *  Created on: Oct 9, 2024
 *      @author: Jerry Ukwela (jukwela@metrohealth.org)
 */

#ifndef INC_ST32_APPLICFG_H_
#define INC_ST32_APPLICFG_H_

#include <string.h>
#include <stdio.h>

// Integers
#define INTEGER8  signed char 	///< Signed int8 representation in CANFest
#define INTEGER16 short			///< Signed int16 representation in CANFest
#define INTEGER24 long			///< Signed int24 representation in CANFest
#define INTEGER32 long			///< Signed int32 representation in CANFest
#define INTEGER40 double		///< Signed int40 representation in CANFest
#define INTEGER48 double		///< Signed int48 representation in CANFest
#define INTEGER56 double		///< Signed int56 representation in CANFest
#define INTEGER64 double		///< Signed int64 representation in CANFest

// Unsigned integers
#define UNS8   unsigned char	///< Unsigned int8 representation in CANFest
#define UNS16  unsigned short	///< Unsigned int16 representation in CANFest
#define UNS32  unsigned long	///< Unsigned int32 representation in CANFest
#define UNS24  unsigned long	///< Unsigned int24 representation in CANFest
#define UNS40  unsigned double	///< Unsigned int40 representation in CANFest
#define UNS48  unsigned double	///< Unsigned int48 representation in CANFest
#define UNS56  unsigned double	///< Unsigned int56 representation in CANFest
#define UNS64  unsigned double	///< Unsigned int64 representation in CANFest


// Reals
#define REAL32	float			///< Floating integer representation in CANFest
#define REAL64  double			///< Floating double integer representation in CANFest
//#include <can_stm.h>

// MSG functions
// not finished, the strings have to be placed to the flash and printed out
// using the printf_P function
/// Definition of MSG_ERR
// ---------------------
#ifdef DEBUG_ERR_CONSOLE_ON
#define MSG_ERR(num, str, val)      \
          printf(num, ' ');	\
          printf(str);		\
          printf(val);		\
          printf('\n');
#else
#    define MSG_ERR(num, str, val)
#endif

/// Definition of MSG_WAR
// ---------------------
#ifdef DEBUG_WAR_CONSOLE_ON
#define MSG_WAR(num, str, val)      \
          printf(num, ' ');	\
          printf(str);		\
          printf(val);		\
          printf('\n');
#else
#    define MSG_WAR(num, str, val)
#endif

typedef void* CAN_HANDLE;

typedef void* CAN_PORT;

#endif /* INC_ST32_APPLICFG_H_ */
