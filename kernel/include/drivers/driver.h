//
// Created by 98max on 11/10/2022.
//

#ifndef MAX_OS_DRIVERS_DRIVER_H
#define MAX_OS_DRIVERS_DRIVER_H

namespace maxOS
{
    namespace drivers {

        class Driver {
            public:
                Driver();

                ~Driver();

                virtual void Activate();

                virtual void Deactivate();

                virtual int Reset();
        };

        class DriverManager {
            public:                     //Public For testing
                Driver *drivers[255];   //Fixed length for now as there is dynamic memory in the OS but I haven't setup drivers this way yet.
                int numDrivers;
            public:
                DriverManager();

                void AddDriver(Driver *);

                void ActivateAll();
        };
    }
}

#endif //MAX_OS_DRIVERS_DRIVER_H
