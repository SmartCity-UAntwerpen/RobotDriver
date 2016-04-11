#include "msgqueue.h"

int startQueue(msgqueue_t* msgqueue)
{
    msgqueue->queueActiveFlag = true;

    if(!msgqueue->queueThreadRunning)
    {
        if(pthread_create(&msgqueue->queueThread, NULL, (void* (*)(void*))msgqueue->processMessage, NULL) < 0)
        {
            printf("Error while creating dispatch thread!\n");

            return 2;
        }

        msgqueue->queueThreadRunning = true;

        return 0;
    }
    else
    {
        return 1;
    }
}

int stopQueue(msgqueue_t* msgqueue)
{
    msgqueue->queueActiveFlag = false;

    //Wait for dispatch thread to finish
    pthread_join(msgqueue->queueThread, NULL);

    msgqueue->queueThreadRunning = false;

    return 0;
}

int flushQueue(msgqueue_t* msgqueue)
{
    msg_t *msgPointer;

    pthread_mutex_lock(&msgqueue->queueLock);

    while(msgqueue->queuePointer != NULL)
    {
        //Set temparory pointer to first element
        msgPointer = msgqueue->queuePointer;

        //Switch front of queue to next element
        msgqueue->queuePointer = msgqueue->queuePointer->Next;

        //Free msgPointer
        free(msgPointer);
    }

    pthread_mutex_unlock(&msgqueue->queueLock);

    return 0;
}

int addMsg(msgqueue_t* msgqueue, msg_t* message)
{
    msg_t *msgPointer;

    pthread_mutex_lock(&msgqueue->queueLock);

    //Check if queue is empty
    if(msgqueue->queuePointer == NULL)
    {
        msgqueue->queuePointer = message;
    }
    else
    {
        msgPointer = msgqueue->queuePointer;

        while(msgPointer->Next != NULL)
        {
            msgPointer = msgPointer->Next;
        }

        //Add new message to the end of the queue
        msgPointer->Next = message;
    }

    pthread_mutex_unlock(&msgqueue->queueLock);

    return 0;
}
