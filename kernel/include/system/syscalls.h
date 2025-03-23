//
// Created by 98max on 27/10/2022.
//

#ifndef MAXOS_SYSTEM_SYSCALLS_H
#define MAXOS_SYSTEM_SYSCALLS_H

#include <stdint.h>
#include <stddef.h>
#include <hardwarecommunication/interrupts.h>
#include <common/vector.h>
#include <processes/scheduler.h>
#include <common/colour.h>


namespace MaxOS{
    namespace system{

        // Forward declaration
        class SyscallManager;


        typedef struct SyscallArguments{
            uint64_t arg0;
            uint64_t arg1;
            uint64_t arg2;
            uint64_t arg3;
            uint64_t arg4;
            uint64_t arg5;
        } syscall_args_t;

        // Could use a class based response but a single class might want multiple handlers eg fs
        typedef syscall_args_t* (*syscall_func_t)(syscall_args_t* args);

        /**
         * @class SyscallManager
         * @brief Handles system calls
         */
        class SyscallManager : hardwarecommunication::InterruptHandler{

          protected:
            syscall_func_t m_syscall_handlers[256];
            syscall_args_t* m_current_args;


          public:
              SyscallManager(hardwarecommunication::InterruptManager*interrupt_manager);
              ~SyscallManager();

              system::cpu_status_t* handle_interrupt(system::cpu_status_t* esp) final;

              void set_syscall_handler(uint8_t syscall, syscall_func_t handler);
              void remove_syscall_handler(uint8_t syscall);

              // General Syscalls
              static syscall_args_t* syscall_write(syscall_args_t* args);

          };

    }

}

#endif //MAXOS_SYSTEM_SYSCALLS_H
