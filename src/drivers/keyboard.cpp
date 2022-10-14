//
// Created by 98max on 9/10/2022.
//

#include <drivers/keyboard.h>



using namespace maxos::common;
using namespace maxos::drivers;
using namespace maxos::hardwarecommunication;

///___Handler___

KeyboardEventHandler::KeyboardEventHandler(){

};

void KeyboardEventHandler::OnKeyDown(char)
{
}

void KeyboardEventHandler::OnKeyUp(char)
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
            case 0x3B: handler->OnKeyDown('F1'); break;
            case 0x3C: handler->OnKeyDown('F2'); break;
            case 0x3D: handler->OnKeyDown('F3'); break;
            case 0x3E: handler->OnKeyDown('F4'); break;
            case 0x3F: handler->OnKeyDown('F5'); break;
            case 0x40: handler->OnKeyDown('F6'); break;
            case 0x41: handler->OnKeyDown('F7'); break;
            case 0x42: handler->OnKeyDown('F8'); break;
            case 0x43: handler->OnKeyDown('F9'); break;
            case 0x44: handler->OnKeyDown('F10'); break;
            case 0x57: handler->OnKeyDown('F11'); break;
            case 0x58: handler->OnKeyDown('F12'); break;

            //First Row
            case 0x29: (Shift) ? handler->OnKeyDown('~') : handler->OnKeyDown('`'); break;
            case 0x02: (Shift) ? handler->OnKeyDown('!') : handler->OnKeyDown('1'); break;
            case 0x03: (Shift) ? handler->OnKeyDown('@') : handler->OnKeyDown('2'); break;
            case 0x04: (Shift) ? handler->OnKeyDown('#') : handler->OnKeyDown('3'); break;
            case 0x05: (Shift) ? handler->OnKeyDown('$') : handler->OnKeyDown('4'); break;
            case 0x06: (Shift) ? handler->OnKeyDown('%') : handler->OnKeyDown('5'); break;
            case 0x07: (Shift) ? handler->OnKeyDown('^') : handler->OnKeyDown('6'); break;
            case 0x08: (Shift) ? handler->OnKeyDown('&') : handler->OnKeyDown('7'); break;
            case 0x09: (Shift) ? handler->OnKeyDown('*') : handler->OnKeyDown('8'); break;
            case 0x0A: (Shift) ? handler->OnKeyDown('(') : handler->OnKeyDown('9'); break;
            case 0x0B: (Shift) ? handler->OnKeyDown(')') : handler->OnKeyDown('0'); break;
            case 0x0C: (Shift) ? handler->OnKeyDown('_') : handler->OnKeyDown('-'); break;
            case 0x0D: (Shift) ? handler->OnKeyDown('+') : handler->OnKeyDown('='); break;

            //Second Row
            case 0x10: (Shift) ? handler->OnKeyDown('Q') : handler->OnKeyDown('q'); break;
            case 0x11: (Shift) ? handler->OnKeyDown('W') : handler->OnKeyDown('w'); break;
            case 0x12: (Shift) ? handler->OnKeyDown('E') : handler->OnKeyDown('e'); break;
            case 0x13: (Shift) ? handler->OnKeyDown('R') : handler->OnKeyDown('r'); break;
            case 0x14: (Shift) ? handler->OnKeyDown('T') : handler->OnKeyDown('t'); break;
            case 0x15: (Shift) ? handler->OnKeyDown('Y') : handler->OnKeyDown('y'); break;
            case 0x16: (Shift) ? handler->OnKeyDown('U') : handler->OnKeyDown('u'); break;
            case 0x17: (Shift) ? handler->OnKeyDown('I') : handler->OnKeyDown('i'); break;
            case 0x18: (Shift) ? handler->OnKeyDown('O') : handler->OnKeyDown('o'); break;
            case 0x19: (Shift) ? handler->OnKeyDown('P') : handler->OnKeyDown('p'); break;
            case 0x1A: (Shift) ? handler->OnKeyDown('{') : handler->OnKeyDown('['); break;
            case 0x1B: (Shift) ? handler->OnKeyDown('}') : handler->OnKeyDown(']'); break;

            //Third Row
            case 0x1E: (Shift) ? handler->OnKeyDown('A') : handler->OnKeyDown('a'); break;
            case 0x1F: (Shift) ? handler->OnKeyDown('S') : handler->OnKeyDown('s'); break;
            case 0x20: (Shift) ? handler->OnKeyDown('D') : handler->OnKeyDown('d'); break;
            case 0x21: (Shift) ? handler->OnKeyDown('F') : handler->OnKeyDown('f'); break;
            case 0x22: (Shift) ? handler->OnKeyDown('G') : handler->OnKeyDown('g'); break;
            case 0x23: (Shift) ? handler->OnKeyDown('H') : handler->OnKeyDown('h'); break;
            case 0x24: (Shift) ? handler->OnKeyDown('J') : handler->OnKeyDown('j'); break;
            case 0x25: (Shift) ? handler->OnKeyDown('K') : handler->OnKeyDown('k'); break;
            case 0x26: (Shift) ? handler->OnKeyDown('L') : handler->OnKeyDown('l'); break;
            case 0x27: (Shift) ? handler->OnKeyDown(':') : handler->OnKeyDown(';'); break;
            case 0x28: (Shift) ? handler->OnKeyDown('"') : handler->OnKeyDown('\''); break;
            case 0x2B: (Shift) ? handler->OnKeyDown('|') : handler->OnKeyDown('\\'); break;

            //Row Four
            case 0x2C: (Shift) ? handler->OnKeyDown('Z') : handler->OnKeyDown('z'); break;
            case 0x2D: (Shift) ? handler->OnKeyDown('X') : handler->OnKeyDown('x'); break;
            case 0x2E: (Shift) ? handler->OnKeyDown('C') : handler->OnKeyDown('c'); break;
            case 0x2F: (Shift) ? handler->OnKeyDown('V') : handler->OnKeyDown('v'); break;
            case 0x30: (Shift) ? handler->OnKeyDown('B') : handler->OnKeyDown('b'); break;
            case 0x31: (Shift) ? handler->OnKeyDown('N') : handler->OnKeyDown('n'); break;
            case 0x32: (Shift) ? handler->OnKeyDown('M') : handler->OnKeyDown('m'); break;
            case 0x33: (Shift) ? handler->OnKeyDown('<') : handler->OnKeyDown(','); break;
            case 0x34: (Shift) ? handler->OnKeyDown('>') : handler->OnKeyDown('.'); break;
            case 0x35: (Shift) ? handler->OnKeyDown('?') : handler->OnKeyDown('/'); break;

            //Numpad
            case 0x4A: handler->OnKeyDown('-'); break;
            case 0x4E: handler->OnKeyDown('+'); break;
            //Most other numpad are confilcting


            //Special
            case 0x1C: handler->OnKeyDown('\n'); break;             //Enter
            case 0x39: handler->OnKeyDown(' '); break;              //Space
            case 0x0F: handler->OnKeyDown('    '); break;           //Tab
            case 0x1D: handler->OnKeyDown('CRTL'); break;           //Left Control
            case 0x5B: handler->OnKeyDown('WIN'); break;            //Windows Key
            case 0x3A: Shift = !Shift; break;                       //Caps Lock
            case 0x38: handler->OnKeyDown('ALT'); break;            //Left ALT
            case 0x01: handler->OnKeyDown('ESC'); break;            //Escape
            case 0x37: handler->OnKeyDown('PRNT'); break;           //Print Screen
            case 0x46: handler->OnKeyDown('SCRL'); break;           //Scroll Lock
            case 0x52: handler->OnKeyDown('INST'); break;           //Insert
            case 0x47: handler->OnKeyDown('HOME'); break;           //Home
            case 0x53: handler->OnKeyDown('DEL'); break;            //Delete
            case 0x4F: handler->OnKeyDown('END'); break;            //End
            case 0x51: handler->OnKeyDown('PGDWN'); break;          //Page Down
            case 0x49: handler->OnKeyDown('PGUp'); break;           //Page Up
            case 0x48: handler->OnKeyDown('ARUP'); break;           //Arrow Up
            case 0x50: handler->OnKeyDown('ARDN'); break;           //Arrow Down
            case 0x4B: handler->OnKeyDown('ARLF'); break;           //Arrow Left
            case 0x4D: handler->OnKeyDown('ARRT'); break;           //Arrow Right
            case 0x0E: handler->OnKeyDown('BACKSPACE'); break;

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