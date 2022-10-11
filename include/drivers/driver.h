//
// Created by 98max on 11/10/2022.
//

#ifndef MAX_OS_DRIVERS_DRIVER_H
#define MAX_OS_DRIVERS_DRIVER_H

namespace maxos
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
        private:
            Driver *drivers[255];   //Fixed length for now as there isn't dynamic memory in the OS yet.
            int numDrivers;
        public:
            DriverManager();

            void AddDriver(Driver *);

            void ActivateAll();
        };
    }
}

#endif //MAX_OS_DRIVERS_DRIVER_H
