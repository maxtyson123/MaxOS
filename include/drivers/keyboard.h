//
// Created by 98max on 9/10/2022.
//

#ifndef MAX_OS_DRIVERS_KEYBOARD_H
#define MAX_OS_DRIVERS_KEYBOARD_H

#include <common/types.h>
#include <hardwarecommunication/interrupts.h>
#include <hardwarecommunication/port.h>
#include <drivers/driver.h>

namespace maxos
{
    namespace drivers {

        class KeyboardEventHandler {
        public:
            KeyboardEventHandler();

            virtual void OnKeyDown(char);

            virtual void OnKeyUp(char);
        };

        class KeyboardDriver : public maxos::hardwarecommunication::InterruptHandler, public Driver {
            maxos::hardwarecommunication::Port8Bit dataPort;
            maxos::hardwarecommunication::Port8Bit commandPort;

            KeyboardEventHandler *handler;
        public:
            KeyboardDriver(maxos::hardwarecommunication::InterruptManager *manager, KeyboardEventHandler *handler);

            ~KeyboardDriver();

            virtual maxos::common::uint32_t HandleInterrupt(maxos::common::uint32_t esp);

            virtual void Activate();
        };
    }
}

#endif //MAX_OS_DRIVERS_KEYBOARD_H
