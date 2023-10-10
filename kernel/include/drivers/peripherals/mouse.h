//
// Created by 98max on 10/10/2022.
//

#ifndef MAX_OS_DRIVERS_PERIPHERALS_MOUSE_H
#define MAX_OS_DRIVERS_PERIPHERALS_MOUSE_H


#include <common/types.h>
#include <common/vector.h>
#include <common/eventHandler.h>
#include <hardwarecommunication/interrupts.h>
#include <hardwarecommunication/port.h>
#include <drivers/driver.h>

namespace maxOS {
    namespace drivers {
        namespace peripherals {

            enum MouseEvents{
                MOUSE_MOVE,
                MOUSE_DOWN,
                MOUSE_UP
            };

            class MouseMoveEvent : public common::Event<MouseEvents>{
                public:
                    common::int8_t x;
                    common::int8_t y;
                    MouseMoveEvent(common::int8_t x, common::int8_t y);
                    ~MouseMoveEvent();
            };

            class MouseDownEvent : public common::Event<MouseEvents>{
                public:
                    common::uint8_t button;
                    MouseDownEvent(common::uint8_t button);
                    ~MouseDownEvent();
            };

            class MouseUpEvent : public common::Event<MouseEvents>{
                public:
                    common::uint8_t button;
                    MouseUpEvent(common::uint8_t button);
                    ~MouseUpEvent();
            };

            class MouseEventHandler : public common::EventHandler<MouseEvents>{

                public:
                    MouseEventHandler();
                    ~MouseEventHandler();

                    void onEvent(common::Event<MouseEvents>* event);

                    virtual void onMouseDownEvent(common::uint8_t button);
                    virtual void onMouseUpEvent(common::uint8_t button);
                    virtual void onMouseMoveEvent(common::int8_t x, common::int8_t y);
            };


            class MouseDriver : public hardwarecommunication::InterruptHandler, public Driver, public common::EventManager<MouseEvents>{
                hardwarecommunication::Port8Bit dataPort;
                hardwarecommunication::Port8Bit commandPort;

                common::uint32_t HandleInterrupt(common::uint32_t esp);

                common::uint8_t buffer[3];
                common::uint8_t offest;
                common::uint8_t buttons;

            public:
                MouseDriver(hardwarecommunication::InterruptManager *manager);
                ~MouseDriver();

                virtual void activate();
                common::string getDeviceName();
            };
        }
    }
}


#endif //MAX_OS_DRIVERS_PERIPHERALS_MOUSE_H
