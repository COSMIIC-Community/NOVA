/**
 * @file pdo.h
 *
 *  Created on: Oct 9, 2024
 *      Author: jenej
 */

#ifndef INC_PDO_H_
#define INC_PDO_H_

#include "applicfg.h"
#include "def.h"


#include "can.h"

typedef struct struct_s_PDO_status s_PDO_status;


/* Status of the TPDO : */
#define PDO_INHIBITED 0x01
#define PDO_RTR_SYNC_READY 0x01

/** The PDO structure */
struct struct_s_PDO_status {
  UNS8 transmit_type_parameter;
  TIMER_HANDLE event_timer;
  TIMER_HANDLE inhibit_timer;
  Message last_message;
};

#define s_PDO_status_Initializer {0, TIMER_NONE, TIMER_NONE, Message_Initializer}

/** definitions of the different types of PDOs' transmission
 *
 * SYNCHRO(n) means that the PDO will be transmited every n SYNC signal.
 */
#define TRANS_EVERY_N_SYNC(n) (n) /*n = 1 to 240 */
#define TRANS_SYNC_ACYCLIC    0    /* Trans after reception of n SYNC. n = 1 to 240 */
#define TRANS_SYNC_MIN        1    /* Trans after reception of n SYNC. n = 1 to 240 */
#define TRANS_SYNC_MAX        240  /* Trans after reception of n SYNC. n = 1 to 240 */
#define TRANS_RTR_SYNC        252  /* Transmission on request */
#define TRANS_RTR             253  /* Transmission on request */
#define TRANS_EVENT_SPECIFIC  254  /* Transmission on event */
#define TRANS_EVENT_PROFILE   255  /* Transmission on event */


UNS8 buildPDO(CO_Data* d, UNS8 numPdo, Message *pdo);
UNS8 sendPDOrequest( CO_Data* d, UNS16 RPDOIndex );
UNS8 processPDO (CO_Data* d, Message *m);
UNS8 sendPDOevent (CO_Data* d);
UNS8 _sendPDOevent(CO_Data* d, UNS8 isSyncEvent);
void PDOInit(CO_Data* d);
void PDOStop(CO_Data* d);
void PDOEventTimerAlarm(CO_Data* d, UNS32 pdoNum);
void PDOInhibitTimerAlarm(CO_Data* d, UNS32 pdoNum);
void CopyBits(UNS8 NbBits, UNS8* SrcByteIndex, UNS8 SrcBitIndex, UNS8 SrcBigEndian, UNS8* DestByteIndex, UNS8 DestBitIndex, UNS8 DestBigEndian);
void sendPdo(CO_Data * d, UNS32 pdoNum, Message * pdo);

#endif /* INC_PDO_H_ */
