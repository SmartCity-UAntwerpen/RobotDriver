#include "drivequeue.h"

static pthread_t queueThread;
static pthread_mutex_t queueLock;

static struct msgqueue_t* driveQueue;

static driveFinishedCallback_t* driveFinishedCallback;

int initDriveQueue(void)
{
    //Allocated message queue struct
    driveQueue = (msgqueue_t*) malloc (sizeof(msgqueue_t));

    if(driveQueue != NULL)
    {
        //Initialise parameters
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

int deinitDriveQueue(void)
{
    if(driveQueue == NULL)
    {
        //Drive queue is not initialised
        return 1;
    }

    if(driveQueue->queueActiveFlag == true)
    {
        //Can not destroy drive queue while active
        return 2;
    }

    //Flush all queue elements
    flushQueue(driveQueue);

    //Free memory of drive queue
    free(driveQueue);

    driveQueue = NULL;

    return 0;
}

void setDriveFinishedCallback(driveFinishedCallback_t callback)
{
    driveFinishedCallback = callback;
}

msgqueue_t* getDriveQueue(void)
{
    return driveQueue;
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

            //Signal driving finished
            if(driveFinishedCallback != NULL)
            {
                driveFinishedCallback(NULL);
            }
        }

        if(!queueIsEmpty(driveQueue) && driveQueue->queueActiveFlag)
        {
            msgPointer = getNextMsg(driveQueue);
        }

        if(msgPointer != NULL)
        {
            //Start next command
            _startNextActivity(msgPointer);

            //Free message
            freeMsg(msgPointer);
        }

        msgPointer = NULL;

        nanosleep(&time, NULL);
    }

    return NULL;
}

int _startNextActivity(struct msg_t* message)
{
    int dist_angle = 0;

    switch(message->id)
    {
        case DRIVE_FOLLOWLINE:
            return DriveLineFollow(80);
        case DRIVE_FOLLOWLINE_DISTANCE:
            dist_angle = ((int*)message->values)[0];
            return DriveLineFollowDistance(dist_angle, 80);
        case DRIVE_STRAIGHT_DISTANCE:
            dist_angle = ((int*)message->values)[0];
            return DriveStraightDistance(dist_angle, 80);
        case DRIVE_BACKWARDS_DISTANCE:
            dist_angle = ((int*)message->values)[0];
            return DriveStraightDistance(-dist_angle, 80);
        case DRIVE_TURN_RIGHT:
            return DriveRotateLWheel(90, 70);
        case DRIVE_ANGLE_RIGHT:
            dist_angle = ((int*)message->values)[0];
            return DriveRotateLWheel(dist_angle, 70);
        case DRIVE_ROTATE_RIGHT:
            dist_angle = ((int*)message->values)[0];
            return DriveRotateCenter(dist_angle, 80);
        case DRIVE_TURN_LEFT:
            return DriveRotateRWheel(90, 70);
        case DRIVE_ANGLE_LEFT:
            dist_angle = ((int*)message->values)[0];
            return DriveRotateRWheel(dist_angle, 70);
        case DRIVE_ROTATE_LEFT:
            dist_angle = ((int*)message->values)[0];
            return DriveRotateCenter(-dist_angle, 80);
        default:
            printf("Command id: %d unknown!", message->id);
            return -1;
    }
}
