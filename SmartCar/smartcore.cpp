#include "smartcore.h"

using namespace SC;

//Default constructor
SmartCore::SmartCore()
{
    running = false;
    abort = false;
    mode = MODE_STANDALONE;
}

SmartCore::~SmartCore()
{
    printf("Core shutdown...\n");

    //Stop drive queue
    if(getDriveQueue() != NULL)
    {
        stopQueue(getDriveQueue());
    }

    //Stop watchdog
    stopWatchdog();

    //Stop driving
    AbortDriving();

    //Stop RestInterface
    stopRestInterface();

    //Stop camera
    closeCamera();

    //Destroy configuration
    deinitConfiguration();
}

int SmartCore::initialiseCore(int argc, char *argv[])
{
    int res;

    AnsiSetColor(ANSI_ATTR_OFF,ANSI_BLACK,ANSI_GREEN);
    printf("Initialising SmartCore...\n");

    //Initialise drivequeue
    AnsiSetColor(ANSI_ATTR_OFF,ANSI_BLACK,ANSI_WHITE);
    printf("Init drivequeue...");
    res = initDriveQueue();
    if(res > 0)
    {
        AnsiSetColor(ANSI_ATTR_OFF,ANSI_BLACK,ANSI_RED);
        printf("FAIL: initDriveQueue() error code %d\n", res);

        AnsiSetColor(ANSI_ATTR_OFF,ANSI_BLACK,ANSI_WHITE);
 //       return 1;
    }
    else
    {
        AnsiSetColor(ANSI_ATTR_OFF,ANSI_BLACK,ANSI_GREEN);
        printf("OK\n");
    }

    //Setup motordrivers
    AnsiSetColor(ANSI_ATTR_OFF,ANSI_BLACK,ANSI_WHITE);
    printf("Init motordrivers...");
    res = DriveInit();
    if(res > 0)
    {
        AnsiSetColor(ANSI_ATTR_OFF,ANSI_BLACK,ANSI_RED);
        printf("FAIL: DriveInit() error code %d\n", res);

        AnsiSetColor(ANSI_ATTR_OFF,ANSI_BLACK,ANSI_WHITE);
  //      return 2;
    }
    else
    {
        AnsiSetColor(ANSI_ATTR_OFF,ANSI_BLACK,ANSI_GREEN);
        printf("OK\n");
    }

    //Intialise restinterface
    AnsiSetColor(ANSI_ATTR_OFF,ANSI_BLACK,ANSI_WHITE);
    printf("Init REST-controller...");
    res = initRestInterface();
    if(res > 1)
    {
        AnsiSetColor(ANSI_ATTR_OFF,ANSI_BLACK,ANSI_RED);
        printf("FAIL: initRestInterface() error code %d\n", res);

        AnsiSetColor(ANSI_ATTR_OFF,ANSI_BLACK,ANSI_WHITE);
  //      return 3;
    }
    else
    {
        AnsiSetColor(ANSI_ATTR_OFF,ANSI_BLACK,ANSI_GREEN);
        printf("OK\n");
    }

    //Initialise camera
    AnsiSetColor(ANSI_ATTR_OFF,ANSI_BLACK,ANSI_WHITE);
    printf("Init camera...");
    res = initCamera();
    if(initCamera() > 1)
    {
        AnsiSetColor(ANSI_ATTR_OFF,ANSI_BLACK,ANSI_RED);
        printf("FAIL: initCamera() error code %d\n", res);

        AnsiSetColor(ANSI_ATTR_OFF,ANSI_BLACK,ANSI_WHITE);
  //      return 4;
    }
    else
    {
        AnsiSetColor(ANSI_ATTR_OFF,ANSI_BLACK,ANSI_GREEN);
        printf("OK\n");
    }

    //Initialise map
    AnsiSetColor(ANSI_ATTR_OFF,ANSI_BLACK,ANSI_WHITE);
    printf("Read mapfile...");
    res = parseMapFile("MAP.dmap");
    if(res > 0)
    {
        AnsiSetColor(ANSI_ATTR_OFF,ANSI_BLACK,ANSI_YELLOW);
        printf("WARNING: could not load map file. parseMapFile() error code %d\n", res);
    }
    else
    {
        AnsiSetColor(ANSI_ATTR_OFF,ANSI_BLACK,ANSI_GREEN);
        printf("OK\n");
    }

    //Initialise configuration
    //Load default configuration
    AnsiSetColor(ANSI_ATTR_OFF,ANSI_BLACK,ANSI_WHITE);
    printf("Load configuration...");
    res = initConfiguration();
    if(res > 0)
    {
        AnsiSetColor(ANSI_ATTR_OFF,ANSI_BLACK,ANSI_RED);
        printf("FAIL: initConfiguration() error code %d\n", res);

        AnsiSetColor(ANSI_ATTR_OFF,ANSI_BLACK,ANSI_WHITE);
        return 5;
    }

    //Read configuration if available
    res = readConfigFile("sc-conf");
    if(res > 1)
    {
        AnsiSetColor(ANSI_ATTR_OFF,ANSI_BLACK,ANSI_RED);
        printf("FAIL: loadConfigFile() error code %d\n", res);

        AnsiSetColor(ANSI_ATTR_OFF,ANSI_BLACK,ANSI_WHITE);
        return 6;
    }
    else if(res == 1)
    {
        AnsiSetColor(ANSI_ATTR_OFF,ANSI_BLACK,ANSI_YELLOW);
        printf("WARNING: no configuration file found. loadConfigFile() error code %d\n", res);
    }
    else
    {
        AnsiSetColor(ANSI_ATTR_OFF,ANSI_BLACK,ANSI_GREEN);
        printf("OK\n");
    }

    AnsiSetColor(ANSI_ATTR_OFF,ANSI_BLACK,ANSI_WHITE);
    return 0;
}

int SmartCore::run()
{
    running = true;

    while(!abort)
    {
        abort = true;
    }

    running = false;
    abort = false;

    return 0;
}

void SmartCore::stop()
{
    if(running)
    {
        abort = true;
    }
}

bool SmartCore::isRunning()
{
    return running;
}
