/**
 * @file emcy.h
 *
 *  Created on: Oct 9, 2024
 *      Author: jenej
 */

#ifndef INC_EMCY_H_
#define INC_EMCY_H_

#include <applicfg.h>

/* The error states
 * ----------------- */
typedef enum enum_errorState {
  Error_free		= 0x00,
  Error_occurred	= 0x01
} e_errorState;

typedef struct {
	UNS16 errCode;
	UNS8 errRegMask;
	UNS8 active;
} s_errors;

#include "data.h"


typedef void (*post_emcy_t)(CO_Data* d, UNS8 nodeID, UNS16 errCode, UNS8 errReg);
void _post_emcy(CO_Data* d, UNS8 nodeID, UNS16 errCode, UNS8 errReg);

/*************************************************************************
 * Functions
 *************************************************************************/

UNS8 EMCY_setError(CO_Data* d, UNS16 errCode, UNS8 errRegMask, UNS16 addInfo);
void EMCY_errorRecovered(CO_Data* d, UNS16 errCode);
void emergencyInit(CO_Data* d);
void emergencyStop(CO_Data* d);
void proceedEMCY(CO_Data* d, Message* m);

#endif /* INC_EMCY_H_ */
