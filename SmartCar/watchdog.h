#ifndef WATCHDOG_H_
#define WATCHDOG_H_

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <pthread.h>
#include "TimeService.h"

#define watchdogPackageNeededMS 250

int startWatchdog(void);
int resetWatchdog(void);
int stopWatchdog(void);

/**
 * @brief callback type for the command WatchdogTimeOutCallback
 */
typedef void *WatchdogTimeOutCallback_t(void);

/**
 * @brief callback setter for the command WatchdogTimeOut
 * @param callback new callback for the command WatchdogTimeOut
 */
void setWatchdogTimeOutCallback(WatchdogTimeOutCallback_t callback);

int watchdogRunning(void);

void* watchingThread(void* args);

#endif
