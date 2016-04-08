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

namespace SC
{
    enum OperationMode
    {
        MODE_STANDALONE,
        MODE_SERVER_VERIFICATION,
        MODE_SERVER_ROUTE,
        MODE_SERVER_DIRECTIONS,
        MODE_MAX
    };

    class SmartCore
    {
        public:
            /*
             * Default constructor
             */
            SmartCore(void);

            /*
             * Copy constructor
             */
            SmartCore(const SmartCore& core);

            /*
             * Destructor
             */
            virtual ~SmartCore(void);

            int initialiseCore(int argc, char *argv[]);

            int run(void);

            void stop(void);

            bool isRunning(void);

        private:
            bool running;
            bool abort;
            OperationMode mode;
    };
}

#endif
