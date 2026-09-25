/**
 * @file nmtMaster.c
 *
 *  @date Created on: Oct 10, 2024
 *  @author Jerry Ukwela (jeu6@case.edu)
 *  @addtogroup CANFestival
 *  @{
 *  @defgroup nmtmaster NMT Master
 *  @}
 */

#include "canfestival.h"
#include "nmtMaster.h"
#include "sysdep.h"

/**
 * @ingroup nmtmaster
 * @brief Transmit a NMT message on the network to the slave whose nodeId is node ID.
 *
 * @param *d Pointer to a CAN object data structure
 * @param nodeId Id of the slave node
 * @param cs The order of state changement \n\n
 *
 *
 * \n\n
 * @return errorcode
 *                   - 0 if the NMT message was send
 *                   - 1 if an error occurs
 */
UNS8 masterSendNMTstateChange(CO_Data* d, UNS8 Node_ID, UNS8 cs)
{
  Message m;

  MSG_WAR(0x3501, "Send_NMT cs : ", cs);
  MSG_WAR(0x3502, "    to node : ", Node_ID);
  /* message configuration */
  m.cob_id = 0x0000; /*(NMT) << 7*/
  m.rtr = NOT_A_REQUEST;
  m.len = 2;
  m.data[0] = cs;
  m.data[1] = Node_ID;

  return canSend(d->canHandle, &m);
}



/**
 * @ingroup nmtmaster
 * @brief Transmit a NodeGuard message on the network to the slave whose nodeId is node ID
 *
 * @param *d Pointer to a CAN object data structure
 * @param nodeId Id of the slave node
 * @return
 *         - 0 is returned if the NodeGuard message was send.
 *         - 1 is returned if an error occurs.
 */
UNS8 masterSendNMTnodeguard(CO_Data* d, UNS8 nodeId)
{
  Message m;

  /* message configuration */
  UNS16 tmp = nodeId | (NODE_GUARD << 7);
  m.cob_id = UNS16_LE(tmp);
  m.rtr = REQUEST;
  m.len = 0;

  MSG_WAR(0x3503, "Send_NODE_GUARD to node : ", nodeId);

  return canSend(d->canHandle, &m);
}

/**
 * @ingroup nmtmaster
 * @brief Ask the state of the slave node whose nodeId is node Id.
 *
 * To ask states of all nodes on the network (NMT broadcast), nodeId must be equal to 0
 * @param *d Pointer to a CAN object data structure
 * @param nodeId Id of the slave node
 */
void masterRequestNodeState(CO_Data* d, UNS8 nodeId)
{
  /* FIXME: should warn for bad toggle bit. */

  /* NMTable configuration to indicate that the master is waiting
    for a Node_Guard frame from the slave whose node_id is ID
  */
  d->NMTable[nodeId] = Unknown_state; /* A state that does not exist
                                       */

  if (nodeId == 0) { /* NMT broadcast */
    UNS8 i = 0;
    for (i = 0 ; i < NMT_MAX_NODE_ID ; i++) {
      d->NMTable[i] = Unknown_state;
    }
  }
  masterSendNMTnodeguard(d,nodeId);
}

