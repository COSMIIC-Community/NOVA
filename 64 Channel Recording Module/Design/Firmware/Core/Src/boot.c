/**
 * @file boot.c
 *
 *  @date Created on: Oct 11, 2024
 *  @author Jerry Ukwela (jeu6@case.edu)
 */


#include "boot.h"

/**
  * @brief  Initializes node ID and serial number data in the object dictionary
  * @ingroup FESCAN
  */
UNS8 processBOOT(CO_Data* d, Message* m){
	Message r;
	uint8_t node = getNodeId(d);

	if (m->cob_id == 0x140 && m->len == 3 && m->data[0] == node && m->data[1]==0 && m->data[2]==0){
		r.data[0] = ObjDict_obj1018_Product_Code; // module type (last byte of bootloader)
		r.data[1] = 0x02;  // node is running an app
		r.data[2] = (ObjDict_obj1018_Serial_Number) & 0xFF;			//SN LB
		r.data[3] = (ObjDict_obj1018_Serial_Number >> 8) & 0xFF;    //SN HB
		r.data[4] = (ObjDict_obj1018_Revision_Number >> 0) & 0xFF;	//App Version Low Byte
		r.data[5] = (ObjDict_obj1018_Revision_Number >> 8) & 0xFF;	//App Version High Byte
		r.data[6] = node;

		r.len = 7;
		r.rtr = NOT_A_REQUEST;
		r.cob_id = 0x150;

		return canSend(d->canHandle, &r);
	}
	else
	{
		return 0;
	}
}
