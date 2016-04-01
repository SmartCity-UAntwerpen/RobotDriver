#ifndef RESTINTERFACE_H
#define RESTINTERFACE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <curl/curl.h>

#ifdef __cplusplus	//Check if the compiler is C++
	extern "C"	//Code needs to be handled as C-style code
	{
#endif

int initRestInterface(void);

int stopRestInterface(void);

int httpGet(char const* url, char* message, char* errMessage);

int httpPost(char const* url, char const* data, char const* header, char* errMessage);

#ifdef __cplusplus		//Check if the compiler is C++
	}		//End the extern "C" bracket
#endif

#endif
