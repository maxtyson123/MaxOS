/**
 * @file syscalls.h
 * @brief Defines the SyscallManager class for handling system calls from userspace applications
 *
 * @date 27th October 2022
 * @author Max Tyson
 *
 * @todo '<system/syscalls.h>' Rename / make clear that this references the system lib
 */

#ifndef MAXOS_SYSTEM_SYSCALLS_H
#define MAXOS_SYSTEM_SYSCALLS_H

#include <cstdint>
#include <cstddef>
#include <hardwarecommunication/interrupts.h>
#include <common/vector.h>
#include <common/colour.h>
#include <memory/memorymanagement.h>
#include <processes/scheduler.h>
#include <system/syscalls.h>


namespace MaxOS::system {

	// Forward declaration
	class SyscallManager;

	/**
	 * @struct SyscallArguments
	 * @brief The arguments passed to a syscall (simplified representation of values in registers)
	 *
	 * @typedef syscall_args_t
	 * @brief Alias for SyscallArguments struct
	 */
	typedef struct SyscallArguments {

		uint64_t arg0;                  ///< First argument (in rdi)
		uint64_t arg1;                  ///< Second argument (in rsi)
		uint64_t arg2;                  ///< Third argument (in rdx)
		uint64_t arg3;                  ///< Fourth argument (in rcx)
		uint64_t arg4;                  ///< Fifth argument (in r8)
		uint64_t arg5;                  ///< Sixth argument (in r9)
		uint64_t return_value;          ///< The return value of the syscall (in rax)
		cpu_status_t* return_state;     ///< The CPU state to return to after the syscall

	} syscall_args_t;

	/// @todo Could use a class based response but a single class might want multiple handlers e.g. fs
	typedef syscall_args_t* (* syscall_func_t)(syscall_args_t* args);

	/**
	 * @class SyscallManager
	 * @brief Provides an API for userspace applications to interact with the kernel
	 *
	 * @todo Very c style, should be made class based that automatically registers
	 */
	class SyscallManager : hardwarecommunication::InterruptHandler {

		private:
			syscall_func_t m_syscall_handlers[256] = { };

			inline static common::Spinlock s_lock = { };

		public:
			SyscallManager();
			~SyscallManager();

			cpu_status_t* handle_interrupt(cpu_status_t* esp) final;

			void set_syscall_handler(::syscore::SyscallType syscall, syscall_func_t handler);
			void remove_syscall_handler(::syscore::SyscallType syscall);

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


#endif //MAXOS_SYSTEM_SYSCALLS_H
