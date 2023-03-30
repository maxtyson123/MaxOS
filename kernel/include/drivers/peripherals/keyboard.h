//
// Created by 98max on 9/10/2022.
//

#ifndef MAX_OS_DRIVERS_PERIPHERALS_KEYBOARD_H
#define MAX_OS_DRIVERS_PERIPHERALS_KEYBOARD_H

#include <common/types.h>
#include <hardwarecommunication/interrupts.h>
#include <hardwarecommunication/port.h>
#include <drivers/driver.h>

namespace maxOS
{
    namespace drivers {

        namespace peripherals{

            class KeyboardEventHandler {
            public:
                KeyboardEventHandler();

                virtual void onKeyDown(char*);

                virtual void OnKeyUp(char*);
            };

            class KeyboardDriver : public maxOS::hardwarecommunication::InterruptHandler, public Driver {
                maxOS::hardwarecommunication::Port8Bit dataPort;
                maxOS::hardwarecommunication::Port8Bit commandPort;

                KeyboardEventHandler *handler;
            public:
                KeyboardDriver(maxOS::hardwarecommunication::InterruptManager *manager, KeyboardEventHandler *handler);

                ~KeyboardDriver();

                virtual maxOS::common::uint32_t HandleInterrupt(maxOS::common::uint32_t esp);

                virtual void Activate();
            };

        }


    }
}

#endif //MAX_OS_DRIVERS_PERIPHERALS_KEYBOARD_H
