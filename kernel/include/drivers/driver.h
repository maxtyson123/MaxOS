//
// Created by 98max on 11/10/2022.
//

#ifndef MAX_OS_DRIVERS_DRIVER_H
#define MAX_OS_DRIVERS_DRIVER_H

#include <common/outputStream.h>
#include <memory/memorymanagement.h>
#include <hardwarecommunication/interrupts.h>

namespace maxOS
{
    namespace drivers {

        class DriverEventHandler
        {
            public:
                DriverEventHandler();
                ~DriverEventHandler();

                //Todo: yea... this is a bit of a mess. I should probably make a class for the event itself.
        };

        class Driver {
            protected:
                common::OutputStream* driverMessageStream;
                common::Vector<DriverEventHandler*> driverEventHandlers;

            public:
                Driver(common::OutputStream* driverMessageStream = 0);
                ~Driver();

                void errorMessage(common::string message);
                void errorMessage(char charToWrite);
                void errorMessage(int intToWrite);
                void errorMessage(common::uint32_t hexToWrite);

                void connectDriverEventHandler(DriverEventHandler* driverEventHandler);
                void disconnectDriverEventHandler(DriverEventHandler* driverEventHandler);

                virtual void activate();
                virtual void deactivate();
                virtual common::uint32_t reset();

                virtual common::string getVendorName();
                virtual common::string getDeviceName();
        };

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
