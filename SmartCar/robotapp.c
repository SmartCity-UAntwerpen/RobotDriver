#include "robotapp.h"

pthread_t commThread;

void RobotApp(int argc, char *argv[])
{
    struct msg *newMsg;

    setSpeakerMute(false);

    //Read arguments
    /*if(argc < 1)
    {
        printf("Insufficient arguments provided!\n");

        return;
    }*/

    //Setup motordrivers
    DriveInit();

    //Initialize map
    //initializeMap();

    printf("Program up and running...\n");
    espeak("Initialisation complete.");

    if(runQueueDispatcher() != 0)
    {
        printf("Failed to start queue dispatcher!");
    }

    sleep(2);

    printf("WARNING: Loading drive commands\n");
    espeak("WARNING! Stand back!");

    int i = 0;

    while(i < 2)
    {
        newMsg = (struct msg *) malloc (sizeof(struct msg));

        newMsg->id = DRIVE_STRAIGHT;
        newMsg->numOfParm = 0;
        newMsg->values = NULL;
        newMsg->Next = NULL;

        addMsg(newMsg);

        printf("Message added: DRIVE_STRAIGHT\n");

        sleep(1);

        newMsg = (struct msg *) malloc (sizeof(struct msg));

        newMsg->id = DRIVE_RIGHT;
        newMsg->numOfParm = 0;
        newMsg->values = NULL;
        newMsg->Next = NULL;

        addMsg(newMsg);

        printf("Message added: DRIVE_RIGHT\n");

        sleep(1);

        //i++;
    }

    sleep(5);

    printf("Abort driving\n");

    AbortDriving();

    sleep(2);

    flushQueue();

    newMsg = (struct msg *) malloc (sizeof(struct msg));

    newMsg->id = DRIVE_STRAIGHT;
    newMsg->numOfParm = 0;
    newMsg->values = NULL;
    newMsg->Next = NULL;

    addMsg(newMsg);

    addMsg(newMsg);

    printf("Message added: DRIVE_STRAIGHT\n");

    while(1)
    {

    }
/*
while(1)
{
    DriveLineFollow(80);

    espeak("Driving forward.");

    WaitForDriving();

    //DriveRotateCenter(180, 70);
    DriveRotateLWheel(90, 80);

    espeak("Turning.");

    WaitForDriving();

    //DriveLineFollow(80);

    //espeak("Driving forward.");

    //WaitForDriving();

    //DriveRotateCenter(-180, 70);

    //espeak("Turning.");

    //WaitForDriving();

    //espeak("Ready. Smart City.");
}*/
}
