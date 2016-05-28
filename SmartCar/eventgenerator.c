#include "eventgenerator.h"

void* driveFinishedEvent(void* args)
{
    msg_t* event = _createDefaultEventMsg();

    event->id = EVENT_DRIVE_FINISHED;

    publishEvent(event);

    return NULL;
}

void* trafficLightDetectedEvent(void* args)
{
    char* detectedString;
    void* eventValue;
    msg_t* event = _createDefaultEventMsg();

    event->id = EVENT_TRAFFICLIGHT_DETECTION;
    event->numOfParm = 1;

    switch(*(int*)args)
    {
        case TRAFFICLIGHT_GREEN:
            detectedString = "GREEN";
            break;
        case TRAFFICLIGHT_RED:
            detectedString = "RED";
            break;
        case TRAFFICLIGHT_NONE:
            detectedString = "NONE";
            break;
        default:
            detectedString = "ERROR";
            break;
    }

    eventValue = malloc(strlen(detectedString) + 1);
    memcpy(eventValue, detectedString, strlen(detectedString) + 1);

    event->values = eventValue;

    publishEvent(event);

    return NULL;
}

void* tagReadEvent(void* args)
{
    void* eventValue;
    msg_t* event = _createDefaultEventMsg();

    event->id = EVENT_TAG_DETECTION;
    event->numOfParm = 1;

    eventValue = malloc(strlen((char*)args) + 1);
    memcpy(eventValue, args, strlen((char*)args) + 1);

    event->values = eventValue;

    publishEvent(event);

    return NULL;
}

void* getDriveDistance(void* args)
{
    void* eventValue;
    msg_t* event = _createDefaultEventMsg();
    int wheelL, wheelR;
    int wheelAverage;

    wheelAverage = getWheelPosition(&wheelL, &wheelR);

    event->id = EVENT_TRAVEL_DISTANCE;
    event->numOfParm = 1;

    eventValue = malloc(sizeof(int));
    memcpy(eventValue, (void*)&wheelAverage, sizeof(int));

    event->values = eventValue;

    publishEvent(event);

    return NULL;
}

void* liftGotoEvent(void* args)
{
    void* eventValue;
    msg_t* event = _createDefaultEventMsg();

    event->id = EVENT_LIFT_GOTO;
    event->numOfParm = 1;

    eventValue = malloc(strlen((char*)args) + 1);
    memcpy(eventValue, args, strlen((char*)args) + 1);

    event->values = eventValue;

    publishEvent(event);

    return NULL;
}

void* getLiftHeight(void* args)
{
    void* eventValue;
    msg_t* event;
    float liftHeight;

    if(LiftGetHeight(&liftHeight) == 0)
    {
        event = _createDefaultEventMsg();

        event->id = EVENT_LIFT_HEIGHT;
        event->numOfParm = 1;

        eventValue = malloc(sizeof(float));
        memcpy(eventValue, (void*)&liftHeight, sizeof(float));

        event->values = eventValue;

        publishEvent(event);
    }

    return NULL;
}

msg_t* _createDefaultEventMsg(void)
{
    msg_t* eventMsg = (msg_t*) malloc (sizeof(msg_t));

    eventMsg->type = EVENT_MSG;
    eventMsg->id = MSG_ID_EVENT_TOTAL;
    eventMsg->numOfParm = 0;
    eventMsg->values = NULL;
    eventMsg->Next = NULL;

    return eventMsg;
}
