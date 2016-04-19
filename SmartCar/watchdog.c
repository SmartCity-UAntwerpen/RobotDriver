#include "watchdog.h"

static int _watchdogActive = 0;
static long watchTime;
pthread_t watchdogThread;
pthread_mutex_t watchdogTimerMutex;
WatchdogTimeOutCallback_t* watchdogTimeOutCallback;

int startWatchdog(void)
{
	if(!_watchdogActive)
	{
		_watchdogActive = 1;

		if(pthread_create(&watchdogThread, NULL, watchingThread, NULL) < 0)
		{
			_watchdogActive = 0;
			printf("Error while creating watching thread!\n");

			return 1;
		}
	}
	else
	{
		printf("Watchdog is already running or is not initialized!\n");

		return 1;
	}

	return 0;
}

int resetWatchdog(void)
{
	//Set new time-out time
	pthread_mutex_lock(&watchdogTimerMutex);
	watchTime = (GetTimeSec() * 1000.0 + GetTimeUs() / 1000.0) + watchdogPackageNeededMS;
	pthread_mutex_unlock(&watchdogTimerMutex);

	return 0;
}

int stopWatchdog(void)
{
	if(_watchdogActive)
	{
		_watchdogActive = 0;

		//Wait till thread is finished
		pthread_join(watchdogThread, NULL);

		return 0;
	}

	return 1;
}

void setWatchdogTimeOutCallback(WatchdogTimeOutCallback_t callback)
{
	watchdogTimeOutCallback = callback;
}

int watchdogRunning(void)
{
	return _watchdogActive;
}

void* watchingThread(void* args)
{
	long now;

	pthread_mutex_lock(&watchdogTimerMutex);
	watchTime = (GetTimeSec() * 1000.0 + GetTimeUs() / 1000.0) + watchdogPackageNeededMS;
	pthread_mutex_unlock(&watchdogTimerMutex);

	while(_watchdogActive)
	{
		now = (GetTimeSec() * 1000.0 + GetTimeUs() / 1000.0);		//Get system time

		if(watchTime < now)
		{
			watchdogTimeOutCallback();
		}
	}

	return NULL;
}
