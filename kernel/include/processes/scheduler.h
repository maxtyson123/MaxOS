//
// Created by 98max on 25/02/2025.
//

#ifndef MAXOS_PROCESSES_SCHEDULER_H
#define MAXOS_PROCESSES_SCHEDULER_H

#include <common/vector.h>
#include <system/cpu.h>
#include <hardwarecommunication/interrupts.h>
#include <processes/process.h>
#include <memory/memorymanagement.h>
#include <processes/ipc.h>

namespace MaxOS {
	namespace processes {

		class GlobalScheduler : public hardwarecommunication::InterruptHandler{

			private:
				inline static GlobalScheduler* s_instance = nullptr;
				bool m_active { false };

				GlobalResourceRegistry m_global_resource_registry = {};
				ResourceRegistry<SharedMemory> m_shared_memory_registry;
				ResourceRegistry<SharedMessageEndpoint> m_shared_messages_registry;

				common::Spinlock m_lock;

				uint64_t m_next_pid;
				uint64_t m_next_tid;

				common::Map<uint64_t, uint64_t> m_core_pids;
				common::Map<uint64_t, uint64_t> m_core_tids;

			public:
				GlobalScheduler(system::Multiboot& multiboot);
				~GlobalScheduler();

				static GlobalScheduler* system_scheduler();
				static Scheduler* core_scheduler();

				system::cpu_status_t* handle_interrupt(system::cpu_status_t* status) final;
				static system::cpu_status_t* yield(system::cpu_status_t* current);

				static void activate();
				static void deactivate();
				void balance();

				static void load_multiboot_elfs(system::Multiboot* multiboot);
				static void print_running_header();

				uint64_t add_process(Process* process);
				uint64_t add_thread(Thread* thread);

				uint64_t remove_process(Process* process);
				system::cpu_status_t* force_remove_process(Process* process);

				static Process* current_process();
				static Process* get_process(uint64_t pid);
				static uint64_t next_pid();

				static Thread* current_thread();
				static Thread* get_thread(uint64_t tid);
				static uint64_t next_tid();
		};

		constexpr size_t TICKS_PER_EVENT = 20;

		/**
		 * @class Scheduler
		 * @brief Schedules processes to run on the core via their threads
		 */
		class Scheduler {

			private:
				common::Vector<Process*> m_processes;
				common::Vector<Thread*> m_threads;

				uint64_t m_next_thread_index;
				bool m_active;

				uint64_t m_ticks;

				static system::cpu_status_t* load_process(Process* process, Thread* thread);

			public:
				Scheduler();
				~Scheduler();

				system::cpu_status_t* schedule(system::cpu_status_t* status);
				system::cpu_status_t* schedule_next(system::cpu_status_t* status);
				system::cpu_status_t* yield();

				uint64_t add_process(Process* process);
				uint64_t remove_process(Process* process);
				system::cpu_status_t* force_remove_process(Process* process);
				uint64_t add_thread(Thread* thread);

				Process* current_process();
				Process* get_process(uint64_t pid);
				uint64_t process_amount();

				Thread* current_thread();
				Thread* get_thread(uint64_t tid);
				uint64_t thread_amount();

				[[nodiscard]] uint64_t ticks() const;

				void activate();
				void deactivate();
		};

	}
}

#endif // MAXOS_PROCESSES_SCHEDULER_H
