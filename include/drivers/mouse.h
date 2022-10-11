//
// Created by 98max on 10/10/2022.
//

#ifndef MAX_OS_DRIVERS_MOUSE_H
#define MAX_OS_DRIVERS_MOUSE_H


#include <common/types.h>
#include <hardwarecommunication/interrupts.h>
#include <hardwarecommunication/port.h>
#include <drivers/driver.h>

namespace maxos {
    namespace drivers {

        class MouseEventHandler {
        public:
            MouseEventHandler();

            virtual void OnActivate();

            virtual void OnMouseDown(maxos::common::uint8_t button);

            virtual void OnMouseUp(maxos::common::uint8_t button);

            virtual void OnMouseMove(int x, int y);
        };


        class MouseDriver : public maxos::hardwarecommunication::InterruptHandler, public Driver {
            maxos::hardwarecommunication::Port8Bit dataPort;
            maxos::hardwarecommunication::Port8Bit commandPort;

            maxos::common::uint8_t buffer[3];
            maxos::common::uint8_t offest;
            maxos::common::uint8_t buttons;

            MouseEventHandler *handler;

        public:
            MouseDriver(maxos::hardwarecommunication::InterruptManager *manager, MouseEventHandler *handler);

            ~MouseDriver();

            virtual maxos::common::uint32_t HandleInterrupt(maxos::common::uint32_t esp);

            virtual void Activate();
        };
    }
}


#endif //MAX_OS_DRIVERS_MOUSE_H
