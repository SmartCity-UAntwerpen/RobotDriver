#include "speaker.h"

static bool _speakerMute = false;
static pid_t speakerChild;

void espeak(char const* message)
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

        _playSpeakerCommand(command);
    }
}

void playWav(char const* file)
{
    char command[158];
    char lim_fileName[128];

    if(!_speakerMute)
    {
        //Filename limited to 128 characters
        strncpy(lim_fileName, file, 128);
        lim_fileName[128] = '\0';

        //Create audio command
        sprintf(command, "aplay \"audio/%s.wav\" 2>/dev/null", lim_fileName);

        _playSpeakerCommand(command);
    }
}

int stopSpeaker(void)
{
    if(speakerPlaying())
    {
        kill(speakerChild, SIGKILL);

        //Kill all aplay instances (SIGPIPE)
        system("killall -13 aplay 2>/dev/null");

        while(speakerPlaying())
        {
            //Wait for child to exit
        }

        return 0;
    }
    else
    {
        //Speaker not playing
        return 1;
    }
}

int _playSpeakerCommand(char* speakerCommand)
{
    char* command;

    if(speakerPlaying())
    {
        stopSpeaker();
    }

    command = (char*) malloc (strlen(speakerCommand) + 1);
    strncpy(command, speakerCommand, strlen(speakerCommand));
    command[strlen(speakerCommand)] = '\0';

    if(_runSystemCommand(command))
    {
        if(command != NULL)
        {
            free(command);
        }

        printf("Error while creating speaker process!\n");

        return 1;
    }

    return 0;
}

int _runSystemCommand(char* command)
{
    if(command == NULL)
    {
        //No command given
        return 2;
    }

    speakerChild = fork();

    if(speakerChild == 0)
    {
        //Child process

        //Run system command
        system(command);

        free(command);

        //Terminate Child process
        _exit(0);
    }
    else if(speakerChild == -1)
    {
        //Could not create child process
        return 1;
    }

    free(command);

    return 0;
}

void setSpeakerMute(bool muted)
{
    _speakerMute = muted;
}

bool getSpeakerMute(void)
{
    return _speakerMute;
}

bool speakerPlaying(void)
{
    int status;

    pid_t result = waitpid(speakerChild, &status, WNOHANG);

    return result == 0 ? 1 : 0;
}
