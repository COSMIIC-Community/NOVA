/**
 * @file def.h
 *
 *  Created on: Oct 9, 2024
 *      Author: jenej
 *
 */

#ifndef INC_DEF_H_
#define INC_DEF_H_

#include "config.h"

/** definitions used for object dictionary access. ie SDO Abort codes . (See DS 301 v.4.02 p.48)
 */
#define OD_SUCCESSFUL 	             0x00000000
#define OD_READ_NOT_ALLOWED          0x06010001
#define OD_WRITE_NOT_ALLOWED         0x06010002
#define OD_NO_SUCH_OBJECT            0x06020000
#define OD_NOT_MAPPABLE              0x06040041
#define OD_LENGTH_DATA_INVALID       0x06070010
#define OD_NO_SUCH_SUBINDEX 	     0x06090011
#define OD_VALUE_RANGE_EXCEEDED      0x06090030 /* Value range test result */
#define OD_VALUE_TOO_LOW             0x06090031 /* Value range test result */
#define OD_VALUE_TOO_HIGH            0x06090032 /* Value range test result */
/* Others SDO abort codes
 */
#define SDOABT_TOGGLE_NOT_ALTERNED   0x05030000
#define SDOABT_TIMED_OUT             0x05040000
#define SDOABT_OUT_OF_MEMORY         0x05040005 /* Size data exceed SDO_MAX_LENGTH_TRANSFERT */
#define SDOABT_GENERAL_ERROR         0x08000000 /* Error size of SDO message */
#define SDOABT_LOCAL_CTRL_ERROR      0x08000021
#define SDOABT_APP_TIMEOUT           0x08000040

/******************** CONSTANTS ****************/

/** Constantes which permit to define if a PDO frame
   is a request one or a data one
*/
/* Should not be modified */
#define REQUEST 1
#define NOT_A_REQUEST 0

/* Misc constants */
/* -------------- */
/* Should not be modified */
#define Rx 0
#define Tx 1
#ifndef TRUE
#define TRUE  1
#endif
#ifndef FALSE
#define FALSE 0
#endif

#define ERR_NOERROR 0

/** Status of the SDO transmission
 */
#define SDO_RESET                0x0      /* Transmission not started. Init state. */
#define SDO_FINISHED             0x1      /* data are available */
#define	SDO_ABORTED_RCV          0x80     /* Received an abort message. Data not available */
#define	SDO_ABORTED_INTERNAL     0x85     /* Aborted but not because of an abort message (Timeout) */
#define	SDO_DOWNLOAD_IN_PROGRESS 0x2
#define	SDO_UPLOAD_IN_PROGRESS   0x3


/* Status of the node during the SDO transfer : */
#define SDO_SERVER  0x1
#define SDO_CLIENT  0x2
#define SDO_UNKNOWN 0x3

/* SDOrx ccs: client command specifier */
#define DOWNLOAD_SEGMENT_REQUEST     0
#define INITIATE_DOWNLOAD_REQUEST    1
#define INITIATE_UPLOAD_REQUEST      2
#define UPLOAD_SEGMENT_REQUEST       3
#define ABORT_TRANSFER_REQUEST       4

/* SDOtx scs: server command specifier */
#define UPLOAD_SEGMENT_RESPONSE      0
#define DOWNLOAD_SEGMENT_RESPONSE    1
#define INITIATE_DOWNLOAD_RESPONSE   3
#define INITIATE_UPLOAD_RESPONSE     2
#define ABORT_TRANSFER_REQUEST       4


/*  Function Codes
   ---------------

*/
#define NMT	   		0x0
#define SYNC       	0x1
#define BOOT       	0x2
#define PDO1tx     	0x3
#define PDO1rx     	0x4
#define PDO2tx     	0x5
#define PDO2rx     	0x6
#define PDO3tx     	0x7
#define PDO3rx     	0x8
#define PDO4tx     	0x9
#define PDO4rx     	0xA
#define SDOtx      	0xB
#define SDOrx      	0xC
#define NODE_GUARD 	0xE
#define LSS 	   	0xF

/* NMT Command Specifier, implemented by a master device */
/* ------------------------------------------------------------- */
/* Broadcast Network command */
#define NMT_Start_Nodes               0x01
#define NMT_Stop_Nodes                0x02
#define NMT_Enter_Patient_Operation   0x03
#define NMT_Enter_X_Manual            0x04
#define NMT_Enter_Y_Manual            0x05
#define NMT_Enter_Stop_Stim           0x06
#define NMT_Enter_Wait_Mode           0x07    /* Command to return to waiting for next */
#define NMT_Enter_Patient_Manual      0x08    /* corresponds to 'Test Patient Mode ' */
#define NMT_Enter_Produce_X_Manual    0x09
#define NMT_Do_Save_Cmd               0x0A
#define NMT_Do_Restore_Cmd            0x0B
#define NMT_Enter_Record_X            0x0C
#define NMT_Net_PDO_On                0x0D
#define NMT_Net_PDO_Off               0x0E

/* Broadcast Network management */
#define NMT_Return_Node_Table	      0x10
#define NMT_Return_Single_Node	      0x11

/* Group Management   */
#define NMT_Group_On                  0x20
#define NMT_Group_Off                 0x21
#define NMT_All_Groups_Off            0x22


/* Local Node command */
#define NMT_Reset_Node                0x81
#define NMT_Reset_Comunication        0x82
#define NMT_Enter_Bootloader          0x83
#define NMT_Start_Sync                0x84
#define NMT_Stop_Sync                 0x85
#define NMT_Start_PDO                 0x86
#define NMT_Stop_PDO                  0x87
#define NMT_Set_RTC                   0x88
#define NMT_Set_Alarm                 0x89
#define NMT_Reset_Watchdog            0x8A
#define NMT_Radio_WOR_ON              0x8B
#define NMT_Radio_WOR_OFF             0x8C
#define NMT_Run_Script                0x8D
#define NMT_Enable_Script             0x8E
#define NMT_Disable_Script            0x8F
#define NMT_Erase_Serial_Eprom        0x90
#define NMT_Reset_Block_Counter       0x91
#define NMT_Radio_Address             0x92
#define NMT_Wake_Remote_Modules       0x93
#define NMT_WOR_Start_POLL            0x94
#define NMT_Network_On                0x95
#define NMT_Network_Off               0x96
#define NMT_Halt_RTC                  0x97
#define NMT_Clear_CAN_Errors          0x98
#define NMT_Set_Radio_Power           0x99
#define NMT_Init_NV_Memory            0x9A
#define NMT_Init_CAN                  0x9B
#define NMT_Reset_Radio_Counters      0x9C
#define NMT_Reset_OD_Defaults         0x9D
#define NMT_Reset_Module              0x9E
#define NMT_Enter_Low_Power           0x9F
#define NMT_Start_SDO_Graph           0xA1
#define NMT_Stop_SDO_Graph            0xA2
#define NMT_Start_HS_Graph            0xA3
#define NMT_Stop_HS_Graph             0xA4

/* Local Node command (module implementation) */
#define NMT_Set_IMU_Mode			  0xC3		///< Set IMU mode (ISPU, Accelerometer, and Gyroscope)

/** Status of the LSS transmission
 */
#define LSS_RESET                0x0      /* Transmission not started. Init state. */
#define LSS_FINISHED             0x1      /* data are available */
#define	LSS_ABORTED_INTERNAL     0x2     /* Aborted but not because of an abort message. */
#define	LSS_TRANS_IN_PROGRESS 	 0x3

/* constantes used in the different state machines */
/* ----------------------------------------------- */
/* Must not be modified -- used in PDO.c */
#define state1  0x01
#define state2  0x02
#define state3  0x03
#define state4  0x04
#define state5  0x05
#define state6  0x06
#define state7  0x07
#define state8  0x08
#define state9  0x09
#define state10 0x0A
#define state11 0x0B

#endif /* INC_DEF_H_ */
