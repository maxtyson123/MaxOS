//
// Created by 98max on 27/10/2022.
//

#ifndef MAXOS_SYSTEM_SYSCALLS_H
#define MAXOS_SYSTEM_SYSCALLS_H

#include <stdint.h>
#include <hardwarecommunication/interrupts.h>
#include <stddef.h>

namespace maxOS{
    namespace system{

        /**
         * @class SyscallHandler
         * @brief Handles system calls
         */
        class SyscallHandler : hardwarecommunication::InterruptHandler{

        public:
            SyscallHandler(hardwarecommunication::InterruptManager*interrupt_manager, uint8_t interrupt_number);
            ~SyscallHandler();

            virtual void handle_interrupt();

        };

    }

}

#endif //MAXOS_SYSTEM_SYSCALLS_H
