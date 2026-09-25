/*
 * canfestival.h
 *
 *  Created on: Oct 9, 2024
 *      Author: jenej
 */

#ifndef INC_ST32_CANFESTIVAL_H_
#define INC_ST32_CANFESTIVAL_H_

#include "applicfg.h"
#include "data.h"

void initTimer(void);
UNS8 canSend(FDCAN_HandleTypeDef *hcan, Message *m);
UNS8 canChangeBaudRate(CAN_PORT port, char* baud);

#endif /* INC_ST32_CANFESTIVAL_H_ */
