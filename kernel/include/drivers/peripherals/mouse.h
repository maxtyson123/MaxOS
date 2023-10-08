//
// Created by 98max on 10/10/2022.
//

#ifndef MAX_OS_DRIVERS_PERIPHERALS_MOUSE_H
#define MAX_OS_DRIVERS_PERIPHERALS_MOUSE_H


#include <common/types.h>
#include <common/vector.h>
#include <hardwarecommunication/interrupts.h>
#include <hardwarecommunication/port.h>
#include <drivers/driver.h>

namespace maxOS {
    namespace drivers {
        namespace peripherals {

            class MouseEventHandler {
                public:
                    MouseEventHandler();
                    ~MouseEventHandler();

                    virtual void onMouseDownEvent(common::uint8_t button);
                    virtual void onMouseUpEvent(common::uint8_t button);
                    virtual void onMouseMoveEvent(common::int8_t x, common::int8_t y);
            };


            class MouseDriver : public hardwarecommunication::InterruptHandler, public Driver {
                hardwarecommunication::Port8Bit dataPort;
                hardwarecommunication::Port8Bit commandPort;

                common::uint32_t HandleInterrupt(common::uint32_t esp);

                common::uint8_t buffer[3];
                common::uint8_t offest;
                common::uint8_t buttons;

                common::Vector<MouseEventHandler*> mouseEventHandlers;

            public:
                MouseDriver(hardwarecommunication::InterruptManager *manager);
                ~MouseDriver();

                virtual void activate();
                common::string getDeviceName();

                void connectMouseEventHandler(MouseEventHandler* handler);
                void disconnectMouseEventHandler(MouseEventHandler* handler);

            };
        }
    }
}


#endif //MAX_OS_DRIVERS_PERIPHERALS_MOUSE_H
