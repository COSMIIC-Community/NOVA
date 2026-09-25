/**
 * @file can.h
 *
 *  Created on: Oct 9, 2024
 *      @author: Jerry Ukwela (jukwela@metrohealth.org)
 */

#ifndef INC_CAN_H_
#define INC_CAN_H_

#include "applicfg.h"

typedef struct {
  UNS16 cob_id;	/**< message's ID */
  UNS8 rtr;		/**< remote transmission request. (0 if not rtr message, 1 if rtr message) */
  UNS8 len;		/**< message's length (0 to 8) */
  UNS8 data[8]; /**< message's datas */
} Message;

#define Message_Initializer {0,0,0,{0,0,0,0,0,0,0,0}}

typedef UNS8 (*canSend_t)(Message *);

#endif /* INC_CAN_H_ */
