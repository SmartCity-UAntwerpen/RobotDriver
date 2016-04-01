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

typedef struct jsonAttribute_t
{
    char property[65];
    bool stringValue;
    char value[129];
    struct jsonAttribute_t* next;
} jsonAttribute_t;

int addJSONAttribute(jsonAttribute_t** object, char const* property, char const* value, bool stringValue);

int destroyJSONObject(jsonAttribute_t* object);

int parseJSONString(jsonAttribute_t* object, char* jsonString, int size);

int parseJSONObject(char* jsonString, jsonAttribute_t** object);

#ifdef __cplusplus		//Check if the compiler is C++
	}		//End the extern "C" bracket
#endif

#endif
