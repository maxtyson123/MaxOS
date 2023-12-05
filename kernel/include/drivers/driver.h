//
// Created by 98max on 11/10/2022.
//

#ifndef MAX_OS_DRIVERS_DRIVER_H
#define MAX_OS_DRIVERS_DRIVER_H

#include <stdint.h>
#include <common/outputStream.h>
#include <memory/memorymanagement.h>
#include <hardwarecommunication/interrupts.h>
#include <common/eventHandler.h>
#include <common/string.h>

namespace maxOS
{
    namespace drivers {


        class Driver {
            protected:
            public:

                common::OutputStream* driverMessageStream;
                Driver(common::OutputStream* driverMessageStream = 0);
                ~Driver();

                void errorMessage(string message);
                void errorMessage(char charToWrite);
                void errorMessage(int intToWrite);
                void errorMessage(uint32_t hexToWrite);

                virtual void activate();
                virtual void deactivate();
                virtual void initialise();
                virtual uint32_t reset();

                virtual string getVendorName();
                virtual string getDeviceName();
        };

        //NOTE: Driver doesn't use the EventHandler class because it doesn't need to be connected to anything (May want to change this later)
        class DriverSelectorEventHandler
        {
        public:
            DriverSelectorEventHandler();
            ~DriverSelectorEventHandler();
            virtual void onDriverSelected(Driver* driver);
        };

        class DriverSelector
        {
        public:
            DriverSelector();
            ~DriverSelector();
            virtual void selectDrivers(DriverSelectorEventHandler* handler, hardwarecommunication::InterruptManager* interruptManager, common::OutputStream* errorMessageStream);
        };

        class DriverManager : public DriverSelectorEventHandler {
            public:
                common::Vector<Driver*> drivers;
            public:
                DriverManager();
                ~DriverManager();

                void addDriver(Driver*);
                void removeDriver(Driver*);
                void onDriverSelected(Driver* driver);
        };
    }
}

#endif //MAX_OS_DRIVERS_DRIVER_H
