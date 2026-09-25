/**
 * @file states.c
 *
 * @date Created on: Oct 10, 2024
 * @author Jerry Ukwela (jeu6@case.edu)
 * @addtogroup CANFestival
 * @{
 * @defgroup statemachine State Machine
 * @}
 */


#include "data.h"
#include "sysdep.h"

/** Prototypes for internals functions */
void switchCommunicationState(CO_Data* d,
	s_state_communication *newCommunicationState);

/**********************************************************************************************************
*                                       LOCAL GLOBAL VARIABLES
**********************************************************************************************************/



/**
 * @ingroup statemachine
 * @brief Returns the state of the node
 * @param *d Pointer on a CAN object data structure
 * @return The node state
 */
e_nodeState getState(CO_Data* d)
{
  return d->nodeState;
}


/**
 * @brief Called by driver/app when receiving messages
 * @param *d Pointer on a CAN object data structure
 * @param *m Pointer on a CAN message structure
 */
void canDispatch(CO_Data* d, Message *m)
{
    UNS16 cob_id = UNS16_LE(m->cob_id);

	switch(cob_id >> 7)
	{
		case SYNC:		/* can be a SYNC or a EMCY message */
			if(cob_id == 0x080)	/* SYNC */
			{
				if(d->CurrentCommunicationState.csSYNC)
					processSYNC(d,m);
			} else 		/* EMCY */
				if(d->CurrentCommunicationState.csEmergency)
					proceedEMCY(d,m);
			break;
		case BOOT:
			if(cob_id == 0x140)
			{
				//Bootloader Query Node command
				processBOOT(d,m);
			}
			break;
		case PDO1tx:
		case PDO1rx:
		case PDO2tx:
		case PDO2rx:
		case PDO3tx:
		case PDO3rx:
		case PDO4tx:
		case PDO4rx:
			if (d->CurrentCommunicationState.csPDO)
				processPDO(d,m);
			break;
		case SDOtx:
		case SDOrx:
			if (d->CurrentCommunicationState.csSDO)
				processSDO(d,m);
			break;
		case NODE_GUARD:
			if (d->CurrentCommunicationState.csHeartbeat || !(*(d->iam_a_slave)))
				processNODE_GUARD(d,m);
			break;
		case NMT:
			// cause a reset to enter bootloader
			if ( ((m->data[1]== 0x00) || (m->data[1]== getNodeId(d))) &&
				 (m->data[0]== NMT_Enter_Bootloader) )
			{
				NVIC_SystemReset();
			}

			if (*(d->iam_a_slave))
			{
				processNMTstateChange(d,m);
			}
#ifdef CO_ENABLE_LSS
		case LSS:
			if (!d->CurrentCommunicationState.csLSS)break;
			if ((*(d->iam_a_slave)) && cob_id==MLSS_ADRESS)
			{
				proceedLSS_Slave(d,m);
			}
			else if(!(*(d->iam_a_slave)) && cob_id==SLSS_ADRESS)
			{
				proceedLSS_Master(d,m);
			}
			break;
#endif
	} // end CASE statement


}

#define StartOrStop(CommType, FuncStart, FuncStop) \
	if(newCommunicationState->CommType && d->CurrentCommunicationState.CommType == 0){\
		MSG_WAR(0x9999,#FuncStart, 9999);\
		d->CurrentCommunicationState.CommType = 1;\
		FuncStart;\
	}else if(!newCommunicationState->CommType && d->CurrentCommunicationState.CommType == 1){\
		MSG_WAR(0x9999,#FuncStop, 9999);\
		d->CurrentCommunicationState.CommType = 0;\
		FuncStop;\
	}
#define None

/**
 * @ingroup statemachine
 * @brief <BRIEF> switchCommunicationState
 * @param d
 * @param newCommunicationState
*/
void switchCommunicationState(CO_Data* d, s_state_communication *newCommunicationState)
{
#ifdef CO_ENABLE_LSS
	StartOrStop(csLSS,	startLSS(d),	stopLSS(d))
#endif
	StartOrStop(csSDO,			None,				resetSDO(d))
	StartOrStop(csSYNC,			startSYNC(d),		stopSYNC(d))
	StartOrStop(csHeartbeat,	heartbeatInit(d),	heartbeatStop(d))
	StartOrStop(csEmergency,	emergencyInit(d),	emergencyStop(d))
	StartOrStop(csPDO,			PDOInit(d),			PDOStop(d))
	StartOrStop(csBoot_Up,		None,				slaveSendBootUp(d))
}

/**
 * @ingroup statemachine
 * @brief Change the state of the node
 * @note All states other than Waiting and Stopped use the same Communication state structure
    - INTEGER8 csBoot_Up     0
	- INTEGER8 csSDO         1
	- INTEGER8 csEmergency   1
	- INTEGER8 csSYNC        1
	- INTEGER8 csHeartbeat   1
	- INTEGER8 csPDO         1
	- INTEGER8 csLSS         0
 * @param *d Pointer on a CAN object data structure
 * @param newState The state to assign
 * @return
 */
UNS8 setState(CO_Data* d, e_nodeState newState)
{
	if(newState != d->nodeState){
		switch( newState ){
			case Mode_Patient_Control:
			{
				s_state_communication newCommunicationState = {0, 1, 1, 1, 1, 1, 0};
				d->nodeState = Mode_Patient_Control;
				switchCommunicationState(d, &newCommunicationState);
			}
			break;

			case Mode_X_Manual:
			{
				s_state_communication newCommunicationState = {0, 1, 1, 1, 1, 1, 0};
				d->nodeState = Mode_X_Manual;
				switchCommunicationState(d, &newCommunicationState);
			}
			break;

			case Mode_Y_Manual:
			{
				s_state_communication newCommunicationState = {0, 1, 1, 1, 1, 1, 0};
				d->nodeState = Mode_Y_Manual;
				switchCommunicationState(d, &newCommunicationState);
			}
			break;

			case Waiting:
			{
				s_state_communication newCommunicationState = {0, 1, 1, 0, 1, 0, 0};
				d->nodeState = Waiting;
				switchCommunicationState(d, &newCommunicationState);
				(*d->waiting)(d);
			}
			break;

			case Stopped:
			{
				s_state_communication newCommunicationState = {0, 1, 1, 0, 1, 0, 0};
				d->nodeState = Stopped;
				switchCommunicationState(d, &newCommunicationState);
				(*d->stopped)(d);
			}
			break;

            case Mode_Patient_Manual:
			{
				s_state_communication newCommunicationState = {0, 1, 1, 1, 1, 1, 0};
				d->nodeState = Mode_Patient_Manual;
				switchCommunicationState(d, &newCommunicationState);
			}
			break;

            case Mode_Produce_X_Manual:
			{
				s_state_communication newCommunicationState = {0, 1, 1, 1, 1, 1, 0};
				d->nodeState = Mode_Produce_X_Manual;
				switchCommunicationState(d, &newCommunicationState);
				/* call user app init callback now. */
			}
			break;

			case Mode_Record_X:
			{
				s_state_communication newCommunicationState = {0, 1, 1, 1, 1, 1, 0};
				d->nodeState = Mode_Record_X;
				switchCommunicationState(d, &newCommunicationState);
				/* call user app init callback now. */
			}
			break;

			default:
				return 0xFF;

		}/* end switch case */

	}
	/* d->nodeState contains the final state */
	/* may not be the requested state */
    return d->nodeState;
}

/**
 * @ingroup statemachine
 * @brief Returns the nodId
 * @param *d Pointer on a CAN object data structure
 * @return
 */
UNS8 getNodeId(CO_Data* d)
{
  return *d->bDeviceNodeId;
}

/**
 * @ingroup statemachine
 * @brief Define the node ID. Initialize the object dictionary
 * @param *d Pointer on a CAN object data structure
 * @param nodeId The node ID to assign
 */
void setNodeId(CO_Data* d, UNS8 nodeId)
{
  UNS16 offset = d->firstIndex->SDO_SVR;

#ifdef CO_ENABLE_LSS
  d->lss_transfer.nodeID = nodeId;
  *d->bDeviceNodeId = nodeId;

  if(nodeId==0xFF)
    {
          *d->bDeviceNodeId = nodeId;
          return;
    }
    else
  #endif
    if(!(nodeId > 0 && nodeId <= 127))
    {
            MSG_WAR(0x2D01, "Invalid NodeID",nodeId);
            return;
    }

  if(offset)
  {
    /* Adjust COB-ID Client->Server (rx) only id already set to default value or id not valid (id==0xFF)*/
    //if((*(UNS32*)d->objdict[offset].pSubindex[1].pObject == 0x600 + *d->bDeviceNodeId)||(*d->bDeviceNodeId==0xFF))
    {
      /* cob_id_client = 0x600 + nodeId; */
      *(UNS32*)d->objdict[offset].pSubindex[1].pObject = 0x600 + nodeId;
    }
    /* Adjust COB-ID Server -> Client (tx) only id already set to default value or id not valid (id==0xFF)*/
    //if((*(UNS32*)d->objdict[offset].pSubindex[2].pObject == 0x580 + *d->bDeviceNodeId)||(*d->bDeviceNodeId==0xFF))
    {
      /* cob_id_server = 0x580 + nodeId; */
      *(UNS32*)d->objdict[offset].pSubindex[2].pObject = 0x580 + nodeId;
    }
  }

  /*
   	Initialize the server(s) SDO parameters
  	Remember that only one SDO server is allowed, defined at index 0x1200


  */

  /* ** Initialize the transmit PDO communication parameters. Only for 0x1800 to 0x1803 */
  {
    UNS8 i = 0;
    UNS16 offset = d->firstIndex->PDO_TRS;
    UNS16 lastIndex = d->lastIndex->PDO_TRS;
    UNS32 cobID[] = {0x180, 0x280, 0x380, 0x480};
    i = 0;
    if( offset ) while ((offset <= lastIndex) && (i < 4))
    {
      //if((*(UNS32*)d->objdict[offset].pSubindex[1].pObject == cobID[i] + *d->bDeviceNodeId)||(*d->bDeviceNodeId==0xFF))
	      *(UNS32*)d->objdict[offset].pSubindex[1].pObject = cobID[i] + nodeId;
      i ++;
      offset ++;
    }
  }

  /* Update EMCY COB-ID if already set to default*/
  if((*d->error_cobid == *d->bDeviceNodeId + 0x80)||(*d->bDeviceNodeId==0xFF))
    *d->error_cobid = nodeId + 0x80;

  /* bDeviceNodeId is defined in the object dictionary. */
  *d->bDeviceNodeId = nodeId;
}

/**
 * @ingroup statemachine
 * @brief Function that user app can overload
 * @param *d Pointer on a CAN object data structure
 */
void _mode_X_Manual(CO_Data* d){}

/**
 * @ingroup statemachine
 * @brief Function that user app can overload
 * @param *d Pointer on a CAN object data structure
 */
void _mode_Y_Manual(CO_Data* d){}

/**
 * @ingroup statemachine
 * @brief Function that user app can overload
 * @param *d Pointer on a CAN object data structure
 */
void _waiting(CO_Data* d){}

/**
 * @ingroup statemachine
 * @brief Function that user app can overload
 * @param *d Pointer on a CAN object data structure
 */
void _stopped(CO_Data* d){}

/**
 * @ingroup statemachine
 * @brief Function that user app can overload
 * @param *d Pointer on a CAN object data structure
 */
void _mode_Patient_Manual(CO_Data* d){}

/**
 * @ingroup statemachine
 * @brief Function that user app can overload
 * @param *d Pointer on a CAN object data structure
 */
void _mode_Patient_Control(CO_Data* d){}


