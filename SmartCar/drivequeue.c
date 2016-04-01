#include "drivequeue.h"

static pthread_t queueThread;
static pthread_mutex_t queueLock;

int initDriveQueue(void)
{
    //Allocated message queue struct
    driveQueue = (msgqueue_t*) malloc (sizeof(msgqueue_t));

    if(driveQueue != NULL)
    {
        //Initialize parameters
        driveQueue->processMessage = (processMessageFunction) _processDriveMessage;
        driveQueue->queueActiveFlag = false;
        driveQueue->queueLock = queueLock;
        driveQueue->queuePointer = NULL;
        driveQueue->queueThread = queueThread;
        driveQueue->queueThreadRunning = false;

        return 0;
    }
    else
    {
        //Queue is not properly allocated!
        return 1;
    }
}

void* _processDriveMessage(void* args)
{
    struct timespec time;
    struct msg_t *msgPointer = NULL;

    time.tv_sec = 0;
    time.tv_nsec = 10000000;

    while(driveQueue->queueActiveFlag)
    {
        if(IsDriving())
        {
            //Wait for drivethread to finish
            WaitForDriving();
        }

        pthread_mutex_lock(&driveQueue->queueLock);

        if(driveQueue->queuePointer != NULL && driveQueue->queueActiveFlag)
        {
            msgPointer = driveQueue->queuePointer;

            //Remove message from queue
            driveQueue->queuePointer = driveQueue->queuePointer->Next;
        }

        pthread_mutex_unlock(&driveQueue->queueLock);

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

int _startNextActivity(struct msg_t* message)
{
    switch(message->id)
    {
        case DRIVE_STRAIGHT:
            return DriveLineFollow(80);
        case DRIVE_D_STRAIGHT:
            return DriveLineFollowDistance(120, 80);
        case DRIVE_RIGHT:
            return DriveRotateLWheel(90, 70);
        case DRIVE_LEFT:
            return DriveRotateRWheel(90, 70);
        default:
            printf("Command id: %d unknown!", message->id);
            return -1;
    }
}
