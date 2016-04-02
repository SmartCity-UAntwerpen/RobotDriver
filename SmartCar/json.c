#include "json.h"

int addJSONMemberStringValue(jsonMember_t** object, char const* name, char const* value, bool stringValue)
{
    jsonMember_t* newMember;
    jsonMember_t* lastMember;

    newMember = (jsonMember_t*) malloc (sizeof(jsonMember_t));

    if(newMember == NULL)
    {
        printf("System out of memory! Could not allocate memory to add a member to the object!\n");

        return 1;
    }

    //Set name
    newMember->name = (char*) malloc (strlen(name));

    if(newMember->name == NULL)
    {
        printf("System out of memory! Could not allocate memory to set the name of a member!\n");

        return 1;
    }

    memcpy(newMember->name, name, strlen(name));
    newMember->name[strlen(name)] = '\0';

    //Set value
    newMember->value = (char*) malloc (strlen(value));

    if(newMember->value == NULL)
    {
        printf("System out of memory! Could not allocate memory to set the value of a member!\n");

        return 1;
    }

    memcpy(newMember->value, value, strlen(value));
    newMember->value[strlen(value)] = '\0';

    //Set string flag
    newMember->stringValue = stringValue;

    //Set object
    newMember->object = NULL;

    //Set object flag
    newMember->objectValue = false;

    //Set next
    newMember->next = NULL;

    if(*object != NULL)
    {
        lastMember = *object;

        //Get last member of object
        while(lastMember->next != NULL)
        {
            lastMember = lastMember->next;
        }

        //Add new member to object
        lastMember->next = newMember;
    }
    else
    {
        *object = newMember;
    }

    return 0;
}

int addJSONMemberObjectValue(jsonMember_t** object, char const* name, jsonMember_t* value)
{
    jsonMember_t* newMember;
    jsonMember_t* lastMember;

    newMember = (jsonMember_t*) malloc (sizeof(jsonMember_t));

    if(newMember == NULL)
    {
        printf("System out of memory! Could not allocate memory to add a member to the object!\n");

        return 1;
    }

    //Set name
    newMember->name = (char*) malloc (strlen(name));

    if(newMember->name == NULL)
    {
        printf("System out of memory! Could not allocate memory to set the name of a member!\n");

        return 1;
    }

    memcpy(newMember->name, name, strlen(name));
    newMember->name[strlen(name)] = '\0';

    //Set value
    newMember->value = NULL;

    //Set string flag
    newMember->stringValue = false;

    //Set object
    newMember->object = value;

    //Set object flag
    newMember->objectValue = true;

    //Set next
    newMember->next = NULL;

    if(*object != NULL)
    {
        lastMember = *object;

        //Get last member of object
        while(lastMember->next != NULL)
        {
            lastMember = lastMember->next;
        }

        //Add new member to object
        lastMember->next = newMember;
    }
    else
    {
        *object = newMember;
    }

    return 0;
}

int destroyJSONObject(jsonMember_t** object)
{
    jsonMember_t* memberPointer;

    if(*object == NULL)
    {
        //No object
        return 1;
    }

    while((jsonMember_t*) *object != NULL)
    {
        //Set temporary pointer to first element
        memberPointer = (jsonMember_t*) *object;

        //Switch front of member list to next element
        *object = (*object)->next;

        if(memberPointer->object != NULL)
        {
            //Free memory of object value
            destroyJSONObject(&memberPointer->object);
        }

        if(memberPointer->name != NULL)
        {
            //Free memory of name
            free((char*) memberPointer->name);
        }

        if(memberPointer->value != NULL)
        {
            //Free memory of string value
            free((char*) memberPointer->value);
        }

        //Free allocated memory of memberPointer
        free((jsonMember_t*) memberPointer);
    }

    *object = NULL;

    return 0;
}

int parseJSONString(jsonMember_t* object, char* jsonString, int size)
{
    int charCounter = 0;

    return _parseJSONStringRecursiveHelper(object, jsonString, size, &charCounter);
}

int _parseJSONStringRecursiveHelper(jsonMember_t* object, char* jsonString, int size, int* charCounter)
{
    int paramLength = 0;
    int overhead = 0;
    int recursiveResult = 0;
    jsonMember_t* member;

    if(object == NULL)
    {
        //No object to parse
        return 1;
    }

    if(size < *charCounter + 2)
    {
        //To small for initial string
        return 2;
    }

    //Open JSON string
    jsonString[*charCounter] = '{';
    (*charCounter)++;

    //Add members
    member = object;

    while(member != NULL)
    {
        //Add member name to JSON string
        paramLength = strlen(member->name);
        overhead = 3;

        //Check if member name + five character overhead can be added to string
        if(size < *charCounter + paramLength + overhead)
        {
            //String is to small
            return 2;
        }

        jsonString[*charCounter] = '\"';
        (*charCounter)++;

        memcpy(jsonString + *charCounter, member->name, paramLength);
        *charCounter = *charCounter + paramLength;

        memcpy(jsonString + *charCounter, "\":", 2);
        *charCounter = *charCounter + 2;

        //Add member value to JSON string
        if(member->objectValue)
        {
            recursiveResult = _parseJSONStringRecursiveHelper(member->object, jsonString, size, charCounter);

            if(recursiveResult > 0)
            {
                //Error in recursive call
                return recursiveResult;
            }

            //Remove terminator '\0' character from recursive call
            (*charCounter)--;
        }
        else
        {
            paramLength = strlen(member->value);

            //Set overhead depending on String flag
            if(member->stringValue)
            {
                overhead = 2;
            }
            else
            {
                overhead = 0;
            }

            //Check if member value + overhead can be added to string
            if(size < *charCounter + paramLength + overhead)
            {
                //String is to small
                return 2;
            }

            if(member->stringValue)
            {
                //Append string quote
                jsonString[*charCounter] = '\"';
                (*charCounter)++;
            }

            memcpy(jsonString + *charCounter, member->value, paramLength);
            *charCounter = *charCounter + paramLength;

            if(member->stringValue)
            {
                //Append string quote
                jsonString[*charCounter] = '\"';
                (*charCounter)++;
            }
        }

        //Check for other members
        if(member->next != NULL)
        {
            //Append comma character for next member
            jsonString[*charCounter] = ',';
            (*charCounter)++;
        }

        member = member->next;
    }

    if(size < *charCounter + 2)
    {
        //String is to small to hold terminator characters
        return 2;
    }

    jsonString[*charCounter] = '}';
    jsonString[*charCounter + 1] = '\0';

    *charCounter = *charCounter + 2;

    return 0;
}

int parseJSONObject(char* jsonString, jsonMember_t** object)
{
    int charCounter = 0;
    int result;

    //Destroy JSON object if existing
    destroyJSONObject(object);

    result = _parseJSONObjectRecursiveHelper(jsonString, object, &charCounter);

    if(result > 0)
    {
    printf("ERROR while parsing object: %d\n", result);
        //Parsing failed, free object
        destroyJSONObject(object);
    }

    return result;
}

int _parseJSONObjectRecursiveHelper(char* jsonString, jsonMember_t** object, int* charCounter)
{
    int subStringCharCounter = 0;
    int recursiveResult = 0;
    bool openQuote = false;
    jsonMember_t* newMember = NULL;
    jsonMember_t* lastMember = NULL;

    //First opening character
    if(jsonString[*charCounter] != '{')
    {
        //Invalid JSON string
        return 2;
    }

    (*charCounter)++;

    while(jsonString[*charCounter] != '}')
    {
        //Allocate memory for member
        newMember = (jsonMember_t*) malloc (sizeof(jsonMember_t));

        if(newMember == NULL)
        {
            printf("System out of memory! Could not allocate memory to add a member to the object!\n");

            return 1;
        }

        //Initialize JSON member
        newMember->name = NULL;
        newMember->value = NULL;
        newMember->stringValue = false;
        newMember->object = NULL;
        newMember->objectValue = false;
        newMember->next = NULL;

        //Get member name
        if(jsonString[*charCounter] != '\"')
        {
            destroyJSONObject(&newMember);

            //Invalid JSON string
            return 2;
        }

        (*charCounter)++;

        //Get member name
        subStringCharCounter = 0;
        while(!(jsonString[*charCounter + subStringCharCounter] == '\"' && jsonString[*charCounter + subStringCharCounter - 1] != '\\') && (*charCounter + subStringCharCounter) < strlen(jsonString))
        {
            subStringCharCounter++;
        }

        //Set member name
        newMember->name = (char*) malloc (subStringCharCounter + 1);

        if(newMember->name == NULL)
        {
            printf("System out of memory! Could not allocate memory to set the name of a member!\n");

            return 1;
        }

        memcpy(newMember->name, jsonString + (*charCounter), subStringCharCounter);
        newMember->name[subStringCharCounter] = '\0';

        (*charCounter) = (*charCounter) + subStringCharCounter;

        if(jsonString[*charCounter] != '\"')
        {
            destroyJSONObject(&newMember);

            //Invalid JSON string
            return 2;
        }

        (*charCounter)++;

        if(jsonString[*charCounter] != ':')
        {
            destroyJSONObject(&newMember);

            //Invalid JSON string
            return 2;
        }

        (*charCounter)++;

        //Detect value type
        if(jsonString[*charCounter] == '{')
        {
            //Object value
            newMember->objectValue = true;

            recursiveResult = _parseJSONObjectRecursiveHelper(jsonString, &newMember->object, charCounter);

            if(recursiveResult > 0)
            {
                //Error in recursive call
                return recursiveResult;
            }

            //Skip closing bracket
            (*charCounter)++;
        }
        else if(jsonString[*charCounter] == '"')
        {
            //String value
            newMember->stringValue = true;

            (*charCounter)++;

            //Get member value
            subStringCharCounter = 0;
            while(!(jsonString[*charCounter + subStringCharCounter] == '\"' && jsonString[*charCounter + subStringCharCounter - 1] != '\\') && (*charCounter + subStringCharCounter) < strlen(jsonString))
            {
                subStringCharCounter++;
            }

            //Set member value
            newMember->value = (char*) malloc (subStringCharCounter + 1);

            if(newMember->value == NULL)
            {
                printf("System out of memory! Could not allocate memory to set the value of a member!\n");

                return 1;
            }

            memcpy(newMember->value, jsonString + (*charCounter), subStringCharCounter);
            newMember->value[subStringCharCounter] = '\0';

            (*charCounter) = (*charCounter) + subStringCharCounter + 1;
        }
        else
        {
            //Non-string value

            if(jsonString[*charCounter] == '[')
            {
                //Array value

                //Get member value
                subStringCharCounter = 0;
                while(!(jsonString[*charCounter + subStringCharCounter] == ']') && !openQuote && (*charCounter + subStringCharCounter) < strlen(jsonString))
                {
                    if(jsonString[*charCounter + subStringCharCounter] == '\"' && jsonString[*charCounter + subStringCharCounter - 1] == '\\')
                    {
                        openQuote = !openQuote;
                    }

                    subStringCharCounter++;
                }

                //Append closing bracket to string
                subStringCharCounter++;
            }
            else
            {
                //Single value

                //Get member value
                subStringCharCounter = 0;
                while(!((jsonString[*charCounter + subStringCharCounter] == ',') || jsonString[*charCounter + subStringCharCounter] == '}') && (*charCounter + subStringCharCounter) < strlen(jsonString))
                {
                    subStringCharCounter++;
                }
            }

            //Set member value
            newMember->value = (char*) malloc (subStringCharCounter + 1);

            if(newMember->value == NULL)
            {
                printf("System out of memory! Could not allocate memory to set the value of a member!\n");

                return 1;
            }

            memcpy(newMember->value, jsonString + (*charCounter), subStringCharCounter);
            newMember->value[subStringCharCounter] = '\0';

            (*charCounter) = (*charCounter) + subStringCharCounter;
        }

        if(jsonString[*charCounter] == ',')
        {
            //Skip comma character between members
            (*charCounter)++;
        }

        if(*object == NULL)
        {
            //First iteration
            *object = newMember;
        }
        else
        {
            lastMember = *object;

            //Get last member of object
            while(lastMember->next != NULL)
            {
                lastMember = lastMember->next;
            }

            //Add new member to object
            lastMember->next = newMember;
        }
    }

    return 0;
}
