#ifndef DRIVEQUEUE_H
#define DRIVEQUEUE_H

#include <time.h>
#include "msgqueue.h"
#include "drive.h"

#ifdef __cplusplus	//Check if the compiler is C++
	extern "C"	//Code needs to be handled as C-style code
	{
#endif

struct msgqueue_t* driveQueue;

int initDriveQueue(void);

void* _processDriveMessage(void* args);

int _startNextActivity(struct msg_t* message);

#ifdef __cplusplus		//Check if the compiler is C++
	}		//End the extern "C" bracket
#endif

#endif
