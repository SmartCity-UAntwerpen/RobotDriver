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

    //Intialize RestInterface
    if(initRestInterface() > 1)
    {
        printf("Failed to initialize REST interface!\n");
    }

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

    printf("CREATING JSON OBJECT\n");

    struct jsonMember_t* jsonObject = NULL;
    struct jsonMember_t* jsonSubObject = NULL;
    struct jsonMember_t* jsonArrayObject = NULL;
    struct jsonMember_t* jsonArrayObject2 = NULL;

    if(addJSONMemberStringValue(&jsonObject, "key", "vmpool", true) > 0)
    {
        printf("Error while adding member to JSON object\n");
    }
    else
    {
        printf("Json object success\n");
    }

    if(addJSONMemberStringValue(&jsonSubObject, "value", "hello", true) > 0)
    {
        printf("Error while adding member to JSON object\n");
    }
    else
    {
        printf("Json subobject success\n");
    }

    if(addJSONMemberStringValue(&jsonArrayObject, "ArrayMember", "1", false) > 0)
    {
        printf("Error while adding member to JSON object\n");
    }
    else
    {
        printf("Json subobject success\n");
    }

    if(addJSONMemberStringValue(&jsonArrayObject2, "ArrayMember", "2", false) > 0)
    {
        printf("Error while adding member to JSON object\n");
    }
    else
    {
        printf("Json subobject success\n");
    }

    jsonArrayObject->next = jsonArrayObject2;
    jsonSubObject->next = jsonArrayObject;

    if(addJSONMemberObjectValue(&jsonObject, "Subobject", jsonSubObject) > 0)
    {
        printf("Error while adding member to JSON object\n");
    }
    else
    {
        printf("Adding subobject success\n");
    }

    if(addJSONMemberStringValue(&jsonObject, "INTEGER", "500", false) > 0)
    {
        printf("Error while adding Integer member to JSON object\n");
    }
    else
    {
        printf("Adding Integer member success\n");
    }

    if(addJSONMemberStringValue(&jsonObject, "ARRAY", "[500,\"test\",3]", false) > 0)
    {
        printf("Error while adding Array member to JSON object\n");
    }
    else
    {
        printf("Adding Array member success\n");
    }

    char jsonString[450];

    if(parseJSONString(jsonObject, jsonString, 450) > 0)
    {
        printf("Error while parsing JSON object\n");
    }
    else
    {
        printf("JSON STRING: %s\n", jsonString);
    }

    if(parseJSONObject(jsonString, &jsonObject) > 0)
    {
        printf("Failed to parse object from string\n");
    }
    else
    {
        printf("Parsing object from String success\n");
    }

    printf("%s\n", jsonObject->name);
    printf("%s\n\n", jsonObject->value);

    printf("%s\n", jsonObject->next->name);
    printf("%s\n", jsonObject->next->object->name);
    printf("%s\n", jsonObject->next->object->value);
    printf("%s\n", jsonObject->next->object->next->name);
    printf("%s\n", jsonObject->next->object->next->value);
    printf("%s\n", jsonObject->next->object->next->next->name);
    printf("%s\n\n", jsonObject->next->object->next->next->value);

    printf("%s\n", jsonObject->next->next->name);
    printf("%s\n\n", jsonObject->next->next->value);

    printf("%s\n", jsonObject->next->next->next->name);
    printf("%s\n\n", jsonObject->next->next->next->value);

    if(parseJSONString(jsonObject, jsonString, 130) > 0)
    {
        printf("Error while parsing JSON object\n");
    }
    else
    {
        printf("JSON STRING: %s\n", jsonString);
    }

    destroyJSONObject(&jsonObject);
/*
    char message[1024];
    char errMessage[500];

    if(httpGet("http://192.168.1.10:1304/systemLoad", message, errMessage) > 0)
    {
        printf("%s\n", errMessage);
    }

    printf("%s\n", message);

    if(httpPost("http://192.168.1.10:1304/systemConfig", "{\"key\":\"vmpool\",\"value\":\"hello\"}", "Content-Type: application/json", errMessage) > 0)
    {
        printf("%s\n", errMessage);
    }
    else
    {
        printf("%s\n", errMessage);
    }

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
