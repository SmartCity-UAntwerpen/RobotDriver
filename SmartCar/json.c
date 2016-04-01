#include "json.h"

int addJSONAttribute(jsonAttribute_t** object, char const* property, char const* value, bool stringValue)
{
    jsonAttribute_t* newAttribute;
    jsonAttribute_t* lastAttribute;

    newAttribute = (jsonAttribute_t*) malloc (sizeof(jsonAttribute_t));

    if(newAttribute == NULL)
    {
        printf("System out of memory! Could not allocate memory to add a attribute to the object!\n");

        return 1;
    }

    //Set property
    memcpy(newAttribute->property, property, 64);
    newAttribute->property[65] = '\0';

    //Set value
    memcpy(newAttribute->value, value, 128);
    newAttribute->value[129] = '\0';

    //Set string flag
    newAttribute->stringValue = stringValue;

    //Set next
    newAttribute->next = NULL;

    if(*object != NULL)
    {
        lastAttribute = *object;

        //Get last attribute of object
        while(lastAttribute->next != NULL)
        {
            lastAttribute = lastAttribute->next;
        }

        //Add new attribute to object
        lastAttribute->next = newAttribute;
    }
    else
    {
        *object = newAttribute;
    }

    return 0;
}

int destroyJSONObject(jsonAttribute_t* object)
{
    jsonAttribute_t* attributePointer;

    if(object == NULL)
    {
        //No object
        return 1;
    }

    while((jsonAttribute_t*) object != NULL)
    {
        //Set temporary pointer to first element
        attributePointer = (jsonAttribute_t*) object;

        //Switch front of attribute list to next element
        object = object->next;

        //Free allocated memory of attributePointer
        free((jsonAttribute_t*) attributePointer);
    }

    return 0;
}

int parseJSONString(jsonAttribute_t* object, char* jsonString, int size)
{
    int charCounter = 0;
    int paramLength = 0;
    int overhead = 0;
    jsonAttribute_t* attribute;

    if(object == NULL)
    {
        //No object to parse
        return 1;
    }

    if(size < 2)
    {
        //To small for initial string
        return 2;
    }

    //Open JSON string
    jsonString[charCounter] = '{';
    charCounter++;

    //Add attributes
    attribute = object;

    while(attribute != NULL)
    {
        //Add attribute name to JSON string
        paramLength = strlen(attribute->property);
        overhead = 3;

        //Check if attribute name + five character overhead can be added to string
        if(size < charCounter + paramLength + overhead)
        {
            //String is to small
            return 2;
        }

        jsonString[charCounter] = '\"';
        charCounter++;

        memcpy(jsonString + charCounter, attribute->property, paramLength);
        charCounter = charCounter + paramLength;

        memcpy(jsonString + charCounter, "\":", 2);
        charCounter = charCounter + 2;

        //Add attribute value to JSON string
        paramLength = strlen(attribute->value);

        //Set overhead depending on String flag
        if(attribute->stringValue)
        {
            overhead = 2;
        }
        else
        {
            overhead = 0;
        }

        //Check if attribute value + overhead can be added to string
        if(size < charCounter + paramLength + overhead)
        {
            //String is to small
            return 2;
        }

        if(attribute->stringValue)
        {
            //Append string quote
            jsonString[charCounter] = '\"';
            charCounter++;
        }

        memcpy(jsonString + charCounter, attribute->value, paramLength);
        charCounter = charCounter + paramLength;

        if(attribute->stringValue)
        {
            //Append string quote
            jsonString[charCounter] = '\"';
            charCounter++;
        }

        //Check for other attributes
        if(attribute->next != NULL)
        {
            //Append comma character for next attribute
            jsonString[charCounter] = ',';
            charCounter++;
        }

        attribute = attribute->next;
    }

    if(size < charCounter + 2)
    {
        //String is to small to hold terminator characters
        return 2;
    }

    jsonString[charCounter] = '}';
    jsonString[charCounter + 1] = '\0';

    return 0;
}

int parseJSONObject(char* jsonString, jsonAttribute_t** object)
{

    return 0;
}
