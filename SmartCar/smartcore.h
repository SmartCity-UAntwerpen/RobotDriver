#ifndef SMARTCORE_H
#define SMARTCORE_H

#include <stdlib.h>
#include <stdbool.h>
#include "ansi.h"
#include "drive.h"
#include "msgqueue.h"
#include "drivequeue.h"
#include "travel.h"
#include "dijkstra.h"
#include "map.h"
#include "restInterface.h"
#include "json.h"
#include "watchdog.h"
#include "config.h"
#include "configfile.h"
#include "serversocket.h"
#include "robotapp.h"

namespace SC
{
    class SmartCore
    {
        public:
            /*
             * Get singleton instance
             */
            static SmartCore* getInstance();

            /*
             * Destructor
             */
            virtual ~SmartCore(void);

            int initialiseCore(int argc, char *argv[]);

            int run(void);

            void stop(void);

            bool isRunning(void);

            size_t processCommand(char* command, char* response, size_t maxLength);

            size_t processDriveCommand(char* command, char* response, size_t maxLength);

        private:
            static SmartCore* smartCore_instance;
            bool running;
            bool abort;

            /*
             * Default constructor
             */
            SmartCore(void);

            /*
             * Copy constructor
             */
            SmartCore(const SmartCore& core);

            /*
             * Assignment operator
             */
            SmartCore& operator=(SmartCore const& core);

            int startProcesses(void);

            int stopProcesses(void);
    };
}

size_t receivedCommand(char* command, char* response, size_t maxLength);

#endif
