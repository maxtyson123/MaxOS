//
// Created by 98max on 10/10/2022.
//

#ifndef MAX_OS_DRIVERS_PERIPHERALS_MOUSE_H
#define MAX_OS_DRIVERS_PERIPHERALS_MOUSE_H


#include <stdint.h>
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
                    int8_t x;
                    int8_t y;
                    MouseMoveEvent(int8_t x, int8_t y);
                    ~MouseMoveEvent();
            };

            class MouseDownEvent : public common::Event<MouseEvents>{
                public:
                    uint8_t button;
                    MouseDownEvent(uint8_t button);
                    ~MouseDownEvent();
            };

            class MouseUpEvent : public common::Event<MouseEvents>{
                public:
                    uint8_t button;
                    MouseUpEvent(uint8_t button);
                    ~MouseUpEvent();
            };

            class MouseEventHandler : public common::EventHandler<MouseEvents>{

                public:
                    MouseEventHandler();
                    ~MouseEventHandler();

                    common::Event<MouseEvents>* onEvent(common::Event<MouseEvents>* event);

                    virtual void onMouseDownEvent(uint8_t button);
                    virtual void onMouseUpEvent(uint8_t button);
                    virtual void onMouseMoveEvent(int8_t x, int8_t y);
            };


            class MouseDriver : public hardwarecommunication::InterruptHandler, public Driver, public common::EventManager<MouseEvents>{
                hardwarecommunication::Port8Bit dataPort;
                hardwarecommunication::Port8Bit commandPort;

                void HandleInterrupt();

                uint8_t buffer[3];
                uint8_t offest;
                uint8_t buttons;

            public:
                MouseDriver(hardwarecommunication::InterruptManager *manager);
                ~MouseDriver();

                virtual void activate();
                string getDeviceName();
            };
        }
    }
}


#endif //MAX_OS_DRIVERS_PERIPHERALS_MOUSE_H
