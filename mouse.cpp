//
// Created by 98max on 10/10/2022.
//

#include "mouse.h"

///__Handler__

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
    uint8_t status = (dataPort.Read() | 2);                  //Read current state then set it to 2 becuase this will be the new state and clear the bit
    commandPort.Write(0x60);                            //Tell: change current state
    dataPort.Write(status);                             //Write back the current state

    commandPort.Write(0xD4);
    dataPort.Write(0xF4);                               //Final Activation of mouse
    dataPort.Read();
}

uint32_t MouseDriver::HandleInterrupt(uint32_t esp){

    uint8_t status = commandPort.Read();
    if(!(status & 0x20)) //Only if the 6th bit of data is one then there is data to handle
        return esp;                      //Otherwise dont bother handling this input

    static int8_t x=40,y=12;                //Intialize mouse in the center of the screen

    buffer[offest] = dataPort.Read();       //Read mouse info into buffer

    if(handler == 0){                       //If theres no handler then dont do anything
        return esp;
    }

    offest = (offest + 1) % 3;              //Move through the offest

    if(offest == 0)//If the mouse data tramsission is complete (3rd peice of data is through)
    {

        handler->OnMouseMove(buffer[1], buffer[2]);     //If things go wrong with mouse in the future then y = -buffer[2];



        //Detect button press
        for (int i = 0; i < 3; ++i) {
            //move the bit, and compare it to buffer 0 != //move the bit, and compare it to buttons
            if((buffer[0] & (0x01 << i)) !=  (buttons & (0x01<<1))) //Check if its the same as the previous button state
            {
                //Handle the button press
                if(buttons & (0x1<<i))
                    handler->OnMouseUp(i+1);
                else
                    handler->OnMouseDown(i+1);

                }

            }

        buttons = buffer[0];
    }
    return esp;
}