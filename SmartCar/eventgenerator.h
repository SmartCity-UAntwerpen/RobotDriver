#ifndef EVENTGENERATOR_H
#define EVENTGENERATOR_H

#include "msgqueue.h"
#include "eventpublisher.h"
#include "trafficlight.h"
#include "drive.h"

#ifdef __cplusplus	//Check if the compiler is C++
	extern "C"	//Code needs to be handled as C-style code
	{
#endif

void* driveFinishedEvent(void* args);

void* trafficLightDetectedEvent(void* args);

void* tagReadEvent(void* args);

void* getDriveDistance(void* args);

void* liftGotoEvent(void* args);

void* getLiftHeight(void* args);

msg_t* _createDefaultEventMsg(void);

#ifdef __cplusplus		//Check if the compiler is C++
	}		//End the extern "C" bracket
#endif

#endif
