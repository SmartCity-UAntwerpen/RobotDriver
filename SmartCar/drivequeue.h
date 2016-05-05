#ifndef DRIVEQUEUE_H
#define DRIVEQUEUE_H

#include <time.h>
#include "msgqueue.h"
#include "drive.h"

#ifdef __cplusplus	//Check if the compiler is C++
	extern "C"	//Code needs to be handled as C-style code
	{
#endif

typedef void* driveFinishedCallback_t(void* args);

int initDriveQueue(void);

int deinitDriveQueue(void);

void setDriveFinishedCallback(driveFinishedCallback_t callback);

msgqueue_t* getDriveQueue(void);

void* _processDriveMessage(void* args);

int _startNextActivity(struct msg_t* message);

#ifdef __cplusplus		//Check if the compiler is C++
	}		//End the extern "C" bracket
#endif

#endif
