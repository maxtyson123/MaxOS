//
// Created by 98max on 10/10/2022.
//

#include <drivers/mouse.h>

using namespace maxOS::common;
using namespace maxOS::drivers;
using namespace maxOS::hardwarecommunication;


///__Handler__

MouseEventHandler::MouseEventHandler() {
}

void MouseEventHandler::OnActivate(){

}

void MouseEventHandler::OnMouseDown(uint8_t button){

}

void MouseEventHandler::OnMouseUp(uint8_t button){

}

void MouseEventHandler::OnMouseMove(int x, int y){

}


///__Driver__

MouseDriver::MouseDriver(InterruptManager* manager, MouseEventHandler* handler)
        : InterruptHandler(0x2C, manager),  //0x2C is mouse object, pass the manager paramerter to the base object
          dataPort(0x60),
          commandPort(0x64)
{
    this->handler = handler;
}
MouseDriver::~MouseDriver(){

}

void printf(char* str, bool clearLine = false); //Forward declaration

void MouseDriver::Activate() {
    offest = 0; //The mouse is weird and won't write to exactly 0 sometimes, so there has to be different offsets for different os-es
    buttons = 0;

    commandPort.Write(0xAB);                            //Tell: PIC to send mouse interrupt [or] tell mouse to send interrupts to PIC
    commandPort.Write(0x20);                            //Tell: get current state
    uint8_t status = (dataPort.Read() | 2);             //Read current state then set it to 2 becuase this will be the new state and clear the bit
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

uint32_t MouseDriver::HandleInterrupt(uint32_t esp){

    //The mouse triggers 3 interrupts , one for each byte .
    //Byte 1 : Y overflow | X overflow | Y sign bit | X sign bit | Reserved (1) | Middle button pressed | Right button pressed | Left button pressed
    //Byte 2 : X Movement since the last data packet. "delta X" value -- that is, it measures horizontal mouse movement, with left being negative.
    //Byte 3 : Y Movement since the last data packet, "delta Y", with down (toward the user) being negative.

    uint8_t status = commandPort.Read();
    if(!(status & 0x20)) //Only if the 6th bit of data is one then there is data to handle
        return esp;                      //Otherwise don't bother handling this input

    static int8_t x=40,y=12;                //Initialize mouse in the center of the screen

    buffer[offest] = dataPort.Read();       //Read mouse info into buffer

    if(handler == 0){                       //If there's no handler then don't do anything
        return esp;
    }

    offest = (offest + 1) % 3;              //Move through the offset

    if(offest == 0)//If the mouse data transmission is complete (3rd piece of data is through)
    {

        handler->OnMouseMove((int8_t)buffer[1], -((int8_t)buffer[2]));     //If things go wrong with mouse in the future then y = -buffer[2];



        //Detect button press
        for (int i = 0; i < 3; ++i) {
            //move the bit, and compare it to buffer 0 != //move the bit, and compare it to buttons
            if((buffer[0] & (0x01 << i)) !=  (buttons & (0x01<<1))) //Check if it's the same as the previous becuase if the current state of the buttons is not equal to the previous state of the buttons , then the button must have been pressed or released
            {
                //Handle the button press/release
                if(buttons & (0x1<<i))                  //This if condition is true if the previous state of the button was set to 1 (it was pressed) , so now it must be released as the button state has changed
                    handler->OnMouseUp(i+1);
                else
                    handler->OnMouseDown(i+1);

                }

            }

        buttons = buffer[0];
    }
    return esp;
}