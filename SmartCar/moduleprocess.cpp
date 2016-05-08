#include "moduleprocess.h"

static module_process_t moduleProcesses[MODULE_ID_TOTAL];

int initProcessModules(void)
{
    int i;

    for(i = 0; i < MODULE_ID_TOTAL; i++)
    {
        initProcessModule(&moduleProcesses[i]);
    }

    return 0;
}

int stopProcessModules(void)
{
    int i;

    for(i = 0; i < MODULE_ID_TOTAL; i++)
    {
        stopProcessModule(&moduleProcesses[i]);
    }

    return 0;
}

int initProcessModule(module_process_t* processModule)
{
    if(processModule == NULL)
    {
        //Process module can not be NULL
        return 1;
    }

    //Set default values
    processModule->processThread = 0;
    processModule->running = false;
    processModule->cancel = false;

    return 0;
}

int stopProcessModule(module_process_t* processModule)
{
    if(processModule == NULL)
    {
        //Process module can not be NULL
        return 1;
    }

    if(processModule->running == true)
    {
        pthread_mutex_lock(&processModule->processThreadLock);

        //Set cancel flag high
        processModule->cancel = true;

        pthread_join(processModule->processThread, NULL);

        processModule->running = false;
        processModule->cancel = false;

        pthread_mutex_unlock(&processModule->processThreadLock);
    }
    else
    {
        //Process not running
    }

    return 0;
}

int _createModuleProcess(module_id id, moduleProcessFunction_t functionCall, void* functionArgs)
{
    if(id >= MODULE_ID_TOTAL)
    {
        //Invalid id
        return 3;
    }

    pthread_mutex_lock(&moduleProcesses[id].processThreadLock);

    if(!moduleProcesses[id].running)
    {
        if(pthread_create(&moduleProcesses[id].processThread, NULL, functionCall, functionArgs) < 0)
        {
            printf("Error while creating module %d process thread!\n", id);

            pthread_mutex_unlock(&moduleProcesses[id].processThreadLock);

            return 2;
        }

        moduleProcesses[id].running = true;

        pthread_mutex_unlock(&moduleProcesses[id].processThreadLock);

        return 0;
    }
    else
    {
        //Resource is busy
        pthread_mutex_unlock(&moduleProcesses[id].processThreadLock);

        return 1;
    }
}


int startTrafficLightDetection(void)
{
    return _createModuleProcess(MODULE_CAMERA, _trafficLightDetectionProcess, NULL);
}

void* _trafficLightDetectionProcess(void* args)
{
    int result;

    result = detectTrafficLight(getCameraCapture());

    trafficLightDetectedEvent(&result);

    moduleProcesses[MODULE_CAMERA].running = false;

    return NULL;
}

int startReadTagUID(void)
{
    return _createModuleProcess(MODULE_TAGREADER, _readTagUIDProcess, NULL);
}

void* _readTagUIDProcess(void* args)
{
    int result = 0;
    char UIDTag[24];

    result = TagReaderGetUID(UIDTag);

    if(result != 0)
    {
        //Error occurred while reading tag
        strcpy(UIDTag, "NONE");
    }

    tagReadEvent(&UIDTag);

    moduleProcesses[MODULE_TAGREADER].running = false;

    return NULL;
}
