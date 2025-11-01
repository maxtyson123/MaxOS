/**
 * @file driver.h
 * @brief Defines a base Driver class and DriverManager for managing drivers
 *
 * @date 11th October 2022
 * @author Max Tyson
 */

#ifndef MAX_OS_DRIVERS_DRIVER_H
#define MAX_OS_DRIVERS_DRIVER_H

#include <stdint.h>
#include <common/outputStream.h>
#include <common/eventHandler.h>
#include <common/string.h>
#include <common/logger.h>
#include <memory/memorymanagement.h>

namespace MaxOS
{
    namespace drivers {


        /**
         * @class Driver
         * @brief base class for all drivers, handles the activation, deactivation, initialisation and reset of the driver as well as error messages and identifying the device
         */
        class Driver {
            public:

                Driver();
                ~Driver();

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
