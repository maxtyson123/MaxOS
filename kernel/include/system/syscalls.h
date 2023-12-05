//
// Created by 98max on 27/10/2022.
//

#ifndef MAXOS_SYSTEM_SYSCALLS_H
#define MAXOS_SYSTEM_SYSCALLS_H

#include <stdint.h>
#include <hardwarecommunication/interrupts.h>

namespace maxOS{
    namespace system{
        class SyscallHandler : hardwarecommunication::InterruptHandler{

        public:
            SyscallHandler(hardwarecommunication::InterruptManager* interruptManager, uint8_t interruptNumber);
            ~SyscallHandler();

            virtual void HandleInterrupt();

        };

    }

}

#endif //MAXOS_SYSTEM_SYSCALLS_H
