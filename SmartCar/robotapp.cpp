#include "robotapp.h"

pthread_t commThread;

void RobotApp(int argc, char *argv[])
{
    struct msg *newMsg;

    setSpeakerMute(true);

    //Read arguments
    /*if(argc < 1)
    {
        printf("Insufficient arguments provided!\n");

        return;
    }*/

    //Initialize Drivequeue
    initDriveQueue();

    //Setup motordrivers
    DriveInit();

    //Initialize camera
    if(initCamera() > 1)
    {
        printf("Failed to initialize camera!\n");
    }

    //Initialize map
    parseMapFile("MAP.dmap");

    printf("Program up and running...\n");
    espeak("Initialisation complete.");

    if(startQueue(driveQueue) != 0)
    {
        printf("Failed to start queue dispatcher!");
    }

    printf("WARNING: Loading drive commands\n");
    espeak("WARNING! Stand back!");

    Dijkstra(getRoadMap(), getRoadMapSize(), 13, 7);

    printf("DIJKSTRA FINISHED\n");

    Travel(getRoadMap(), getRoadMapSize(), 13, 7, 70);

    while(1);


/*
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

        newMsg = (struct msg *) malloc (sizeof(struct msg));

        newMsg->id = DRIVE_RIGHT;
        newMsg->numOfParm = 0;
        newMsg->values = NULL;
        newMsg->Next = NULL;

        addMsg(newMsg);

        printf("Message added: DRIVE_RIGHT\n");

        i++;
    }

/*
    WaitForDriving();
    sleep(1);
    flushQueue();
    AbortDriving();

    newMsg = (struct msg *) malloc (sizeof(struct msg));

    newMsg->id = DRIVE_STRAIGHT;
    newMsg->numOfParm = 0;
    newMsg->values = NULL;
    newMsg->Next = NULL;

    //addMsg(newMsg);

    //printf("Message added: DRIVE_STRAIGHT\n");

while(1)
{
    sleep(2);

    pauseDriving();

    sleep(1);

    continueDriving();
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
