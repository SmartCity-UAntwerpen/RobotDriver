#ifndef MSGQUEUE_H
#define MSGQUEUE_H

#include <pthread.h>
#include <time.h>
#include "drive.h"

struct msg {
                int id;
                int numOfParm;
                int* values;
                struct msg *Next;
            };

enum msg_id {
    ABORT,
    DRIVE_STRAIGHT,
    DRIVE_RIGHT,
    DRIVE_LEFT
};

pthread_t dispatchThread;

int runQueueDispatcher(void);
int stopQueueDispatcher(void);
bool msgQueueActive(void);
void* _processMessage(void* args);
int _startNextActivity(struct msg* message);
int flushQueue(void);
int addMsg(struct msg* message);

#endif
