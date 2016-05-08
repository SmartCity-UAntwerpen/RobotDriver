#ifndef MODULEPROCESS_H
#define MODULEPROCESS_H

#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include "eventgenerator.h"
#include "tagreader.h"

#ifdef __cplusplus	//Check if the compiler is C++
	#include "camera.h"
    #include "trafficlightdetect.h"

	extern "C"	//Code needs to be handled as C-style code
	{
#endif

typedef struct module_process_t
{
    pthread_t processThread;
    pthread_mutex_t processThreadLock;
    bool running;
    bool cancel;
} module_process_t;

typedef enum module_id
{
    MODULE_TAGREADER,
    MODULE_CAMERA,
    MODULE_ID_TOTAL
} module_id;

typedef void* moduleProcessFunction_t(void* args);

int initProcessModules(void);

int stopProcessModules(void);

int initProcessModule(module_process_t* processModule);

int stopProcessModule(module_process_t* processModule);

int _createModuleProcess(module_id id, moduleProcessFunction_t functionCall, void* functionArgs);

int startTrafficLightDetection(void);

void* _trafficLightDetectionProcess(void* args);

int startReadTagUID(void);

void* _readTagUIDProcess(void* args);

#ifdef __cplusplus		//Check if the compiler is C++
	}		//End the extern "C" bracket
#endif

#endif
