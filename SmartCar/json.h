#ifndef JSON_H
#define JSON_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#ifdef __cplusplus	//Check if the compiler is C++
	extern "C"	//Code needs to be handled as C-style code
	{
#endif

typedef struct jsonMember_t
{
    char* name;
    bool stringValue;
    char* value;
    struct jsonMember_t* object;
    bool objectValue;
    struct jsonMember_t* next;
} jsonMember_t;

int addJSONMemberStringValue(jsonMember_t** object, char const* name, char const* value, bool stringValue);

int addJSONMemberObjectValue(jsonMember_t** object, char const* name, jsonMember_t* value);

int destroyJSONObject(jsonMember_t** object);

int parseJSONString(jsonMember_t* object, char* jsonString, int size);

int _parseJSONStringRecursiveHelper(jsonMember_t* object, char* jsonString, int size, int* charCounter);

int parseJSONObject(char* jsonString, jsonMember_t** object);

int _parseJSONObjectRecursiveHelper(char* jsonString, jsonMember_t** object, int* charCounter);

#ifdef __cplusplus		//Check if the compiler is C++
	}		//End the extern "C" bracket
#endif

#endif
