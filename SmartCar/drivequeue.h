#ifndef DRIVEQUEUE_H
#define DRIVEQUEUE_H

#include <time.h>
#include "msgqueue.h"
#include "drive.h"

#ifdef __cplusplus	//Check if the compiler is C++
	extern "C"	//Code needs to be handled as C-style code
	{
#endif

int initDriveQueue(void);

msgqueue_t* getDriveQueue(void);

void* _processDriveMessage(void* args);

int _startNextActivity(struct msg_t* message);

#ifdef __cplusplus		//Check if the compiler is C++
	}		//End the extern "C" bracket
#endif

#endif
