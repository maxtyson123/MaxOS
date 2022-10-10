//
// Created by 98max on 10/10/2022.
//

#include "mouse.h"

MouseDriver::MouseDriver(InterruptManager* manager)
        : InterruptHandler(0x2C, manager),  //0x2C is mouse object, pass the manager paramerter to the base object
          dataPort(0x60),
          commandPort(0x64)
{

    offest = 0; //The mouse is weird and won't write to exactly 0 sometimes, so there has to be differnet offests for different os-es
    buttons = 0;

    uint16_t* VideoMemory = (uint16_t*)0xb8000;

    //Show the intial cursor
    VideoMemory[80*12+40] = (VideoMemory[80*12+40] & 0x0F00) << 4           //Get High 4 bits and shift to right (Foreground becomes Background)
                          | (VideoMemory[80*12+40] & 0xF000) >> 4         //Get Low 4 bits and shift to left (Background becomes Foreground)
                          | (VideoMemory[80*12+40] & 0x00FF);             //Keep the last 8 bytes the same (The character)

    commandPort.Write(0xAB);                            //Tell: PIC to send mouse interrupt [or] tell mouse to send interrupts to PIC
    commandPort.Write(0x20);                            //Tell: get current state
    uint8_t status = (dataPort.Read() | 2);                  //Read current state then set it to 2 becuase this will be the new state and clear the bit
    commandPort.Write(0x60);                            //Tell: change current state
    dataPort.Write(status);                             //Write back the current state

    commandPort.Write(0xD4);
    dataPort.Write(0xF4);                               //Final Activation of mouse
    dataPort.Read();
}
MouseDriver::~MouseDriver(){

}

void printf(char* str, bool clearLine = false); //Forward declaration

uint32_t MouseDriver::HandleInterrupt(uint32_t esp){

    uint8_t status = commandPort.Read();
    if(!(status & 0x20)) //Only if the 6th bit of data is one then there is data to handle
        return esp;      //Otherwise dont bother handling this input

    static int8_t x=40,y=12;                //Intialize mouse in the center of the screen

    buffer[offest] = dataPort.Read();       //Read mouse info into buffer
    offest = (offest + 1) % 3;              //Move through the offest

    if(offest == 0)//If the mouse data tramsission is complete (3rd peice of data is through)
    {

        static uint16_t* VideoMemory = (uint16_t*)0xb8000;

        //Show the old cursor
        VideoMemory[80*y+x] = (VideoMemory[80*y+x] & 0x0F00) << 4           //Get High 4 bits and shift to right (Foreground becomes Background)
                              | (VideoMemory[80*y+x] & 0xF000) >> 4         //Get Low 4 bits and shift to left (Background becomes Foreground)
                              | (VideoMemory[80*y+x] & 0x00FF);             //Keep the last 8 bytes the same (The character)

        x += buffer[1];     //Movement on the x-axis (note, mouse passes the info inverted)
        y -= buffer[2];     //Movement on the y-axis (note, mouse passes the info inverted)

        //Make sure mouse position not out of bounds
        if(x < 0) x = 0;
        if(x >= 80) x = 79;

        if(y < 0) y = 0;
        if(y >= 25) y = 24;



        //Show the new cursor by inverting the current character
        VideoMemory[80*y+x] = (VideoMemory[80*y+x] & 0x0F00) << 4           //Get High 4 bits and shift to right (Foreground becomes Background)
                              | (VideoMemory[80*y+x] & 0xF000) >> 4         //Get Low 4 bits and shift to left (Background becomes Foreground)
                              | (VideoMemory[80*y+x] & 0x00FF);             //Keep the last 8 bytes the same (The character)

        //Detect button press
        for (int i = 0; i < 3; ++i) {
            //move the bit, and compare it to buffer 0 != //move the bit, and compare it to buttons
            if((buffer[0] & (0x01 << i)) !=  (buttons & (0x01<<1))) //Check if its the same as the previous button state
            {
                //Handle the button press

                //For debuging
                char* press = "_";
                switch (i) {
                    case 0:
                        press = "*";
                        break;
                    case 1:
                        press = "&";
                        break;
                    case 2:
                        press = "^";
                        break;

                }
                VideoMemory[80*y+x] = (VideoMemory[80*y+x] & 0xFF00) | press[0];
            }
        }
        buttons = buffer[0];
    }
    return esp;
}