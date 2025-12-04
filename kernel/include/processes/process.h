/**
 * @file process.h
 * @brief Defines Process and Thread classes for managing processes and their execution contexts
 *
 * @date 25th February 2025
 * @author Max Tyson
 */

#ifndef MAXOS_PROCESSES_PROCESS_H
#define MAXOS_PROCESSES_PROCESS_H

#include <system/cpu.h>
#include <vector.h>
#include <map.h>
#include <string.h>
#include <cstdint>
#include <memory/virtual.h>
#include <memory/memorymanagement.h>
#include <memory/memoryIO.h>
#include <processes/elf.h>
#include <processes/resource.h>

namespace MaxOS::processes {
	class Process;

	/**
	 * @enum ThreadState
	 * @brief The different operational states a thread can be in
	 *
	 * @typedef thread_state_t
	 * @brief Alias for ThreadState enum
	 */
	typedef enum class ThreadState {
		NEW,
		RUNNING,
		READY,
		SLEEPING,
		WAITING,
		STOPPED
	} thread_state_t;

	/// The size of the stack for each thread (4KB)
	constexpr size_t STACK_SIZE = 0x10000;

	/**
	 * @class Thread
	 * @brief The execution context of a sub-process thread
	 */
	class Thread {

		private:

			uintptr_t m_stack_pointer;
			uintptr_t m_tss_stack_pointer;

			char m_sse_save_region[512] __attribute__((aligned(16)));

		public:
			Thread(void (* _entry_point)(void*), void* args, int arg_amount, Process* parent);
			~Thread();

			void sleep(size_t milliseconds);

			uint64_t tid;                             ///< The thread ID
			uint64_t parent_pid;                      ///< The parent process ID

			system::cpu_status_t execution_state;     ///< The CPU state of the thread
			thread_state_t thread_state;              ///< The current state of the thread

			size_t ticks;                             ///< The number of ticks the thread has run for
			size_t wakeup_time;                       ///< The time at which the thread should wake up (if sleeping)

			[[nodiscard]] uintptr_t tss_pointer() const { return m_tss_stack_pointer; }    ///< Gets the stack pointer to use for the TSS when switching to this thread @return tss

			void save_sse_state();
			void restore_sse_state();

			void save_cpu_state();
	};

	/**
	 * @class Process
	 * @brief A process that can be scheduled by the Scheduler, wraps & manages threads as well as its own address space and resources.
	 */
	class Process {

		private:
			common::Vector<Thread*> m_threads;

			uint64_t m_pid = 0;
			common::Spinlock m_lock;

		public:
			explicit Process(const string& name, bool is_kernel = false);
			Process(const string& name, void (* _entry_point)(void*), void* args, int arg_amount, bool is_kernel = false);
			Process(const string& name, void* args, int arg_amount, ELF64* elf, bool is_kernel = false);
			~Process();

			common::Vector<Thread*> threads();
			void add_thread(Thread* thread);
			void remove_thread(uint64_t tid);

			void set_pid(uint64_t pid);
			[[nodiscard]] uint64_t pid() const;
			[[nodiscard]] uint64_t total_ticks();

			bool is_kernel;                                             ///< Whether this process is a kernel process

			string name;                                                ///< The name of the process
			string working_directory = "/";                             ///< The working directory of the process

			memory::MemoryManager* memory_manager = nullptr;            ///< The manager for memory used by this process
			ResourceManager resource_manager;                           ///< The manger for resources used by this process
	};
}

#endif // MAXOS_PROCESSES_PROCESS_H
