#include "travel.h"

int Travel(NodeStruct* Map, int MapSize, int Start, int Finish, float Speed)
{
    bool hasNext = true;
    int previousPoint = Start;

    struct msg *driveMsg;
    struct msg *turnMsg;

    do
    {
        driveMsg = (struct msg*) malloc(sizeof(struct msg));
        turnMsg = (struct msg*) malloc(sizeof(struct msg));

        turnMsg->id = 0;
        turnMsg->numOfParm = 0;
        turnMsg->values = NULL;
        turnMsg->Next = NULL;
printf("NODE: %s\n", Map[5].RFID);
        switch(Map[previousPoint].NextRelDir)
        {
            case 1:
                printf("Turn left\n");
                turnMsg->id = DRIVE_LEFT;         //Turn left
                break;
            case 2:
                printf("Crossing intersection\n");
                turnMsg->id = DRIVE_D_STRAIGHT;   //Straight forward
                break;
            case 3:
                printf("Turn right\n");
                turnMsg->id = DRIVE_RIGHT;        //Turn right
                break;
        }

        driveMsg->id = DRIVE_STRAIGHT;
        driveMsg->numOfParm = 0;
        driveMsg->values = NULL;
        driveMsg->Next = NULL;

        addMsg(turnMsg);                          //Turn action on intersection
        addMsg(driveMsg);                         //Drive to end of line

        previousPoint = Map[previousPoint].Next;        //Get next node

        if(Map[previousPoint].Next == -1)
        {
            hasNext = false;                            //End of route
        }

    } while(hasNext);

    return 0;
}
