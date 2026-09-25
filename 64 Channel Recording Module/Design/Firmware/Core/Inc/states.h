/**
 * @file states.h
 *
 *  Created on: Oct 9, 2024
 *      Author: jenej
 */

#ifndef INC_STATES_H_
#define INC_STATES_H_

#include <applicfg.h>

/* The nodes states
 * -----------------
 * values are choosen so that they can be sent directly
 * for heartbeat messages...
 * Must be coded on 7 bits only (bit 8 is toggle bit)
 * */

enum enum_nodeState {
  Hibernate                 = 0x00,
  Waiting                   = 0x01,
  Mode_X_Manual             = 0x02,
  Mode_Y_Manual             = 0x03,
  Stopped                   = 0x04,
  Mode_Patient_Control      = 0x05,
  BootCheckReset            = 0x06,
  Mode_Patient_Manual       = 0x07,
  Mode_Produce_X_Manual     = 0x08,
  Mode_Record_X             = 0x09,
  Unknown_state             = 0x0F

};

typedef enum enum_nodeState e_nodeState;
/* Communication state structure */
typedef struct
{
	INTEGER8 csBoot_Up;
	INTEGER8 csSDO;
	INTEGER8 csEmergency;
	INTEGER8 csSYNC;
	INTEGER8 csHeartbeat;
	INTEGER8 csPDO;
	INTEGER8 csLSS;
} s_state_communication;


typedef void (*mode_X_Manual_t)(CO_Data*);
typedef void (*mode_Y_Manual_t)(CO_Data*);
typedef void (*waiting_t)(CO_Data*);
typedef void (*stopped_t)(CO_Data*);
typedef void (*mode_Patient_Manual_t)(CO_Data*);
typedef void (*mode_Patient_Control_t)(CO_Data*);

void _mode_X_Manual(CO_Data* d);
void _mode_Y_Manual(CO_Data* d);
void _waiting(CO_Data* d);
void _stopped(CO_Data* d);
void _mode_Patient_Manual(CO_Data* d);
void _mode_Patient_Control(CO_Data* d);
void _mode_Produce_X_Manual(CO_Data* d);


/************************* prototypes ******************************/

void canDispatch(CO_Data* d, Message *m);
e_nodeState getState (CO_Data* d);
UNS8 setState (CO_Data* d, e_nodeState newState);
UNS8 getNodeId (CO_Data* d);
void setNodeId (CO_Data* d, UNS8 nodeId);


#endif /* INC_STATES_H_ */
