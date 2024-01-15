//
// Created by 98max on 27/10/2022.
//

#include <system/syscalls.h>

using namespace MaxOS;
using namespace MaxOS::common;
using namespace MaxOS::hardwarecommunication;
using namespace MaxOS::system;

///__Handler__///

SyscallHandler::SyscallHandler(InterruptManager*interrupt_manager, uint8_t interrupt_number)
:    InterruptHandler(interrupt_number + interrupt_manager->hardware_interrupt_offset(), interrupt_manager)
{
}

SyscallHandler::~SyscallHandler()
{
}

/**
 * @brief Handles the interrupt for a system call
 *
 * @param esp The stack frame
 */
void SyscallHandler::handle_interrupt()
{
    // TODO: Get the CPU state from the stack frame (maybe make a cpu driver to get the state)
    return;
}

// TODO: Implement the system calls