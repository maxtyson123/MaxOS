//
// Created by 98max on 10/10/2022.
//

#ifndef MAX_OS_MOUSE_H
#define MAX_OS_MOUSE_H


#include "types.h"
#include "interrupts.h"
#include "port.h"
#include "driver.h"

class MouseEventHandler{
 public:
    MouseEventHandler();

    virtual void OnActivate();
    virtual void OnMouseDown(uint8_t button);
    virtual void OnMouseUp(uint8_t button);
    virtual void OnMouseMove(int x, int y);
};


class MouseDriver : public InterruptHandler, public Driver{
    Port8Bit dataPort;
    Port8Bit commandPort;

    uint8_t buffer[3];
    uint8_t offest;
    uint8_t buttons;

    MouseEventHandler* handler;

public:
    MouseDriver(InterruptManager* manager, MouseEventHandler* handler);
    ~MouseDriver();
    virtual uint32_t HandleInterrupt(uint32_t esp);
    virtual void Activate();
};


#endif //MAX_OS_MOUSE_H
