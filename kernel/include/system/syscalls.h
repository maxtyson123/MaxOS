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
#include <system/syscalls.h>	// TODO: Rename / make clear that this references the system lib


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
            uint64_t return_value;
            cpu_status_t* return_state;
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

			inline static common::Spinlock s_lock = {};

          public:
              SyscallManager();
              ~SyscallManager();

              cpu_status_t* handle_interrupt(cpu_status_t* esp) final;

              void set_syscall_handler(::syscore::SyscallType syscall, syscall_func_t handler);
              void remove_syscall_handler(::syscore::SyscallType syscall);

              // Syscalls (TODO: Very c style, should be made class based that automatically registers)
              static syscall_args_t* syscall_close_process(syscall_args_t* args);
              static syscall_args_t* syscall_klog(syscall_args_t* args);
              static syscall_args_t* syscall_allocate_memory(syscall_args_t* args);
              static syscall_args_t* syscall_free_memory(syscall_args_t* args);
			  static syscall_args_t* syscall_resource_create(syscall_args_t* args);
			  static syscall_args_t* syscall_resource_open(syscall_args_t* args);
			  static syscall_args_t* syscall_resource_close(syscall_args_t* args);
			  static syscall_args_t* syscall_resource_write(syscall_args_t* args);
			  static syscall_args_t* syscall_resource_read(syscall_args_t* args);
              static syscall_args_t* syscall_thread_yield(syscall_args_t* args);
              static syscall_args_t* syscall_thread_sleep(syscall_args_t* args);
              static syscall_args_t* syscall_thread_close(syscall_args_t* args);
          };

    }

}

#endif //MAXOS_SYSTEM_SYSCALLS_H
