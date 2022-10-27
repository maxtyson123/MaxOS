//
// Created by 98max on 27/10/2022.
//

#include <syscalls.h>

using namespace maxOS;
using namespace maxOS::common;
using namespace maxOS::hardwarecommunication;

void printf(char* str, bool clearLine = false); //Forward declaration
void printfHex(uint8_t key);                    //Forward declaration

SysCallHandler::SysCallHandler(InterruptManager *interruptManager, uint8_t interruptNumber)
: InterruptHandler(interruptNumber + interruptManager->HardwareInterruptOffset(), interruptManager)
{

}

SysCallHandler::~SysCallHandler() {

}

common::uint32_t SysCallHandler::HandleInterrupt(uint8_t interrupt, uint32_t esp) {

    CPUState* cpu = (CPUState*)esp;

    switch (cpu->eax) {
        case 4:
            printf((char*)cpu->ebx, false);
            break;


        default:
            break;
    }

    return esp;

}
