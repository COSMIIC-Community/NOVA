/**
 * @file app.h
 *
 *  Created on: @date Oct 14, 2024
 *      @author Jerry Ukwela (jeu6@case.edu)
 *      @brief 	Header file for app.c
 */

#ifndef APP_MODULE_APPTASK_H_
#define APP_MODULE_APPTASK_H_


void initAppTask( void );
void updateAppTask( void );
void runAppTask( UNS32 );
void processSYNCMessageForApp(Message* m);
void sleepApplication( void );
void InitAppTaskValues( void );

void setup_intan ( void );

void StartNodesFunc(CO_Data* d, Message *m);
void StopNodesFunc(CO_Data* d, Message *m);
void EnterWaitingFunc(CO_Data* d, Message *m);
void EnterPatientOperationFunc(CO_Data* d, Message *m);
void EnterXManualFunc(CO_Data* d, Message *m);
void EnterYManualFunc(CO_Data* d, Message *m);
void EnterStopStimFunc(CO_Data* d, Message *m);
void EnterPatientManualFunc(CO_Data* d, Message *m);
void EnterProduceXManualFunc(CO_Data* d, Message *m);
void EnterRecordXFunc(CO_Data* d, Message *m);

// Additional module functions can be added to appTask

#endif /* APP_MODULE_APPTASK_H_ */
