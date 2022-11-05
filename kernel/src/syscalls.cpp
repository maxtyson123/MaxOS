//
// Created by 98max on 27/10/2022.
//

#include <syscalls.h>

using namespace maxOS;
using namespace maxOS::common;
using namespace maxOS::hardwarecommunication;

void printf(char* str, bool clearLine = false); //Forward declaration
void printfHex(uint8_t key);                    //Forward declaration


SyscallHandler::SyscallHandler(InterruptManager* interruptManager, uint8_t InterruptNumber)
        :    InterruptHandler(InterruptNumber  + interruptManager->HardwareInterruptOffset(), interruptManager)
{
}

SyscallHandler::~SyscallHandler()
{
}

uint32_t SyscallHandler::HandleInterrupt(uint32_t esp)
{
    CPUState* cpu = (CPUState*)esp;


    switch(cpu->eax)
    {
        case 4:
            printf((char*)cpu->ebx);
            break;

        default:
            break;
    }


    return esp;
}

