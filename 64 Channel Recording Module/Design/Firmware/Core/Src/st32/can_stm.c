/**
 * @file can_stm.c
 *
 *  @date Created on: Oct 9, 2024
 *  @author Jerry Ukwela (jeu6@case.edu)
 *  @ingroup CANFestival
 * 	@ingroup FESCAN
 */

#include "main.h"
#include "can_stm.h"
#include "canfestival.h"
#include <stdlib.h>
#include "ObjDict.h"

#include "app.h"

volatile unsigned char msg_received = 0;

volatile UNS16 BitErrors = 0 ;
volatile UNS16 StuffErrors = 0;
volatile UNS16 FormErrors = 0;
volatile UNS16 OtherErrors = 0;
volatile UNS16 receivedMessages = 0;

UNS8 txErr = 0;
UNS8 rxErr = 0;

unsigned char canInit()
{
  unsigned char j;
  UNS32 PDOData[4];
  UNS8 type = 0;
  UNS32 size = 0;
  UNS32 pdata = 0;

  for ( j = 0; j < 4; j++)
  {
    getODentry( &ObjDict_Data,
                   0x1400 + j,
                   1,
                   &pdata,
                   &size,
                   &type,
                   0);
    PDOData[j] = pdata;
  }
  UNS16 identifiers[] = { 0x0080, (UNS16)getNodeId(&ObjDict_Data), (UNS16)PDOData[0], (UNS16)PDOData[1], (UNS16)PDOData[2], (UNS16)PDOData[3], \
    0x0000, 0x0140, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000 };

  // identifiers[] can be used to assign the CAN filter, is unused here
  TxHeader.IdType = FDCAN_STANDARD_ID;
  TxHeader.Identifier = identifiers[1];

  return 1;
}

void UpdateCANerrors(void)
{

      CAN_BitErrors = BitErrors;
      CAN_StuffErrors = StuffErrors;
      CAN_FormErrors = FormErrors;
      CAN_OtherErrors = OtherErrors;
      CAN_Rx_ErrCounter = (UNS16)rxErr;
      CAN_Tx_ErrCounter = (UNS16)txErr;
}

/**
 * @brief The driver send a CAN message passed from the CANopen stack
 * @param hfdcan, CAN Handle
 * @param *m pointer to message to send
 * @return 1 if  hardware -> CAN frame
 * @ingroup CANFestival FESCAN
 * @note When ENABLE_CAN_TRANSMISSION is 0, this becomes a no-op at compile time
 */
UNS8 canSend(FDCAN_HandleTypeDef *hfdcan, Message *m)
{
#if ENABLE_CAN_TRANSMISSION
  if(HAL_FDCAN_GetTxFifoFreeLevel(hfdcan) > 0)
  {
	  TxHeader.Identifier = m->cob_id;
	  TxHeader.IdType = FDCAN_STANDARD_ID;
	  if(m->rtr)
		  TxHeader.TxFrameType = FDCAN_REMOTE_FRAME;
	  else
		  TxHeader.TxFrameType = FDCAN_DATA_FRAME;

	  TxHeader.DataLength = m->len;			// Set data length code
	  memcpy(&TxData, m->data, m->len);

	  if (HAL_FDCAN_AddMessageToTxFifoQ(hfdcan, &TxHeader, TxData) != HAL_OK)
	  {
	  	  return 0;
	  }

	  CAN_Transmit_Messages++;
	  return 1;
  }

  return 0;
#else
  // CAN transmission disabled - return success without transmitting
  // This entire function body is optimized out by the compiler
  return 1;
#endif
}

void HAL_FDCAN_RxFifo0Callback(FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo0ITs)
{
	switch (RxFifo0ITs)
	{
		case FDCAN_IT_RX_FIFO0_MESSAGE_LOST:
			__HAL_FDCAN_CLEAR_IT(hfdcan, FDCAN_IT_RX_FIFO0_MESSAGE_LOST);
			CAN_TotalErrors++;
			rxErr++;
		case FDCAN_IT_RX_FIFO0_NEW_MESSAGE:
			break;
		default:
			return;
	}

	if(HAL_FDCAN_GetRxMessage(hfdcan, FDCAN_RX_FIFO0, &RxHeader, RxData) != HAL_OK){ Error_Handler(); }
	CAN_Receive_Messages++;

	Message rxm;

	if(RxHeader.IdType != FDCAN_STANDARD_ID)
		return;

	rxm.cob_id = RxHeader.Identifier;
	if(RxHeader.RxFrameType == FDCAN_REMOTE_FRAME)
		rxm.rtr = 1;
	rxm.len = RxHeader.DataLength;
	memcpy(&rxm.data, RxData, RxHeader.DataLength);

	canDispatch(&ObjDict_Data, &rxm);
	UpdateCANerrors();
}

void HAL_FDCAN_RxFifo1Callback(FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo0ITs)
{
	HAL_FDCAN_GetRxMessage(hfdcan, FDCAN_RX_FIFO1, &RxHeader, RxData);
	CAN_Receive_Messages++;
}

/**
 * @brief CAN message callback that handles errors with communication
 * @ingroup FESCAN
 */
void HAL_FDCAN_ErrorCallback(FDCAN_HandleTypeDef *hfdcan)
{
	if(hfdcan->ErrorCode & FDCAN_PROTOCOL_ERROR_NONE)
	{
		return;
	}
	if(hfdcan->ErrorCode & FDCAN_PROTOCOL_ERROR_STUFF)
	{
		StuffErrors++;
	}
	if(hfdcan->ErrorCode & FDCAN_PROTOCOL_ERROR_FORM)
	{
		FormErrors++;
	}
	if(hfdcan->ErrorCode & FDCAN_PROTOCOL_ERROR_ACK)
	{
		OtherErrors++;
	}
	if((hfdcan->ErrorCode & FDCAN_PROTOCOL_ERROR_BIT1) ||
	   (hfdcan->ErrorCode & FDCAN_PROTOCOL_ERROR_BIT0))
	{
		BitErrors++;
	}
	if((hfdcan->ErrorCode & FDCAN_PROTOCOL_ERROR_CRC ) ||
	   (hfdcan->ErrorCode & HAL_FDCAN_ERROR_TIMEOUT) ||
	   (hfdcan->ErrorCode & HAL_FDCAN_ERROR_NOT_INITIALIZED) ||
	   (hfdcan->ErrorCode & HAL_FDCAN_ERROR_NOT_READY)  ||
	   (hfdcan->ErrorCode & HAL_FDCAN_ERROR_NOT_STARTED)  ||
	   (hfdcan->ErrorCode & HAL_FDCAN_ERROR_PARAM))
	{
		OtherErrors++;
	}
//	if(hfdcan->ErrorCode & HAL_CAN_ERROR_BOF)
//	{
//		OtherErrors++;
//	}

//	if((hfdcan->ErrorCode & HAL_CAN_ERROR_TX_TERR0) || (hfdcan->ErrorCode & HAL_CAN_ERROR_TX_TERR1) || (hfdcan->ErrorCode & HAL_CAN_ERROR_TX_TERR2))
//	{
//		txErr++;
//	}
//	if((hfdcan->ErrorCode & HAL_CAN_ERROR_RX_FOV0) || (hfdcan->ErrorCode & HAL_CAN_ERROR_RX_FOV1))
//	{
//		rxErr++;
//	}

	CAN_TotalErrors++;

	hfdcan->ErrorCode = 0;
}

/**
 * @brief
 * @param fd
 * @param baud
 * @return always 0
 */
unsigned char canChangeBaudRate_driver(FDCAN_HandleTypeDef *hfdcan, char* baud)
{
	return 0;
}
