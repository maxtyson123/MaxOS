//
// Created by 98max on 9/10/2022.
//

#ifndef MAX_OS_KEYBOARD_H
#define MAX_OS_KEYBOARD_H

#include "types.h"
#include "interrupts.h"
#include "port.h"
#include "driver.h"

class KeyboardEventHandler{
    public:
        KeyboardEventHandler();

        virtual void OnKeyDown(char);
        virtual void OnKeyUp(char);
};

class KeyboardDriver : public InterruptHandler, public Driver{
        Port8Bit dataPort;
        Port8Bit commandPort;

        KeyboardEventHandler* handler;
    public:
        KeyboardDriver(InterruptManager* manager, KeyboardEventHandler *handler);
        ~KeyboardDriver();
        virtual uint32_t HandleInterrupt(uint32_t esp);
        virtual void Activate();
};


#endif //MAX_OS_KEYBOARD_H
