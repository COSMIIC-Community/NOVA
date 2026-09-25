/**
 * @file sync.h
 *
 *  Created on: Oct 9, 2024
 *      Author: jenej
 */

#ifndef INC_SYNC_H_
#define INC_SYNC_H_

void startSYNC(CO_Data* d);
void stopSYNC(CO_Data* d);
typedef void (*post_sync_t)(CO_Data*);
void _post_sync(CO_Data* d);
typedef void (*post_TPDO_t)(CO_Data*);
void _post_TPDO(CO_Data* d);
UNS8 sendSYNC (CO_Data* d);
UNS8 sendSYNCMessage(CO_Data* d);
UNS8 processSYNC (CO_Data* d, Message* m);

#endif /* INC_SYNC_H_ */
