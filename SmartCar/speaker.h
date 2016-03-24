#ifndef SPEAKER_H
#define SPEAKER_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#ifdef __cplusplus	//Check if the compiler is C++
	extern "C"	//Code needs to be handled as C-style code
	{
#endif

/**
 * \brief Use the computer voice to say a message trough the build-in speaker.
 * \param message :Pointer to the message string to be said
*/
void espeak(char const* message);

void setSpeakerMute(bool muted);

bool getSpeakerMute(void);

#ifdef __cplusplus		//Check if the compiler is C++
	}		//End the extern "C" bracket
#endif

#endif
