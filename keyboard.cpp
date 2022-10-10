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
    static bool Shift = false;


  switch (key) {
            //Initializers that can be ignored
            case 0x45:
                break;
            case 0xFA:
                break;
            case 0xC5:
                break;

            //First Row
            case 0x29: (Shift) ? printf("~") : printf("`"); break;
            case 0x02: (Shift) ? printf("!") : printf("1"); break;
            case 0x03: (Shift) ? printf("@") : printf("2"); break;
            case 0x04: (Shift) ? printf("#") : printf("3"); break;
            case 0x05: (Shift) ? printf("$") : printf("4"); break;
            case 0x06: (Shift) ? printf("%") : printf("5"); break;
            case 0x07: (Shift) ? printf("^") : printf("6"); break;
            case 0x08: (Shift) ? printf("&") : printf("7"); break;
            case 0x09: (Shift) ? printf("*") : printf("8"); break;
            case 0x0A: (Shift) ? printf("(") : printf("9"); break;
            case 0x0B: (Shift) ? printf(")") : printf("0"); break;
            case 0x0C: (Shift) ? printf("_") : printf("-"); break;
            case 0x0D: (Shift) ? printf("+") : printf("="); break;

            //Second Row
            case 0x10: (Shift) ? printf("Q") : printf("q"); break;
            case 0x11: (Shift) ? printf("W") : printf("w"); break;
            case 0x12: (Shift) ? printf("E") : printf("e"); break;
            case 0x13: (Shift) ? printf("R") : printf("r"); break;
            case 0x14: (Shift) ? printf("T") : printf("t"); break;
            case 0x15: (Shift) ? printf("Y") : printf("y"); break;
            case 0x16: (Shift) ? printf("U") : printf("u"); break;
            case 0x17: (Shift) ? printf("I") : printf("i"); break;
            case 0x18: (Shift) ? printf("O") : printf("o"); break;
            case 0x19: (Shift) ? printf("P") : printf("p"); break;
            case 0x1A: (Shift) ? printf("{") : printf("["); break;
            case 0x1B: (Shift) ? printf("}") : printf("]"); break;

            //Third Row
            case 0x1E: (Shift) ? printf("A") : printf("a"); break;
            case 0x1F: (Shift) ? printf("S") : printf("s"); break;
            case 0x20: (Shift) ? printf("D") : printf("d"); break;
            case 0x21: (Shift) ? printf("F") : printf("f"); break;
            case 0x22: (Shift) ? printf("G") : printf("g"); break;
            case 0x23: (Shift) ? printf("H") : printf("h"); break;
            case 0x24: (Shift) ? printf("J") : printf("j"); break;
            case 0x25: (Shift) ? printf("K") : printf("k"); break;
            case 0x26: (Shift) ? printf("L") : printf("l"); break;
            case 0x27: (Shift) ? printf(":") : printf(";"); break;
            case 0x28: (Shift) ? printf("\"") : printf("'"); break;
            case 0x2B: (Shift) ? printf("|") : printf("\\"); break;

            //Row Four
            case 0x2C: (Shift) ? printf("Z") : printf("z"); break;
            case 0x2D: (Shift) ? printf("X") : printf("x"); break;
            case 0x2E: (Shift) ? printf("C") : printf("c"); break;
            case 0x2F: (Shift) ? printf("V") : printf("v"); break;
            case 0x30: (Shift) ? printf("B") : printf("b"); break;
            case 0x31: (Shift) ? printf("N") : printf("n"); break;
            case 0x32: (Shift) ? printf("M") : printf("m"); break;
            case 0x33: (Shift) ? printf("<") : printf(","); break;
            case 0x34: (Shift) ? printf(">") : printf("."); break;
            case 0x35: (Shift) ? printf("?") : printf("/"); break;

            //Special
            case 0x1C: printf("\n"); break;     //Enter
            case 0x39: printf(" "); break;      //Space
            case 0x0F: printf("    "); break;   //Tab
            case 0x1D: printf("CRTL"); break;   //Left Control
            case 0x5B: printf("WIN"); break;    //Windows Key
            case 0x3A: Shift = !Shift; break;       //Caps Lock
            case 0x38: printf("ALT"); break;    //Left ALT
            case 0x0E: printf("BACKSPACE"); break;

            //   Left       Right
            case 0x2A: case 0x36: Shift = !Shift;  break;         //Shift Onpres
            case 0xAA: case 0xB6: Shift = !Shift;  break;        //Shift Onrelease


            ///TODO:
            ///Funct keys F1 etc.
            ///Arrow Keys
            ///Numpad


            default:
                if(key < 0x80) {              //Interrupts 0x80 onwards are just for keyrelease, therefore we can ignore printingthem
                    char *foo = "Keyboard 0x00";
                    char *hex = "0123456789ABCDEF";

                    foo[11] = hex[(key >> 4) & 0xF];
                    foo[12] = hex[key & 0xF];
                    printf(foo);
                }
                break;

    }



    return esp;
}