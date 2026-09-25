/**
 * @file sdo.h
 *
 *  Created on: Oct 9, 2024
 *      Author: jenej
 */

#ifndef INC_SDO_H_
#define INC_SDO_H_

#include "timer.h"

struct struct_s_transfer;

typedef void (*SDOCallback_t)(CO_Data* d, UNS8 nodeId);

/* The Transfer structure
Used to store the different segments of
 - a SDO received before writing in the dictionary
 - the reading of the dictionary to put on a SDO to transmit
*/

struct struct_s_transfer
{
  UNS8           nodeId;     /**<own ID if server, or node ID of the server if client */

  UNS8           whoami;     /**< Takes the values SDO_CLIENT or SDO_SERVER */
  UNS8           state;      /**< state of the transmission : Takes the values SDO_... */
  UNS8           toggle;
  UNS32          abortCode;  /**< Sent or received */
  /**< index and subindex of the dictionary where to store */
  /**< (for a received SDO) or to read (for a transmit SDO) */
  UNS16          index;
  UNS8           subIndex;
  UNS32          count;      /**< Number of data received or to be sent. */
  UNS32          offset;     /**< stack pointer of data[]
                              * Used only to tranfer part of a line to or from a SDO.
                              * offset is always pointing on the next free cell of data[].
                              * WARNING s_transfer.data is subject to ENDIANISATION
                              * (with respect to CANOPEN_BIG_ENDIAN)
                              */
  UNS8           data [SDO_MAX_LENGTH_TRANSFERT];
  UNS8           dataType;   /**< Defined in objdictdef.h Value is visible_string
                              * if it is a string, any other value if it is not a string,
                              * like 0. In fact, it is used only if client.
                              */
  TIMER_HANDLE   timer;      /**< Time counter to implement a timeout in milliseconds.
                              * It is automatically incremented whenever
                              * the line state is in SDO_DOWNLOAD_IN_PROGRESS or
                              * SDO_UPLOAD_IN_PROGRESS, and reseted to 0
                              * when the response SDO have been received.
                              */
  SDOCallback_t Callback;   /**< The user callback func to be called at SDO transaction end */

};
typedef struct struct_s_transfer s_transfer;


struct BODY{
    UNS8 data[8]; /**< The 8 bytes data of the SDO */
};

/* The SDO structure ...*/
struct struct_s_SDO {
  UNS8 nodeId;		/**< In any case, Node ID of the server (case sender or receiver).*/
  struct BODY body;
};


typedef struct struct_s_SDO s_SDO;


void SDOTimeoutAlarm(CO_Data* d, UNS32 id);
void resetSDO (CO_Data* d);
UNS32 SDOlineToObjdict (CO_Data* d, UNS8 line);
UNS32 objdictToSDOline (CO_Data* d, UNS8 line);
UNS32 objdictToSDOBlockline (CO_Data* d, UNS8 line, UNS8 numberOfIndexes);
UNS8 lineToSDO (CO_Data* d, UNS8 line, UNS32 nbBytes, UNS8 * data);
UNS8 SDOtoLine (CO_Data* d, UNS8 line, UNS32 nbBytes, UNS8 * data);
UNS8 failedSDO (CO_Data* d, UNS8 nodeId, UNS8 whoami, UNS16 index, UNS8 subIndex, UNS32 abortCode);
void resetSDOline (CO_Data* d, UNS8 line);
UNS8 initSDOline (CO_Data* d, UNS8 line, UNS8 nodeId, UNS16 index, UNS8 subIndex, UNS8 state);
UNS8 getSDOfreeLine (CO_Data* d, UNS8 whoami, UNS8 *line);
UNS8 getSDOlineOnUse (CO_Data* d, UNS8 nodeId, UNS8 whoami, UNS8 *line);
UNS8 closeSDOtransfer (CO_Data* d, UNS8 nodeId, UNS8 whoami);
UNS8 getSDOlineRestBytes (CO_Data* d, UNS8 line, UNS32 * nbBytes);
UNS8 setSDOlineRestBytes (CO_Data* d, UNS8 line, UNS32 nbBytes);
UNS8 sendSDO (CO_Data* d, UNS8 whoami, s_SDO sdo);
UNS8 sendSDOabort (CO_Data* d, UNS8 whoami, UNS8 nodeId, UNS16 index, UNS8 subIndex, UNS32 abortCode);
UNS8 processSDO (CO_Data* d, Message *m);
UNS8 writeNetworkDict (CO_Data* d, UNS8 nodeId, UNS16 index,
		       UNS8 subIndex, UNS32 count, UNS8 dataType, void *data);
UNS8 writeNetworkDictCallBack (CO_Data* d, UNS8 nodeId, UNS16 index,
		       UNS8 subIndex, UNS32 count, UNS8 dataType, void *data, SDOCallback_t Callback);
UNS8 writeNetworkDictCallBackAI (CO_Data* d, UNS8 nodeId, UNS16 index,
		       UNS8 subIndex, UNS32 count, UNS8 dataType, void *data, SDOCallback_t Callback, UNS8 endianize);
UNS8 readNetworkDict (CO_Data* d, UNS8 nodeId, UNS16 index, UNS8 subIndex, UNS8 dataType);
UNS8 readBlockNetworkDict(CO_Data* d, UNS8 nodeId, UNS16 index, UNS8 subIndex, UNS8 size, UNS8 dataType );
UNS8 readNetworkDictCallback (CO_Data* d, UNS8 nodeId, UNS16 index, UNS8 subIndex, UNS8 dataType, SDOCallback_t Callback);
UNS8 readNetworkDictCallbackAI (CO_Data* d, UNS8 nodeId, UNS16 index, UNS8 subIndex, UNS8 dataType, SDOCallback_t Callback);
UNS8 getReadResultNetworkDict (CO_Data* d, UNS8 nodeId, void* data, UNS32 *size, UNS32 * abortCode);
UNS8 getWriteResultNetworkDict (CO_Data* d, UNS8 nodeId, UNS32 * abortCode);


#endif /* INC_SDO_H_ */
