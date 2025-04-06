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

                Driver(common::OutputStream* driverMessageStream = nullptr);
                ~Driver();

                void error_message(const string& message) const;
                void error_message(char char_to_write) const;
                void error_message(int int_to_write) const;
                void error_message(uint32_t hex_to_write) const;

                virtual void activate();
                virtual void deactivate();
                virtual void initialise();
                virtual uint32_t reset();

                virtual string vendor_name();
                virtual string device_name();
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
              virtual void select_drivers(DriverSelectorEventHandler* handler);
        };

        /**
         * @class DriverManager
         * @brief Manages the drivers, handles the adding and removing of drivers
         */
        class DriverManager : public DriverSelectorEventHandler {
            private:

              common::Vector<DriverSelector*> m_driver_selectors;
              common::Vector<Driver*> m_drivers;

            public:
                DriverManager();
                ~DriverManager();

                void add_driver_selector(DriverSelector*);
                void remove_driver_selector(DriverSelector*);

                void add_driver(Driver*);
                void remove_driver(Driver*);
                void on_driver_selected(Driver*) final;

                void find_drivers();
                uint32_t reset_devices();
                void initialise_drivers();
                void deactivate_drivers();
                void activate_drivers();


        };
    }
}

#endif //MAX_OS_DRIVERS_DRIVER_H
