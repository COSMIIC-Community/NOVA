/**
 * @file nmtSlave.h
 *
 *  @date Created on: Oct 9, 2024
 *  @author Jerry Ukwela (jeu6@case.edu)
 */

#ifndef INC_NMTSLAVE_H_
#define INC_NMTSLAVE_H_

//static UNS8 skipTime = 0;
void processNMTstateChange (CO_Data* d, Message * m);
UNS8 slaveSendBootUp (CO_Data* d);
void StartWatchDog ( CO_Data* d, UNS16 timebase );
void StopWatchDog ( CO_Data* d );
void StartRecording (UNS8 data);
void setNodeStateToStopped( void );

#endif /* INC_NMTSLAVE_H_ */
