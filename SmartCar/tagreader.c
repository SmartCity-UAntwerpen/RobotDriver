#include "tagreader.h"

static FILE* scriptor;

int TagReaderGetUID(char *data)
{
    char buffer[128];

    if(data == NULL)
    {
        printf("Error, data is NULL.\n");
        return 3;   //Other error
    }

    #ifdef DEBUG_TAGREADER
    printf("Start RF-tag UID scan...\n");
    #endif

    //Pipe the read tag info command to scriptor, drop the error stream to null device
    scriptor = popen("echo \"FF CA 00 00 00\" | scriptor 2>&1", "r");
    if(!scriptor)
    {
        printf("Error, cannot open scriptor\n");
        return 3;   //Other error
    }

    char* token;
    bool found = false;

    while(fgets(buffer, 128, scriptor) != NULL && !found)
    {
        //Check for "Can't get readers list"
        if(strstr(buffer, "Can't get readers list") != NULL)
        {
            //Reader is not connected
            printf("Error, cannot contact RF-tag tag reader\n");

            pclose(scriptor);

            return 1;   //Tag reader error
        }
        else
        {
            //Get the UID number
            token = strrchr(buffer, '<');

            if(token != NULL)
            {
                strncpy(data, token + 2, 20);   //Copy UID
                data[20] = '\0';    //String terminator
                found = true;
            }
        }
    }

    pclose(scriptor);

    if(found)
    {
        return 0;   //Tag OK
    }
    else
    {
        return 2;   //No tag detected
    }
}

int TagReaderWriteData(int blockNumber, int dataLength, char* data)
{
    char buffer[128];
    char command[40];

    if(data == NULL)
    {
        printf("Error, data is NULL.\n");
        return 3;
    }

    #ifdef DEBUG_TAGREADER
    printf("Write data to RF-tag...\n");
    #endif

    //Create scriptor command
    //Byte 4: Block number, Byte 5: Data length
    sprintf(command, "echo \"FF D6 00 %02X %02X %s\" | scriptor 2>&1", blockNumber, dataLength, data);

    //Pipe the read tag info command to scriptor, drop the error stream to null device
    scriptor = popen(command, "r");
    if(!scriptor)
    {
        printf("Error, cannot open scriptor\n");
        return 3;   //Other error
    }

    bool found = false;

    while(fgets(buffer, 128, scriptor) != NULL && !found)
    {
        //Check for "Can't get readers list"
        if(strstr(buffer, "Can't get readers list") != NULL)
        {
            //Reader is not connected
            printf("Error, cannot contact RF-tag tag reader\n");

            pclose(scriptor);

            return 1;   //Tag reader error
        }
        else
        {
            //Check for "Normal processing"
            if(strstr(buffer, "Normal processing."))
            {
                found = true;
            }
        }
    }

    pclose(scriptor);

    if(found)
    {
        return 0;   //Tag OK
    }
    else
    {
        return 2;   //No tag detected
    }
}

int TagReaderReadData(int blockNumber, int dataLength, char* data)
{
    char buffer[128];
    char command[40];

    if(data == NULL)
    {
        printf("Error, data is NULL.\n");
        return 3;   //Other error
    }

    #ifdef DEBUG_TAGREADER
    printf("Start RF-tag data scan...\n");
    #endif

    //Create scriptor command
    //Byte 4: Block number, Byte 5: Data length
    sprintf(command, "echo \"FF B0 00 %02X %02X\" | scriptor 2>&1", blockNumber, dataLength);

    //Pipe the read tag info command to scriptor, drop the error stream to null device
    scriptor = popen(command, "r");
    if(!scriptor)
    {
        printf("Error, cannot open scriptor\n");
        return 3;   //Other error
    }

    char* token;
    bool found = false;

    while(fgets(buffer, 128, scriptor) != NULL && !found)
    {
        //Check for "Can't get readers list"
        if(strstr(buffer, "Can't get readers list") != NULL)
        {
            //Reader is not connected
            printf("Error, cannot contact RF-tag tag reader\n");

            pclose(scriptor);

            return 1;   //Tag reader error
        }
        else
        {
            //Get the data
            token = strrchr(buffer, '<');

            if(token != NULL)
            {
                strncpy(data, token + 2, (dataLength*3) - 1);   //Copy UID
                data[(dataLength*3) - 1] = '\0';    //String terminator
                found = true;
            }
        }
    }

    pclose(scriptor);

    if(found)
    {
        return 0;   //Tag OK
    }
    else
    {
        return 2;   //No tag detected
    }
}
