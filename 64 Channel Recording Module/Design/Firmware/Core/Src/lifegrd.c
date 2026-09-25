/**
 * @file lifegrd.c
 *
 *  @date Created on: Oct 10, 2024
 *  @author Jerry Ukwela (jeu6@case.edu)
 *  @addtogroup CANFestival
 * 	@{
 * 	@defgroup heartbeat Heartbeat
 * 	@}
 */


#include <data.h>
#include "lifegrd.h"
#include "canfestival.h"
#include "sysdep.h"
#include "ObjDict.h"

/*Internals prototypes*/
void ConsumerHearbeatAlarm(CO_Data* d, UNS32 id);
void ProducerHearbeatAlarm(CO_Data* d, UNS32 id);
UNS32 OnHearbeatProducerUpdate(CO_Data* d, const indextable * unsused_indextable, UNS8 unsused_bSubindex);

/**
 * @ingroup statemachine
 * @brief To read the state of a node
 * This can be used by the master after having sent a life guard request,
 * of by any node if it is waiting for heartbeat.
 * @param *d Pointer on a CAN object data structure
 * @param nodeId Id of a node
 * @return e_nodeState State of the node corresponding to the nodeId
 */
e_nodeState getNodeState (CO_Data* d, UNS8 nodeId)
{
  e_nodeState networkNodeState = d->NMTable[nodeId];
  return networkNodeState;
}

/*
 * @ingroup heartbeat
 * @brief The Consumer Timer Callback
 * @param *d Pointer on a CAN object data structure
 * @param id
 */
void ConsumerHearbeatAlarm(CO_Data* d, UNS32 id)
{
  /*MSG_WAR(0x00, "ConsumerHearbeatAlarm", 0x00);*/

  /* timer has been notified and is now free (non periodic)*/
  /* -> avoid deleting re-assigned timer if message is received too late*/
  d->ConsumerHeartBeatTimers[id]=TIMER_NONE;
  /* set node state */
  //d->NMTable[nodeId] = Disconnected;
  /*! call heartbeat error with NodeId */
  (*d->heartbeatError)(d, (UNS8)( ((d->ConsumerHeartbeatEntries[id]) & (UNS32)0x00FF0000) >> (UNS8)16 ));
}

/**
 * @ingroup heartbeat
 * @brief This function is responsible to process a canopen-message which seams to be an NMT Error Control
 * Messages. At them moment we assume that every NMT error control message
 * is a heartbeat message.
 * If a BootUp message is detected, it will return the nodeId of the Slave who booted up
 * @param *d Pointer on a CAN object data structure
 * @param *m Pointer on the CAN-message which has to be analysed.
 * @note This feature is not used by the NNP
 */
void processNODE_GUARD(CO_Data* d, Message* m )
{
  UNS8 nodeId = (UNS8) GET_NODE_ID((*m));

  if((m->rtr == 1) )
    /*!
    ** Notice that only the master can have sent this
    ** node guarding request
    */
    {
      /*!
      ** Receiving a NMT NodeGuarding (request of the state by the
      ** master)
      ** Only answer to the NMT NodeGuarding request, the master is
      ** not checked (not implemented)
      */
      if (nodeId == *d->bDeviceNodeId )
        {
          Message msg;
          UNS16 tmp = *d->bDeviceNodeId + 0x700;
          msg.cob_id = UNS16_LE(tmp);
          msg.len = (UNS8)0x01;
          msg.rtr = 0;
          msg.data[0] = d->nodeState;
          if (d->toggle)
            {
              msg.data[0] |= 0x80 ;
              d->toggle = 0 ;
            }
          else
            d->toggle = 1 ;
          /* send the nodeguard response. */
          MSG_WAR(0x3130, "Sending NMT Nodeguard to master, state: ", d->nodeState);
          canSend(d->canHandle, &msg);
        }

    }
  else
    { /* incoming slave heartbeat */
       MSG_WAR(0x3110, "Received NMT nodeId : ", nodeId);
      /* the slave's state receievd is stored in the NMTable */
      /* The state is stored on 7 bit */
      d->NMTable[nodeId] = (e_nodeState) ((*m).data[0] & 0x7F) ;

      /* Boot-Up frame reception - removed by JDCC*/

      if( d->NMTable[nodeId] != Unknown_state )
      {
        UNS8 index, ConsummerHeartBeat_nodeId ;
        for( index = (UNS8)0x00; index < *d->ConsumerHeartbeatCount; index++ )
        {
          ConsummerHeartBeat_nodeId = (UNS8)( ((d->ConsumerHeartbeatEntries[index]) & (UNS32)0x00FF0000) >> (UNS8)16 );
          if ( nodeId == ConsummerHeartBeat_nodeId )
          {
            TIMEVAL time = ( (d->ConsumerHeartbeatEntries[index]) & (UNS32)0x0000FFFF ) ;
            /* Renew alarm for next heartbeat. */

            DelAlarm(d->ConsumerHeartBeatTimers[index]);
            d->ConsumerHeartBeatTimers[index] = SetAlarm(d, index, &ConsumerHearbeatAlarm, MS_TO_TIMEVAL(time), 0);
          }
        }

      }

    }
}

/**
 * @ingroup heartbeat
 * @brief the Consumer Timer Callback.  Heartbeat has been modified to include module Serial Number
 * @param *d Pointer on a CAN object data structure
 * @param id
 */
void ProducerHearbeatAlarm(CO_Data* d, UNS32 id)
{
  if(*d->ProducerHeartBeatTime)
    {
      Message msg;
      /* Time expired, the heartbeat must be sent immediately
      ** generate the correct node-id: this is done by the offset 1792
      ** (decimal) and additionaly
      ** the node-id of this device.
      */
      UNS16 tmp = *d->bDeviceNodeId + 0x700;
      msg.cob_id = UNS16_LE(tmp);
      msg.len = (UNS8)0x01;
      msg.rtr = 0;
      msg.data[0] = d->nodeState; /* No toggle for heartbeat !*/
      /* send the heartbeat */
      MSG_WAR(0x3130, "Producing heartbeat: ", d->nodeState);
      canSend(d->canHandle, &msg);

    }
  else
    {
      d->ProducerHeartBeatTimer = DelAlarm(d->ProducerHeartBeatTimer);
    }
}

/**
 * @ingroup heartbeat
 * @brief This is called when Index 0x1017 is updated.
 * @param *d Pointer on a CAN object data structure
 * @param unsused_indextable
 * @param unsused_bSubindex
 * @return
 */
UNS32 OnHeartbeatProducerUpdate(CO_Data* d, const indextable * unsused_indextable, UNS8 unsused_bSubindex)
{
  heartbeatStop(d);
  heartbeatInit(d);
  return 0;
}

/**
 * @ingroup heartbeat
 * @brief Start heartbeat consumer and producer
 * with respect to 0x1016 and 0x1017
 * object dictionary entries
 * @param *d Pointer on a CAN object data structure
 */
void heartbeatInit(CO_Data* d)
{
  UNS8 index; /* Index to scan the table of heartbeat consumers */
  RegisterSetODentryCallBack(d, 0x1017, 0x00, &OnHeartbeatProducerUpdate);
  d->toggle = 0;

  for( index = (UNS8)0x00; index < *d->ConsumerHeartbeatCount; index++ )
  {
    TIMEVAL time = (UNS16) ( (d->ConsumerHeartbeatEntries[index]) & (UNS32)0x0000FFFF ) ;
    /* MSG_WAR(0x3121, "should_time : ", should_time ) ; */
    if ( time )
      {
        d->ConsumerHeartBeatTimers[index] = SetAlarm(d, index, &ConsumerHearbeatAlarm, MS_TO_TIMEVAL(time), 0);
      }
  }

  if ( *d->ProducerHeartBeatTime )
    {
      TIMEVAL time = *d->ProducerHeartBeatTime;
      ProducerHearbeatAlarm(d, 0); //JML send out first heartbeat

      d->ProducerHeartBeatTimer = SetAlarm(d, 0, &ProducerHearbeatAlarm, MS_TO_TIMEVAL(time), MS_TO_TIMEVAL(time));
    }
}

/**
 * @ingroup heartbeat
 * @brief Stop heartbeat consumer and producer
 * @param *d Pointer on a CAN object data structure
 */
void heartbeatStop(CO_Data* d)
{
  UNS8 index;
  for( index = (UNS8)0x00; index < *d->ConsumerHeartbeatCount; index++ )
    {
      d->ConsumerHeartBeatTimers[index] = DelAlarm(d->ConsumerHeartBeatTimers[index]);
    }

  d->ProducerHeartBeatTimer = DelAlarm(d->ProducerHeartBeatTimer);
}

/**
 * @ingroup heartbeat
 * @brief heartbeat Error
 * @param *d Pointer on a CAN object data structure
 * @param heartbeatID
 */
void _heartbeatError(CO_Data* d, UNS8 heartbeatID){}

/**
 * @ingroup heartbeat
 * @brief post slave bootup
 * @param *d Pointer on a CAN object data structure
 * @param SlaveID
 */
void _post_SlaveBootup(CO_Data* d, UNS8 SlaveID){}
