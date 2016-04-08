#include "travel.h"

int Travel(NodeStruct* Map, int MapSize, int Start, int Finish, float Speed)
{
    bool hasNext = true;
    int previousPoint = Start;

    msg_t *driveMsg;
    msg_t *turnMsg;

    if(Map == NULL)
    {
        //Empty map
        return 1;
    }

    do
    {
        driveMsg = (msg_t*) malloc(sizeof(msg_t));
        turnMsg = (msg_t*) malloc(sizeof(msg_t));

        turnMsg->type = DRIVE_MSG;
        turnMsg->id = 0;
        turnMsg->numOfParm = 0;
        turnMsg->values = NULL;
        turnMsg->Next = NULL;

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

        addMsg(getDriveQueue(), turnMsg);              //Turn action on intersection
        addMsg(getDriveQueue(), driveMsg);             //Drive to end of line

        previousPoint = Map[previousPoint].Next;  //Get next node

        if(Map[previousPoint].Next == -1)
        {
            hasNext = false;                      //End of route
        }

    } while(hasNext);

    return 0;
}
