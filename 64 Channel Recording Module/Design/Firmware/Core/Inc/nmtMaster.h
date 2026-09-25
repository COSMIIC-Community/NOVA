/**
 * @file nmtMaster.h
 *
 *  Created on: Oct 9, 2024
 *      Author: jenej
 */

#ifndef INC_NMTMASTER_H_
#define INC_NMTMASTER_H_

UNS8 masterSendNMTstateChange (CO_Data* d, UNS8 nodeId, UNS8 cs);
UNS8 masterSendNMTnodeguard (CO_Data* d, UNS8 nodeId);
void masterRequestNodeState (CO_Data* d, UNS8 nodeId);

#endif /* INC_NMTMASTER_H_ */
