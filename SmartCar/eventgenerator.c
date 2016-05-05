#include "eventgenerator.h"

void* driveFinishedEvent(void* args)
{
    msg_t* event = _createDefaultEventMsg();

    event->id = EVENT_DRIVE_FINISHED;

    publishEvent(event);

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
