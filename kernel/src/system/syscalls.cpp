//
// Created by 98max on 27/10/2022.
//

#include <system/syscalls.h>

using namespace maxOS;
using namespace maxOS::common;
using namespace maxOS::hardwarecommunication;
using namespace maxOS::system;

void printf(char* str, bool clearLine = false); //Forward declaration
void printfHex(uint8_t key);                    //Forward declaration


SyscallHandler::SyscallHandler(InterruptManager* interruptManager, uint8_t InterruptNumber)
        :    InterruptHandler(InterruptNumber  + interruptManager->HardwareInterruptOffset(), interruptManager)
{
}

SyscallHandler::~SyscallHandler()
{
}

/**
 * @details Handles the interrupt for a system call
 * @param esp The stack frame
 */
uint32_t SyscallHandler::HandleInterrupt(uint32_t esp)
{
    CPUState_Thread* cpu = (CPUState_Thread*)esp;


    switch(cpu->eax)
    {
        case 4:                                 //Write
            printf((char*)cpu->ebx);
            break;

        case 37:                                //Get PID

            break;

        default:
            break;
    }


    return esp;
}

