//
// Created by 98max on 9/10/2022.
//

#ifndef MAX_OS_KEYBOARD_H
#define MAX_OS_KEYBOARD_H

#include "types.h"
#include "interrupts.h"
#include "port.h"



class KeyboardDriver : public InterruptHandler{
        Port8Bit dataPort;
        Port8Bit commandPort;
    public:
        KeyboardDriver(InterruptManager* manager);
        ~KeyboardDriver();
        virtual uint32_t HandleInterrupt(uint32_t esp);

};


#endif //MAX_OS_KEYBOARD_H
