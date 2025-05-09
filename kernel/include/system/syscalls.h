//
// Created by 98max on 27/10/2022.
//

#ifndef MAXOS_SYSTEM_SYSCALLS_H
#define MAXOS_SYSTEM_SYSCALLS_H

#include <stdint.h>
#include <stddef.h>
#include <hardwarecommunication/interrupts.h>
#include <common/vector.h>
#include <common/colour.h>
#include <memory/memorymanagement.h>
#include <processes/scheduler.h>


namespace MaxOS{
    namespace system{

        // Forward declaration
        class SyscallManager;

        /// DO NOT REARRANGE ONLY APPEND TO
        enum SyscallType{
            CLOSE_PROCESS,
            KLOG,
            CREATE_SHARED_MEMORY,
            OPEN_SHARED_MEMORY,
            ALLOCATE_MEMORY,
            FREE_MEMORY,
            CREATE_IPC_ENDPOINT,
            SEND_IPC_MESSAGE,
            REMOVE_IPC_ENDPOINT,
            THREAD_YIELD,
            THREAD_SLEEP,
            THREAD_CLOSE,
        };

        typedef struct SyscallArguments{
            uint64_t arg0;
            uint64_t arg1;
            uint64_t arg2;
            uint64_t arg3;
            uint64_t arg4;
            uint64_t arg5;
            uint64_t return_value;
            system::cpu_status_t* return_state;
        } syscall_args_t;

        // Could use a class based response but a single class might want multiple handlers e.g. fs
        typedef syscall_args_t* (*syscall_func_t)(syscall_args_t* args);

        /**
         * @class SyscallManager
         * @brief Provides an API for userspace applications to interact with the kernel
         */
        class SyscallManager : hardwarecommunication::InterruptHandler{

          protected:
            syscall_func_t m_syscall_handlers[256] = {};
            syscall_args_t* m_current_args;


          public:
              SyscallManager();
              ~SyscallManager();

              system::cpu_status_t* handle_interrupt(system::cpu_status_t* esp) final;

              void set_syscall_handler(uint8_t syscall, syscall_func_t handler);
              void remove_syscall_handler(uint8_t syscall);


              // Syscalls
              static system::syscall_args_t* syscall_close_process(system::syscall_args_t* args);
              static system::syscall_args_t* syscall_klog(system::syscall_args_t* args);
              static system::syscall_args_t* syscall_create_shared_memory(system::syscall_args_t* args);
              static system::syscall_args_t* syscall_open_shared_memory(system::syscall_args_t* args);
              static system::syscall_args_t* syscall_allocate_memory(system::syscall_args_t* args);
              static system::syscall_args_t* syscall_free_memory(system::syscall_args_t* args);
              static system::syscall_args_t* syscall_create_ipc_endpoint(system::syscall_args_t* args);
              static system::syscall_args_t* syscall_send_ipc_message(system::syscall_args_t* args);
              static system::syscall_args_t* syscall_remove_ipc_endpoint(system::syscall_args_t* args);
              static system::syscall_args_t* syscall_thread_yield(system::syscall_args_t* args);
              static system::syscall_args_t* syscall_thread_sleep(system::syscall_args_t* args);
              static system::syscall_args_t* syscall_thread_close(system::syscall_args_t* args);
          };

    }

}

#endif //MAXOS_SYSTEM_SYSCALLS_H
