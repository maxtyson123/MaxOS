//
// Created by 98max on 11/10/2022.
//

#ifndef MAX_OS_DRIVERS_DRIVER_H
#define MAX_OS_DRIVERS_DRIVER_H

#include <common/outputStream.h>

namespace maxOS
{
    namespace drivers {

        class Driver {
            protected:
                common::OutputStream *driverMessageStream;

            public:
                Driver(common::OutputStream *driverMessageStream = 0);
                ~Driver();

                void errorMessage(common::string message);
                void errorMessage(char charToWrite);
                void errorMessage(int intToWrite);
                void errorMessage(common::uint32_t hexToWrite);


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

                // TODO: Device Names and Vendor
                // TODO: eventHandlers for drivers
        };
    }
}

#endif //MAX_OS_DRIVERS_DRIVER_H
