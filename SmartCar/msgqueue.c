#include "msgqueue.h"

pthread_t dispatchThread;
pthread_mutex_t queueLock;

struct msg *queuePointer;

bool queueActiveFlag = false;
bool _dispatchThreadrunning = false;

int runQueueDispatcher(void)
{
    queueActiveFlag = true;

    if(!_dispatchThreadrunning)
    {
        if(pthread_create(&dispatchThread, NULL, _processMessage, NULL) < 0)
        {
            printf("Error while creating dispatch thread!\n");

            return -2;
        }

        _dispatchThreadrunning = true;

        return 0;
    }
    else
    {
        return -1;
    }
}

int stopQueueDispatcher(void)
{
    queueActiveFlag = false;

    //Wait for dispatch thread to finish
    pthread_join(dispatchThread, NULL);

    _dispatchThreadrunning = false;

    return 0;
}

bool msgQueueActive(void)
{
    return _dispatchThreadrunning;
}

void* _processMessage(void* args)
{
    struct timespec time;
    struct msg *msgPointer = NULL;

    time.tv_sec = 0;
    time.tv_nsec = 10000000;

    while(queueActiveFlag)
    {
        if(IsDriving())
        {
            //Wait for drivethread to finish
            WaitForDriving();
        }

        pthread_mutex_lock(&queueLock);

        if(queuePointer != NULL && queueActiveFlag)
        {
            msgPointer = queuePointer;

            //Remove message from queue
            queuePointer = queuePointer->Next;
        }

        pthread_mutex_unlock(&queueLock);

        if(msgPointer != NULL)
        {
            //Start next command
            _startNextActivity(msgPointer);

            //Free message
            free(msgPointer);
        }

        msgPointer = NULL;

        nanosleep(&time, NULL);
    }

    return NULL;
}

int _startNextActivity(struct msg* message)
{
    switch(message->id)
    {
        case DRIVE_STRAIGHT:
            return DriveLineFollow(80);
        case DRIVE_RIGHT:
            return DriveRotateLWheel(90, 70);
        case DRIVE_LEFT:
            return DriveRotateRWheel(90, 70);
        default:
            printf("Command id: %d unknown!", message->id);
            return -1;
    }
}

int flushQueue(void)
{
    struct msg *msgPointer;

    pthread_mutex_lock(&queueLock);

    while(queuePointer != NULL)
    {
        //Set temparory pointer to first element
        msgPointer = queuePointer;

        //Switch front of queue to next element
        queuePointer = queuePointer->Next;

        //Free msgPointer
        free(msgPointer);
    }

    pthread_mutex_unlock(&queueLock);

    return 0;
}

int addMsg(struct msg* message)
{
    struct msg *msgPointer;

    pthread_mutex_lock(&queueLock);

    //Check if queue is empty
    if(queuePointer == NULL)
    {
        queuePointer = message;
    }
    else
    {
        msgPointer = queuePointer;

        while(msgPointer->Next != NULL)
        {
            msgPointer = msgPointer->Next;
        }

        //Add new message to the end of the queue
        msgPointer->Next = message;
    }

    pthread_mutex_unlock(&queueLock);

    return 0;
}
