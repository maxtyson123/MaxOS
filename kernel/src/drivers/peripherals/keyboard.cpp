//
// Created by 98max on 9/10/2022.
//

#include <drivers/peripherals/keyboard.h>



using namespace maxOS::common;
using namespace maxOS::drivers;
using namespace maxOS::drivers::peripherals;
using namespace maxOS::hardwarecommunication;


///___Handler___

KeyboardEventHandler::KeyboardEventHandler(){

};

void KeyboardEventHandler::onKeyDown(char*)
{
}

void KeyboardEventHandler::OnKeyUp(char*)
{
}



///___Driver___
KeyboardDriver::KeyboardDriver(InterruptManager* manager, KeyboardEventHandler *handler)
: InterruptHandler(0x21, manager),  //0x21 is keyboard object, pass the manager paramerter to the base object
  dataPort(0x60),
  commandPort(0x64)
{
    this->handler = handler;
}
KeyboardDriver::~KeyboardDriver(){

}

void printf(char* str, bool clearLine = false); //Forward declaration
void printfHex(uint8_t key);                    //Forward declaration

/**
 * @details Activate the keyboard driver
 */
void KeyboardDriver::Activate() {
        while (commandPort.Read() & 0x1)    //Wait for user to stop pressing key (this is for the start-up key eg.. hold 'F12' for boot menu or hold 'del' for bios ), The wait is needed as the keyboard controller won't send anymore characters until the buffer has been read
            dataPort.Read();
        commandPort.Write(0xAE);                            //Tell: PIC to send keyboard interrupt [or] tell keyboard to send interrupts to PIC
        commandPort.Write(0x20);                            //Tell: get current state
        uint8_t status = (dataPort.Read() | 1)  & ~ 0x10;        //Read current state then set rightmost bit to 1 becuase this will be the new state and clear the bit
        commandPort.Write(0x60);                            //Tell: change current state
        dataPort.Write(status);                             //Write back the current state

        dataPort.Write(0xF4);                               //Final Activation of keyboard

        //Keyboard Controller Commands :
        //
        //0xAE : Enable Keyboard
        //0x20 : Read command byte , after that we read the status from data port
        //0x60 : Write command byte , after that we change the state of the data port
}

/**
 * Handle the keyboard interrupt
 * @param esp  The stack pointer
 * @return returns the passed esp
 */
uint32_t KeyboardDriver::HandleInterrupt(uint32_t esp){

    uint8_t key = dataPort.Read();      //NOTE: The 8th bit is set to 1 if key is released and cleared to 0 if key is pressed

    if(handler == 0){
        return esp;
    }

    static bool Shift = false;


    switch (key) {
            //Initializers that can be ignored
            case 0x45:
                break;
            case 0xFA:
                break;
            case 0xC5:
                break;

            //handler->OnKeyDown(x);    by default it does nothing, however it can be defined in an another class derived from the KeyboardEventHandler class and implement in it OnKeyDown() that does anything when key is pressed

            //Top Row
            case 0x3B:
                handler->onKeyDown("F1"); break;
            case 0x3C:
                handler->onKeyDown("F2"); break;
            case 0x3D:
                handler->onKeyDown("F3"); break;
            case 0x3E:
                handler->onKeyDown("F4"); break;
            case 0x3F:
                handler->onKeyDown("F5"); break;
            case 0x40:
                handler->onKeyDown("F6"); break;
            case 0x41:
                handler->onKeyDown("F7"); break;
            case 0x42:
                handler->onKeyDown("F8"); break;
            case 0x43:
                handler->onKeyDown("F9"); break;
            case 0x44:
                handler->onKeyDown("F10"); break;
            case 0x57:
                handler->onKeyDown("F11"); break;
            case 0x58:
                handler->onKeyDown("F12"); break;

            //First Row
            case 0x29: (Shift) ? handler->onKeyDown("~") : handler->onKeyDown("`"); break;
            case 0x02: (Shift) ? handler->onKeyDown("!") : handler->onKeyDown("1"); break;
            case 0x03: (Shift) ? handler->onKeyDown("@") : handler->onKeyDown("2"); break;
            case 0x04: (Shift) ? handler->onKeyDown("#") : handler->onKeyDown("3"); break;
            case 0x05: (Shift) ? handler->onKeyDown("$") : handler->onKeyDown("4"); break;
            case 0x06: (Shift) ? handler->onKeyDown("%") : handler->onKeyDown("5"); break;
            case 0x07: (Shift) ? handler->onKeyDown("^") : handler->onKeyDown("6"); break;
            case 0x08: (Shift) ? handler->onKeyDown("&") : handler->onKeyDown("7"); break;
            case 0x09: (Shift) ? handler->onKeyDown("*") : handler->onKeyDown("8"); break;
            case 0x0A: (Shift) ? handler->onKeyDown("(") : handler->onKeyDown("9"); break;
            case 0x0B: (Shift) ? handler->onKeyDown(")") : handler->onKeyDown("0"); break;
            case 0x0C: (Shift) ? handler->onKeyDown("_") : handler->onKeyDown("-"); break;
            case 0x0D: (Shift) ? handler->onKeyDown("+") : handler->onKeyDown("="); break;

            //Second Row
            case 0x10: (Shift) ? handler->onKeyDown("Q") : handler->onKeyDown("q"); break;
            case 0x11: (Shift) ? handler->onKeyDown("W") : handler->onKeyDown("w"); break;
            case 0x12: (Shift) ? handler->onKeyDown("E") : handler->onKeyDown("e"); break;
            case 0x13: (Shift) ? handler->onKeyDown("R") : handler->onKeyDown("r"); break;
            case 0x14: (Shift) ? handler->onKeyDown("T") : handler->onKeyDown("t"); break;
            case 0x15: (Shift) ? handler->onKeyDown("Y") : handler->onKeyDown("y"); break;
            case 0x16: (Shift) ? handler->onKeyDown("U") : handler->onKeyDown("u"); break;
            case 0x17: (Shift) ? handler->onKeyDown("I") : handler->onKeyDown("i"); break;
            case 0x18: (Shift) ? handler->onKeyDown("O") : handler->onKeyDown("o"); break;
            case 0x19: (Shift) ? handler->onKeyDown("P") : handler->onKeyDown("p"); break;
            case 0x1A: (Shift) ? handler->onKeyDown("{") : handler->onKeyDown("["); break;
            case 0x1B: (Shift) ? handler->onKeyDown("}") : handler->onKeyDown("]"); break;

            //Third Row
            case 0x1E: (Shift) ? handler->onKeyDown("A") : handler->onKeyDown("a"); break;
            case 0x1F: (Shift) ? handler->onKeyDown("S") : handler->onKeyDown("s"); break;
            case 0x20: (Shift) ? handler->onKeyDown("D") : handler->onKeyDown("d"); break;
            case 0x21: (Shift) ? handler->onKeyDown("F") : handler->onKeyDown("f"); break;
            case 0x22: (Shift) ? handler->onKeyDown("G") : handler->onKeyDown("g"); break;
            case 0x23: (Shift) ? handler->onKeyDown("H") : handler->onKeyDown("h"); break;
            case 0x24: (Shift) ? handler->onKeyDown("J") : handler->onKeyDown("j"); break;
            case 0x25: (Shift) ? handler->onKeyDown("K") : handler->onKeyDown("k"); break;
            case 0x26: (Shift) ? handler->onKeyDown("L") : handler->onKeyDown("l"); break;
            case 0x27: (Shift) ? handler->onKeyDown(":") : handler->onKeyDown(";"); break;
            case 0x28: (Shift) ? handler->onKeyDown("\"") : handler->onKeyDown("'"); break;
            case 0x2B: (Shift) ? handler->onKeyDown("|") : handler->onKeyDown("\\"); break;

            //Row Four
            case 0x2C: (Shift) ? handler->onKeyDown("Z") : handler->onKeyDown("z"); break;
            case 0x2D: (Shift) ? handler->onKeyDown("X") : handler->onKeyDown("x"); break;
            case 0x2E: (Shift) ? handler->onKeyDown("C") : handler->onKeyDown("c"); break;
            case 0x2F: (Shift) ? handler->onKeyDown("V") : handler->onKeyDown("v"); break;
            case 0x30: (Shift) ? handler->onKeyDown("B") : handler->onKeyDown("b"); break;
            case 0x31: (Shift) ? handler->onKeyDown("N") : handler->onKeyDown("n"); break;
            case 0x32: (Shift) ? handler->onKeyDown("M") : handler->onKeyDown("m"); break;
            case 0x33: (Shift) ? handler->onKeyDown("<") : handler->onKeyDown(","); break;
            case 0x34: (Shift) ? handler->onKeyDown(">") : handler->onKeyDown("."); break;
            case 0x35: (Shift) ? handler->onKeyDown("?") : handler->onKeyDown("/"); break;

            //Numpad
            case 0x4A:
                handler->onKeyDown("-"); break;
            case 0x4E:
                handler->onKeyDown("+"); break;
            //Most other numpad are confilcting


            //Special
            case 0x1C:
                handler->onKeyDown("\n"); break;             //Enter
            case 0x39:
                handler->onKeyDown(" "); break;              //Space
            case 0x0F:
                handler->onKeyDown("    "); break;           //Tab
            case 0x1D:
                handler->onKeyDown("CRTL"); break;           //Left Control
            case 0x5B:
                handler->onKeyDown("WIN"); break;            //Windows Key
            case 0x3A: Shift = !Shift; break;                       //Caps Lock
            case 0x38:
                handler->onKeyDown("ALT"); break;            //Left ALT
            case 0x01:
                handler->onKeyDown("ESC"); break;            //Escape
            case 0x37:
                handler->onKeyDown("PRNT"); break;           //Print Screen
            case 0x46:
                handler->onKeyDown("SCRL"); break;           //Scroll Lock
            case 0x52:
                handler->onKeyDown("INST"); break;           //Insert
            case 0x47:
                handler->onKeyDown("HOME"); break;           //Home
            case 0x53:
                handler->onKeyDown("DEL"); break;            //Delete
            case 0x4F:
                handler->onKeyDown("END"); break;            //End
            case 0x51:
                handler->onKeyDown("PGDWN"); break;          //Page Down
            case 0x49:
                handler->onKeyDown("PGUP"); break;           //Page Up
            case 0x48:
                handler->onKeyDown("ARUP"); break;           //Arrow Up
            case 0x50:
                handler->onKeyDown("ARDN"); break;           //Arrow Down
            case 0x4B:
                handler->onKeyDown("ARLF"); break;           //Arrow Left
            case 0x4D:
                handler->onKeyDown("ARRT"); break;           //Arrow Right
            case 0x0E:
                handler->onKeyDown("BACKSPACE"); break;

            //   Left       Right
            case 0x2A: case 0x36: Shift = !Shift;  break;         //Shift Onpres
            case 0xAA: case 0xB6: Shift = !Shift;  break;        //Shift Onrelease


            ///TODO:
            ///Key Release
            ///Special Key FUNCTIONS

            default:
                if(key < 0x80) {              //Interrupts 0x80 onwards are just for keyrelease, therefore we can ignore printing them
                    printf("Keyboard 0x");
                    printfHex(key);
               }
                break;

    }



    return esp;
}