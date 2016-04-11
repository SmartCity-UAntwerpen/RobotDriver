#include "config.h"

static config_t configuration[CONFIG_MAX];

int initConfiguration(void)
{
    //Initialise default configuration
    //Config Carname
    configuration[CONFIG_CARNAME].key = (char*) malloc (strlen("carname") + 1);
    configuration[CONFIG_CARNAME].value = (char*) malloc (strlen("SimCar") + 1);

    if(configuration[CONFIG_CARNAME].key == NULL || configuration[CONFIG_CARNAME].value == NULL)
    {
        printf("Could not allocate memory for configuration!\n");

        return 1;
    }

    strcpy(configuration[CONFIG_CARNAME].key, "carname");
    strcpy(configuration[CONFIG_CARNAME].value, "SimCar");

    //Config Server URL
    configuration[CONFIG_SERVERURL].key = (char*) malloc (strlen("serverurl") + 1);
    configuration[CONFIG_SERVERURL].value = (char*) malloc (strlen("") + 1);

    if(configuration[CONFIG_SERVERURL].key == NULL || configuration[CONFIG_SERVERURL].value == NULL)
    {
        printf("Could not allocate memory for configuration!\n");

        return 1;
    }

    strcpy(configuration[CONFIG_SERVERURL].key, "serverurl");
    strcpy(configuration[CONFIG_SERVERURL].value, "");

    return 0;
}

int deinitConfiguration(void)
{
    int i;

    for(i = 0; i < CONFIG_MAX; i++)
    {
        if(configuration[i].key != NULL)
        {
            free(configuration[i].key);
        }

        if(configuration[i].value != NULL)
        {
            free(configuration[i].value);
        }
    }

    return 0;
}

int setConfigValue(char* key, char* value)
{
    int i = 0;
    bool found = false;

    while(!found && i < CONFIG_MAX)
    {
        if(strcmp(configuration[i].key, key) == 0)
        {
            found = true;
        }
        else
        {
            i++;
        }
    }

    if(!found)
    {
        return 2;
    }

    return setConfigValueWithKey((ConfigKey) i, value);
}

int setConfigValueWithKey(ConfigKey key, char* value)
{
    if(key >= CONFIG_MAX)
    {
        //Invalid configuration key
        return 2;
    }

    //Deallocate previous value
    if(configuration[key].value != NULL)
    {
        free(configuration[key].value);
    }

    configuration[key].value = (char*) malloc (strlen(value) + 1);

    if(configuration[key].value == NULL)
    {
        printf("Could not allocate memory for setting configuration value of key: %d!\n", key);

        return 1;
    }

    memcpy(configuration[key].value, value, strlen(value));
    configuration[key].value[strlen(value)] = '\0';

    return 0;
}

char* getConfigKeyName(ConfigKey key)
{
    if(key >= CONFIG_MAX)
    {
        //Invalid configuration key
        return NULL;
    }

    return configuration[key].key;
}

char* getConfigValue(ConfigKey key)
{
    if(key >= CONFIG_MAX)
    {
        //Invalid configuration key
        return NULL;
    }

    return configuration[key].value;
}
