#ifndef SPEAKER_H
#define SPEAKER_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

#ifdef __cplusplus	//Check if the compiler is C++
	extern "C"	//Code needs to be handled as C-style code
	{
#endif

/**
 * \brief Use the computer voice to say a message trough the build-in speaker.
 * \param message :Pointer to the message string to be said
*/
void espeak(char const* message);

void playWav(char const* file);

int stopSpeaker(void);

int _playSpeakerCommand(char* speakerCommand);

int _runSystemCommand(char* command);

void setSpeakerMute(bool muted);

bool getSpeakerMute(void);

bool speakerPlaying(void);

#ifdef __cplusplus		//Check if the compiler is C++
	}		//End the extern "C" bracket
#endif

#endif
