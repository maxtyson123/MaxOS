//
// Created by 98max on 27/10/2022.
//

#ifndef MAXOS_SYSCALLS_H
#define MAXOS_SYSCALLS_H

#include <common/types.h>
#include <hardwarecommunication/interrupts.h>

namespace maxOS{

    class SysCallHandler : hardwarecommunication::InterruptHandler{

        public:
            SysCallHandler(hardwarecommunication::InterruptManager* interruptManager, common::uint8_t interruptNumber);
            ~SysCallHandler();

            virtual common::uint32_t HandleInterrupt(common::uint8_t interrupt, common::uint32_t esp);

    };



}

#endif //MAXOS_SYSCALLS_H
