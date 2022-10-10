//
// Created by 98max on 10/10/2022.
//

#ifndef MAX_OS_MOUSE_H
#define MAX_OS_MOUSE_H


#include "types.h"
#include "interrupts.h"
#include "port.h"



class MouseDriver : public InterruptHandler{
    Port8Bit dataPort;
    Port8Bit commandPort;

    uint8_t buffer[3];
    uint8_t offest;
    uint8_t buttons;

public:
    MouseDriver(InterruptManager* manager);
    ~MouseDriver();
    virtual uint32_t HandleInterrupt(uint32_t esp);

};


#endif //MAX_OS_MOUSE_H
