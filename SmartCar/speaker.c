#include "speaker.h"

void espeak(char* message)
{
    char command[320];
    char lim_message[256];

    if(!_speakerMute)
    {
        //Message limited to 255 characters
        strncpy(lim_message, message, 256);
        lim_message[256] = '\0';

        //Create espeak command
        sprintf(command, "espeak -ven+f2 -k5 -a50 -s150 \"%s\" --stdout | aplay 2>/dev/null", lim_message);

        system(command);
    }
}

void setSpeakerMute(bool muted)
{
    _speakerMute = muted;
}

bool getSpeakerMute(void)
{
    return _speakerMute;
}
