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

namespace MaxOS
{
    namespace drivers {


        /**
         * @class Driver
         * @brief base class for all drivers, handles the activation, deactivation, initialisation and reset of the driver as well as error messages and identifying the device
         */
        class Driver {
            protected:
            public:

                common::OutputStream* m_driver_message_stream;

                Driver(common::OutputStream* driverMessageStream = 0);
                ~Driver();

                void error_message(string message);
                void error_message(char char_to_write);
                void error_message(int int_to_write);
                void error_message(uint32_t hex_to_write);

                virtual void activate();
                virtual void deactivate();
                virtual void initialise();
                virtual uint32_t reset();

                virtual string get_vendor_name();
                virtual string get_device_name();
        };

        //NOTE: Driver doesn't use the EventHandler class because it doesn't need to be connected to anything (May want to change this later)
        /**
         * @class DriverSelectorEventHandler
         * @brief Event handler for the DriverSelector class, handles the event when a driver is selected
         */
        class DriverSelectorEventHandler
        {
          public:
              DriverSelectorEventHandler();
              ~DriverSelectorEventHandler();
              virtual void on_driver_selected(Driver*);
        };

        /**
         * @class DriverSelector
         * @brief Selects the drivers to be used
         */
        class DriverSelector
        {
          public:
              DriverSelector();
              ~DriverSelector();
              virtual void select_drivers(DriverSelectorEventHandler* handler, hardwarecommunication::InterruptManager* interruptManager);
        };

        /**
         * @class DriverManager
         * @brief Manages the drivers, handles the adding and removing of drivers
         */
        class DriverManager : public DriverSelectorEventHandler {
            public:
                DriverManager();
                ~DriverManager();

                void add_driver(Driver*);
                void remove_driver(Driver*);
                void on_driver_selected(Driver*) final;

                common::Vector<Driver*> drivers;
        };
    }
}

#endif //MAX_OS_DRIVERS_DRIVER_H
