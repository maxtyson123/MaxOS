//
// Created by 98max on 9/10/2022.
//

#include "keyboard.h"

KeyboardDriver::KeyboardDriver(InterruptManager* manager)
: InterruptHandler(0x21, manager),  //0x21 is keyboard object, pass the manager paramerter to the base object
  dataPort(0x60),
  commandPort(0x64)
{
    while (commandPort.Read() & 0x1)    //Wait for user to stop pressing key (this is for the start up key eg. hold 'F12' for boot menu or hold 'del' for bios )
        dataPort.Read();
    commandPort.Write(0xAE);                            //Tell: PIC to send keyboard interrupt [or] tell keyboard to send interrupts to PIC
    commandPort.Write(0x20);                            //Tell: get current state
    uint8_t status = (dataPort.Read() | 1)  & ~ 0x10;        //Read current state then set rightmost bit to 1 becuase this will be the new state and clear the bit
    commandPort.Write(0x60);                            //Tell: change current state
    dataPort.Write(status);                             //Write back the current state

    dataPort.Write(0xF4);                               //Final Activation of keyboard
}
KeyboardDriver::~KeyboardDriver(){

}

void printf(char* str, bool clearLine = false); //Forward declaration

uint32_t KeyboardDriver::HandleInterrupt(uint32_t esp){

    uint8_t key = dataPort.Read();
    if(key < 0x80){             //Interrupts 0x80 onwards are just for keyrelases, therefore we can ignore them
        switch (key) {
            //Initializers that can be ignored
            case 0x45:
                break;
            case 0xFA:
                break;
            case 0xC5:
                break;

            //First Row
            case 0x29: printf("`"); break;  //Shift ~
            case 0x02: printf("1"); break;  //Shift !
            case 0x03: printf("2"); break;  //Shift @
            case 0x04: printf("3"); break;  //Shift #
            case 0x05: printf("4"); break;  //Shift $
            case 0x06: printf("5"); break;  //Shift %
            case 0x07: printf("6"); break;  //Shift ^
            case 0x08: printf("7"); break;  //Shift &
            case 0x09: printf("8"); break;  //Shift *
            case 0x0A: printf("9"); break;  //Shift (
            case 0x0B: printf("0"); break;  //Shift )
            case 0x0C: printf("-"); break;  //Shift _
            case 0x0D: printf("="); break;  //Shift +

            //Second Row
            case 0x0F: printf("TAB"); break;
            case 0x10: printf("q"); break;
            case 0x11: printf("w"); break;
            case 0x12: printf("e"); break;
            case 0x13: printf("r"); break;
            case 0x14: printf("t"); break;
            case 0x15: printf("y"); break;
            case 0x16: printf("u"); break;
            case 0x17: printf("i"); break;
            case 0x18: printf("o"); break;
            case 0x19: printf("p"); break;
            case 0x1A: printf("["); break;      //Shift {
            case 0x1B: printf("]"); break;      //Shift {

            //Third Row
            case 0x3A: printf("CAPS LOCK"); break;
            case 0x1E: printf("a"); break;
            case 0x1F: printf("s"); break;
            case 0x20: printf("d"); break;
            case 0x21: printf("f"); break;
            case 0x22: printf("g"); break;
            case 0x23: printf("h"); break;
            case 0x24: printf("j"); break;
            case 0x25: printf("k"); break;
            case 0x26: printf("l"); break;
            case 0x27: printf(";"); break;  //Shift :
            case 0x28: printf("'"); break;  //Shift "
            case 0x2B: printf("\\"); break; //Shift |

            //Row Four
            case 0x2A: printf("SHIFT"); break;
            case 0x2C: printf("z"); break;
            case 0x2D: printf("x"); break;
            case 0x2E: printf("c"); break;
            case 0x2F: printf("v"); break;
            case 0x30: printf("b"); break;
            case 0x31: printf("n"); break;
            case 0x32: printf("m"); break;
            case 0x33: printf(","); break;  //Shift <
            case 0x34: printf("."); break;  //Shift >
            case 0x35: printf("/"); break;  //Shift ?

            //Special
            case 0x1C: printf("\n"); break;     //Enter
            case 0x39: printf(" "); break;      //Space
            case 0x1D: printf("CRTL"); break;   //Left Control
            case 0x5B: printf("WIN"); break;    //Windows Key
            case 0x38: printf("ALT"); break;    //Left ALT
            case 0x0E: printf("BACKSPACE"); break;


            default:
                char* foo = "Keyboard 0x00";
                char* hex = "0123456789ABCDEF";

                foo[11] = hex[(key >> 4) & 0xF];
                foo[12] = hex[key & 0xF];
                printf(foo);
                break;

        }
    }



    return esp;
}