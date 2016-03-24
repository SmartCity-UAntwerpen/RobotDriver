#ifndef MSGQUEUE_H
#define MSGQUEUE_H

#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#ifdef __cplusplus	//Check if the compiler is C++
	extern "C"	//Code needs to be handled as C-style code
	{
#endif

typedef void (*processMessageFunction)(void*);

struct msg_t {
                int type;
                int id;
                int numOfParm;
                int* values;
                struct msg_t *Next;
             };

enum msg_type {
                DRIVE_MSG,
                MSG_TYPE_TOTAL
              };

enum msg_id_drive {
                    ABORT,
                    DRIVE_STRAIGHT,
                    DRIVE_D_STRAIGHT,
                    DRIVE_RIGHT,
                    DRIVE_LEFT,
                    MSG_ID_DRIVE_TOTAL
                  };

struct msgqueue_t {
                    pthread_t queueThread;
                    pthread_mutex_t queueLock;
                    struct msg_t *queuePointer;
                    bool queueActiveFlag;
                    bool queueThreadRunning;
                    processMessageFunction processMessage;
                  };

int startQueue(struct msgqueue_t* msgqueue);
int stopQueue(struct msgqueue_t* msgqueue);
int flushQueue(struct msgqueue_t* msgqueue);
int addMsg(struct msgqueue_t* msgqueue, struct msg_t* message);

#ifdef __cplusplus		//Check if the compiler is C++
	}		//End the extern "C" bracket
#endif

#endif
