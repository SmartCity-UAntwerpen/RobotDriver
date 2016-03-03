#include "robotapp.h"

pthread_t commThread;

void RobotApp(int argc, char *argv[])
{
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

    printf("Initialisation complete\n");
    espeak("Initialisation complete.");

    //QUEUE
}
