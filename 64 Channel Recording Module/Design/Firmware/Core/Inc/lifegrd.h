/**
 * @file lifegrd.h
 *
 *  Created on: Oct 9, 2024
 *      Author: jenej
 */

#ifndef INC_LIFEGRD_H_
#define INC_LIFEGRD_H_


typedef void (*heartbeatError_t)(CO_Data*, UNS8);
void _heartbeatError(CO_Data* d, UNS8 heartbeatID);

typedef void (*post_SlaveBootup_t)(CO_Data*, UNS8);
void _post_SlaveBootup(CO_Data* d, UNS8 SlaveID);

/*************************************************************************
 * Functions
 *************************************************************************/


e_nodeState getNodeState (CO_Data* d, UNS8 nodeId);
void heartbeatInit(CO_Data* d);
void heartbeatStop(CO_Data* d);
void processNODE_GUARD (CO_Data* d, Message* m);

#endif /* INC_LIFEGRD_H_ */
