//
// Created by 98max on 10/10/2022.
//

#include <drivers/peripherals/mouse.h>

using namespace maxOS::common;
using namespace maxOS::drivers;
using namespace maxOS::drivers::peripherals;
using namespace maxOS::hardwarecommunication;


///__Handler__

MouseEventHandler::MouseEventHandler() {
}

void MouseEventHandler::onMouseDownEvent(uint8_t button){

}

void MouseEventHandler::onMouseUpEvent(uint8_t button){

}

void MouseEventHandler::onMouseMoveEvent(int8_t x, int8_t y){

}

MouseEventHandler::~MouseEventHandler() {

}


///__Driver__

MouseDriver::MouseDriver(InterruptManager* manager)
        : InterruptHandler(0x2C, manager),  //0x2C is mouse object, pass the manager paramerter to the base object
          dataPort(0x60),
          commandPort(0x64)
{
    offest = 2; //The mouse is weird and won't write to exactly 0 sometimes, so there has to be different offsets for different os-es
    buttons = 0;
}
MouseDriver::~MouseDriver(){

}

/**
 * @details Activate the mouse
 */
void MouseDriver::Activate() {


    commandPort.Write(0xAB);                            //Tell: PIC to send mouse interrupt [or] tell mouse to send interrupts to PIC
    commandPort.Write(0x20);                            //Tell: get current state
    uint8_t status = (dataPort.Read() | 2);                  //Read current state then set it to 2 becuase this will be the new state and clear the bit
    commandPort.Write(0x60);                            //Tell: change current state
    dataPort.Write(status);                             //Write back the current state

    commandPort.Write(0xD4);                            //Forward commands
    dataPort.Write(0xF4);                               //Final Activation of mouse
    dataPort.Read();

    //Commands to the keyboard controller: (YES the keyboard)
    //0xA8 | Activate mouse
    //0xA7 | Deactivate mouse
    //0x20 | Read command byte
    //0x60 | Write command byte
    //0xD4 | send the next command to the mouse instead of the keyboard

    //Commands to the mouse controller: (Once forwarded)
    //0xF4 | Tell the mouse to send data to the CPU , Enable Packet streaming when mouse is moved or clicked .
    //0xF5 | Tell the mouse not to send any data to the CPU
    //0xF6 | Reset mouse settings to default settings
}

/**
 * @details Handle the mouse interrupt
 *
 * @param esp
 * @return always returns esp
 */
uint32_t MouseDriver::HandleInterrupt(uint32_t esp){

    //The mouse triggers 3 interrupts , one for each byte .
    //Byte 1 : Y overflow | X overflow | Y sign bit | X sign bit | Reserved (1) | Middle button pressed | Right button pressed | Left button pressed
    //Byte 2 : X Movement since the last data packet. "delta X" value -- that is, it measures horizontal mouse movement, with left being negative.
    //Byte 3 : Y Movement since the last data packet, "delta Y", with down (toward the user) being negative.

    uint8_t status = commandPort.Read();
    if(!(status & 0x20)) //Only if the 6th bit of data is one then there is data to handle
        return esp;                      //Otherwise don't bother handling this input


    buffer[offest] = dataPort.Read();       //Read mouse info into buffer
    offest = (offest + 1) % 3;              //Move through the offset

    if(offest == 0)//If the mouse data transmission is complete (3rd piece of data is through)
    {

        // Pass the mouse data to the mouse event handlers
        for(Vector<MouseEventHandler*>::iterator mouseEventHandler = mouseEventHandlers.begin(); mouseEventHandler != mouseEventHandlers.end(); mouseEventHandler++)
        {

            // If the mouse is moved (buffer 1 and 2 store x and y)
            if(buffer[1] != 0 || buffer[2] != 0)
                (*mouseEventHandler) -> onMouseMoveEvent((int8_t) buffer[1], -((int8_t) buffer[2]));     //If things go wrong with mouse in the future then y = -buffer[2];

            //Detect button press
            for (int i = 0; i < 3; ++i) {


                //Check if it's the same as the previous becuase if the current state of the buttons is not equal to the previous state of the buttons , then the button must have been pressed or released
                if((buffer[0] & (0x01 << i)) !=  (buttons & (0x01<<1)))
                {
                    //This if condition is true if the previous state of the button was set to 1 (it was pressed) , so now it must be released as the button state has changed
                    if(buttons & (0x1<<i))
                        (*mouseEventHandler) -> onMouseUpEvent(i + 1);
                    else
                        (*mouseEventHandler) -> onMouseDownEvent(i + 1);

                }

            }

            // Update the buttons
            buttons = buffer[0];  // TODO: Maybe do this out side of the loop?
        };





    }
    return esp;
}

/**
 * @details Connect a mouse event handler to the mouse driver
 *
 * @param handler the mouse event handler to connect
 */
void MouseDriver::connectMouseEventHandler(MouseEventHandler *handler) {

    // Check if the handler is already connected (find returns end if not found)
    if(mouseEventHandlers.find(handler) != mouseEventHandlers.end())
        return;

    // Add the handler to the list of handlers
    mouseEventHandlers.pushBack(handler);

}

/**
 * @details Disconnect a mouse event handler from the mouse driver
 *
 * @param handler The mouse event handler to disconnect
 */
void MouseDriver::disconnectMouseEventHandler(MouseEventHandler *handler) {

    // Check if the handler is connected (find returns end if not found)
    if(mouseEventHandlers.find(handler) == mouseEventHandlers.end())
        return;

    // Remove the handler from the list of handlers
    mouseEventHandlers.erase(handler);

}
