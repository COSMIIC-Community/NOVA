/**
 * @file lss.h
 *
 *  Created on: Oct 14, 2024
 *      Author: jenej
 */

#ifndef INC_LSS_H_
#define INC_LSS_H_

#define SLSS_ADRESS	0x7E4
#define MLSS_ADRESS	0x7E5

#define SDELAY_OFF		0
#define SDELAY_FIRST 	1
#define SDELAY_SECOND 	2

#define LSS_WAITING_MODE 0
#define LSS_CONFIGURATION_MODE 1

/* Switch mode services */
#define LSS_SM_GLOBAL 				 4
#define LSS_SM_SELECTIVE_VENDOR 	64
#define LSS_SM_SELECTIVE_PRODUCT 	65
#define LSS_SM_SELECTIVE_REVISION 	66
#define LSS_SM_SELECTIVE_SERIAL 	67
#define LSS_SM_SELECTIVE_RESP 		68
/* Configuration services */
#define LSS_CONF_NODE_ID 			17
#define LSS_CONF_BIT_TIMING 		19
#define LSS_CONF_ACT_BIT_TIMING 	21
#define LSS_CONF_STORE 				23
/* Inquire services */
#define LSS_INQ_VENDOR_ID 		90
#define LSS_INQ_PRODUCT_CODE 	91
#define LSS_INQ_REV_NUMBER 		92
#define LSS_INQ_SERIAL_NUMBER 	93
#define LSS_INQ_NODE_ID 		94
/* Identification services */
#define LSS_IDENT_REMOTE_VENDOR 		70
#define LSS_IDENT_REMOTE_PRODUCT 		71
#define LSS_IDENT_REMOTE_REV_LOW 		72
#define LSS_IDENT_REMOTE_REV_HIGH 		73
#define LSS_IDENT_REMOTE_SERIAL_LOW 	74
#define LSS_IDENT_REMOTE_SERIAL_HIGH 	75
#define LSS_IDENT_REMOTE_NON_CONF 		76
#define LSS_IDENT_SLAVE 				79
#define LSS_IDENT_NON_CONF_SLAVE 		80
#define LSS_IDENT_FASTSCAN				81

/*FastScan State Machine*/
#define LSS_FS_RESET	0
#define LSS_FS_PROCESSING 1
#define LSS_FS_CONFIRMATION 2


typedef void (*LSSCallback_t)(CO_Data* d, UNS8 command);

typedef void (*lss_StoreConfiguration_t)(CO_Data* d,UNS8*,UNS8*);
//void _lss_StoreConfiguration(UNS8 *error, UNS8 *spec_error);

//typedef void (*lss_ChangeBaudRate_t)(CO_Data* d,char*);
//void _lss_ChangeBaudRate(char *BaudRate);


struct struct_lss_transfer;

//#include "timer.h"

#ifdef CO_ENABLE_LSS_FS
struct struct_lss_fs_transfer {
	UNS32 FS_LSS_ID[4];
	UNS8 FS_BitChecked[4];
};

typedef struct struct_lss_fs_transfer lss_fs_transfer_t;
#endif

/* The Transfer structure
* Used to store the different fields of the internal state of the LSS
*/

struct struct_lss_transfer {
  UNS8          state;      /* state of the transmission : Takes the values LSS_... */
  UNS8 			command;     /* the LSS command of the transmision */
  UNS8			mode;       /* LSS mode */

  UNS32 		dat1;		/* the data from the last msg received */
  UNS8			dat2;

  UNS8 nodeID;              /* the new nodeid stored to update the nodeid when switching to LSS operational*/
  UNS8 addr_sel_match;    	/* the matching mask for the LSS Switch Mode Selective service */
  UNS8 addr_ident_match;    /* the matching mask for the LSS Identify Remote Slaves service*/

  char *baudRate;           /* the new baudrate stored to update the node baudrate when a Activate Bit
  							 * Timing Parameters is received*/
  UNS16 switchDelay;		/* the period of the two delay */
  UNS8  switchDelayState;   /* the state machine for the switchDelay */
  FDCAN_HandleTypeDef canHandle_t;

                              /* Time counters to implement a timeout in milliseconds.*/
  TIMER_HANDLE timerMSG;	  /* timerMSG is automatically incremented whenever
                              * the lss state is in LSS_TRANS_IN_PROGRESS, and reseted to 0
                              * when the response LSS have been received.
                              */

  TIMER_HANDLE timerSDELAY;	  /* timerSDELAY is automatically incremented whenever
                              * the lss switchDelayState is in SDELAY_FIRST or SDELAY_SECOND, and reseted to 0
                              * when the two periods have been expired.
                              */

  LSSCallback_t Callback;   /* The user callback func to be called at LSS transaction end */

  UNS8 LSSanswer;			/* stores if a message has been received during a timer period */

#ifdef CO_ENABLE_LSS_FS
  UNS32 IDNumber;			/* in the master, the LSS address parameter which it currently tries to identify.
  							 * in the slave, the LSS address parameter which is being checked (LSS-ID[sub]). */
  UNS8 BitChecked;			/* bits of the current IDNumber that are currently checked */
  UNS8 LSSSub;				/* which part of the LSS-ID is currently checked in IDNumber */
  UNS8 LSSNext;				/* which LSSSub value will be used in the next request */
  UNS8 LSSPos;				/* in the slave, which part of the LSS-ID is currently processed*/
  UNS8 FastScan_SM;			/* the state machine for the FastScan protocol */
  TIMER_HANDLE timerFS;		/* timerFS is automatically incremented when the FastScan service
  							 * has been requested and reseted to 0 when the protocol ends.
                              */
#ifdef CO_ENABLE_LSS_FS
  lss_fs_transfer_t lss_fs_transfer;
#endif

#endif
};

#ifdef CO_ENABLE_LSS
typedef struct struct_lss_transfer lss_transfer_t;
#else
typedef UNS8 lss_transfer_t;
#endif



void startLSS(CO_Data* d);
void stopLSS(CO_Data* d);
UNS8 sendLSS (CO_Data* d, UNS8 command,void *dat1, void *dat2);
UNS8 proceedLSS_Master (CO_Data* d, Message* m );
UNS8 proceedLSS_Slave (CO_Data* d, Message* m );
//UNS8 configNetworkNode(CO_Data* d, UNS8 command, void *dat1, void* dat2);
UNS8 configNetworkNode (CO_Data* d, UNS8 command, void *dat1, void* dat2, LSSCallback_t Callback);
UNS8 getConfigResultNetworkNode (CO_Data* d, UNS8 command, UNS32* dat1, UNS8* dat2);

#endif /* INC_LSS_H_ */
