//
// Created by 98max on 11/10/2022.
//

#ifndef MAX_OS_DRIVERS_DRIVER_H
#define MAX_OS_DRIVERS_DRIVER_H

#include <common/outputStream.h>
#include <memory/memorymanagement.h>
#include <hardwarecommunication/interrupts.h>
#include <common/eventHandler.h>
namespace maxOS
{
    namespace drivers {


        class Driver {
            protected:
                common::OutputStream* driverMessageStream;

            public:
                Driver(common::OutputStream* driverMessageStream = 0);
                ~Driver();

                void errorMessage(common::string message);
                void errorMessage(char charToWrite);
                void errorMessage(int intToWrite);
                void errorMessage(common::uint32_t hexToWrite);

                virtual void activate();
                virtual void deactivate();
                virtual void initialise();
                virtual common::uint32_t reset();

                virtual common::string getVendorName();
                virtual common::string getDeviceName();
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
            virtual void selectDrivers(DriverSelectorEventHandler* handler, memory::MemoryManager* memoryManager, hardwarecommunication::InterruptManager* interruptManager, common::OutputStream* errorMessageStream);
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
