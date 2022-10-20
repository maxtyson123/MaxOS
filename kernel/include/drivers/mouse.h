//
// Created by 98max on 10/10/2022.
//

#ifndef MAX_OS_DRIVERS_MOUSE_H
#define MAX_OS_DRIVERS_MOUSE_H


#include <common/types.h>
#include <hardwarecommunication/interrupts.h>
#include <hardwarecommunication/port.h>
#include <drivers/driver.h>

namespace maxOS {
    namespace drivers {

        class MouseEventHandler {
        public:
            MouseEventHandler();

            virtual void OnActivate();

            virtual void OnMouseDown(maxOS::common::uint8_t button);

            virtual void OnMouseUp(maxOS::common::uint8_t button);

            virtual void OnMouseMove(int x, int y);
        };


        class MouseDriver : public maxOS::hardwarecommunication::InterruptHandler, public Driver {
            maxOS::hardwarecommunication::Port8Bit dataPort;
            maxOS::hardwarecommunication::Port8Bit commandPort;

            maxOS::common::uint8_t buffer[3];
            maxOS::common::uint8_t offest;
            maxOS::common::uint8_t buttons;

            MouseEventHandler *handler;

        public:
            MouseDriver(maxOS::hardwarecommunication::InterruptManager *manager, MouseEventHandler *handler);

            ~MouseDriver();

            virtual maxOS::common::uint32_t HandleInterrupt(maxOS::common::uint32_t esp);

            virtual void Activate();
        };
    }
}


#endif //MAX_OS_DRIVERS_MOUSE_H
