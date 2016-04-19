#include "smartcore.h"

using namespace SC;

//Single instance of class
SmartCore* SmartCore::smartCore_instance = NULL;

//Default constructor
SmartCore::SmartCore()
{
    this->running = false;
    this->abort = false;
}

SmartCore::~SmartCore()
{
    stopProcesses();

    //Stop REST Interface
    stopRestInterface();

    //Release server socket
    releaseSocket(&TCPSocket);

    //Stop camera
    closeCamera();

    //Destroy configuration
    deinitConfiguration();
}

//Get singleton instance
SmartCore* SmartCore::getInstance()
{
    if(!smartCore_instance)
    {
        smartCore_instance = new SmartCore();
    }

    return smartCore_instance;
}

int SmartCore::initialiseCore(int argc, char *argv[])
{
    int res;

    AnsiSetColor(ANSI_ATTR_OFF,ANSI_BLACK,ANSI_GREEN);
    printf("Initialising SmartCore...\n");

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
        return 1;
    }

    //Read configuration if available
    res = readConfigFile("sc-conf");
    if(res > 1)
    {
        AnsiSetColor(ANSI_ATTR_OFF,ANSI_BLACK,ANSI_RED);
        printf("FAIL: loadConfigFile() error code %d\n", res);

        AnsiSetColor(ANSI_ATTR_OFF,ANSI_BLACK,ANSI_WHITE);
        return 2;
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

    //Initialise drivequeue
    AnsiSetColor(ANSI_ATTR_OFF,ANSI_BLACK,ANSI_WHITE);
    printf("Init drivequeue...");
    res = initDriveQueue();
    if(res > 0)
    {
        AnsiSetColor(ANSI_ATTR_OFF,ANSI_BLACK,ANSI_RED);
        printf("FAIL: initDriveQueue() error code %d\n", res);

        AnsiSetColor(ANSI_ATTR_OFF,ANSI_BLACK,ANSI_WHITE);
        return 3;
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
  //      return 4;
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
        return 5;
    }
    else
    {
        AnsiSetColor(ANSI_ATTR_OFF,ANSI_BLACK,ANSI_GREEN);
        printf("OK\n");
    }

    //Initialise serversocket
    AnsiSetColor(ANSI_ATTR_OFF,ANSI_BLACK,ANSI_WHITE);
    printf("Init serversocket...");
    res = initialiseSocket(&TCPSocket, atoi(getConfigValue(CONFIG_LISTENINGPORT)), SOCKET_TCP);
    if(res > 1)
    {
        AnsiSetColor(ANSI_ATTR_OFF,ANSI_BLACK,ANSI_RED);
        printf("FAIL: initialiseSocket() error code %d\n", res);

        AnsiSetColor(ANSI_ATTR_OFF,ANSI_BLACK,ANSI_WHITE);
        return 6;
    }
    else
    {
        AnsiSetColor(ANSI_ATTR_OFF,ANSI_BLACK,ANSI_GREEN);
        printf("OK\n");
    }

    //Set message received callback
    setPacketReceivedCallback(&TCPSocket, receivedCommand);

    //Initialise camera
    AnsiSetColor(ANSI_ATTR_OFF,ANSI_BLACK,ANSI_WHITE);
    printf("Init camera...");
    res = initCamera();
    if(initCamera() > 1)
    {
        AnsiSetColor(ANSI_ATTR_OFF,ANSI_BLACK,ANSI_RED);
        printf("FAIL: initCamera() error code %d\n", res);

        AnsiSetColor(ANSI_ATTR_OFF,ANSI_BLACK,ANSI_WHITE);
  //      return 7;
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

    AnsiSetColor(ANSI_ATTR_OFF,ANSI_BLACK,ANSI_WHITE);
    return 0;
}

int SmartCore::run()
{
    int status = 0;

    this->running = true;

    status = startProcesses();

    if(status > 0)
    {
        printf("Core failed to start system! Error code: %d\n", status);

        //System failed to start processes
        this->running = false;
        this->abort = false;

        return status;
    }

    while(!this->abort)
    {
        //Program loop
        _delay_ms(200);
    }

    status = stopProcesses();

    this->running = false;
    this->abort = false;

    return -status;
}

void SmartCore::stop()
{
    if(this->running)
    {
        this->abort = true;
    }
}

int SmartCore::startProcesses()
{
    if(startQueue(getDriveQueue()) > 0)
    {
        //Could not start drive queue
        return 1;
    }

    //Start watchdog
    //startWatchdog();

    //Start server socket
    startListening(&TCPSocket);

    return 0;
}

int SmartCore::stopProcesses()
{
    //Stop drive queue
    if(getDriveQueue() != NULL)
    {
        stopQueue(getDriveQueue());
        flushQueue(getDriveQueue());

        //Free memory
        free(getDriveQueue());
    }

    //Stop watchdog
    stopWatchdog();

    //Stop driving
    AbortDriving();

    //Stop server socket
    stopListening(&TCPSocket);

    return 0;
}

bool SmartCore::isRunning()
{
    return this->running;
}



void* SmartCore::processCommand(char* command)
{
    if(strcmp(command, "SCAR: DRIVE ABORT") == 0)
    {
        flushQueue(getDriveQueue());
        AbortDriving();
    }
    else if(strcmp(command, "SCAR: DRIVE FOLLOWLINE") == 0)
    {

    }
    else if(strcmp(command, "SCAR: DRIVE FORWARD") == 0)
    {

    }
    else if(strcmp(command, "SCAR: DRIVE TURN") == 0)
    {

    }
    else if(strcmp(command, "SCAR: EXIT") == 0)
    {
        this->stop();
    }
    else
    {
        printf("Received unknown command: %s\n", command);
    }

    return NULL;
}

//Wrapper function
void* receivedCommand(char* command)
{
    SmartCore* core = SmartCore::getInstance();

    return core->processCommand(command);
}
