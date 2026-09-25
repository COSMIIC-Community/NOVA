/*
 * can.h
 *
 *  Created on: Oct 9, 2024
 *      Author: jenej
 */

#ifndef INC_ST32_CAN_STM_H_
#define INC_ST32_CAN_STM_H_

// Canfestivals includes
#include "can.h"

#include "config.h"
#include "ObjDict.h"

extern UNS8 txErr;
extern UNS8 rxErr;

/************************* To be called by user app ***************************/

unsigned char canInit();
unsigned char canSend(FDCAN_HandleTypeDef *hcan, Message *m);
void HAL_FDCAN_RxFifo0Callback(FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo0ITs);
void HAL_FDCAN_RxFifo1Callback(FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo0ITs);

void HAL_FDCAN_ErrorCallback(FDCAN_HandleTypeDef *hfdcan);
unsigned char canChangeBaudRate_driver(FDCAN_HandleTypeDef *hcan, char* baud);
void UpdateCANerrors(void);

#endif /* INC_ST32_CAN_STM_H_ */
