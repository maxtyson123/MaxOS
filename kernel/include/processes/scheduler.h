/**
 * @file scheduler.h
 * @brief Defines a GlobalScheduler and Scheduler for managing processes and threads
 *
 * @date 25th February 2025
 * @author Max Tyson
 */

#ifndef MAXOS_PROCESSES_SCHEDULER_H
#define MAXOS_PROCESSES_SCHEDULER_H

#include <libcommon/vector.h>

#include <system/cpu.h>
#include <hardwarecommunication/interrupts.h>
#include <processes/process.h>
#include <memory/memorymanagement.h>
#include <processes/ipc.h>
#include <processes/resources/scheduler_resources.h>


namespace MaxOS::processes {

	/**
	 * @class GlobalScheduler
	 * @brief The global scheduler that manages all processes and threads across all cores
	 */
	class GlobalScheduler : public hardwarecommunication::InterruptHandler {

		private:
			inline static GlobalScheduler* s_instance = nullptr;
			bool m_active = false;

			resources::GlobalResourceRegistry m_global_resource_registry = { };
			resources::ResourceRegistry<SharedMemory> m_shared_memory_registry;
			resources::ResourceRegistry<SharedMessageEndpoint> m_shared_messages_registry;
			resources::ThreadResourceRegistry m_thread_resource_registry;
			resources::ProcessResourceRegistry m_process_resource_registry;

			common::Spinlock m_lock;

			uint64_t m_next_pid;
			uint64_t m_next_tid;

			common::Map<uint64_t, uint64_t> m_core_pids;
			common::Map<uint64_t, uint64_t> m_core_tids;

		public:
			explicit GlobalScheduler(system::Multiboot& multiboot);
			~GlobalScheduler();

			static GlobalScheduler* system_scheduler();
			static Scheduler* core_scheduler();

			system::cpu_status_t* handle_interrupt(system::cpu_status_t* status) final;

			static void activate();
			static void deactivate();
			static bool is_active();

			void balance();
			system::Core* least_busy_core(bool check_threads=false);

			static void load_multiboot_elfs(system::Multiboot* multiboot);
			static void prepare_initrd(multiboot_tag_module* module);
			static void print_running_header();

			uint64_t add_process(Process* process, Scheduler* scheduler = nullptr);
			uint64_t add_thread(Thread* thread, Scheduler* scheduler = nullptr);

			static uint64_t remove_process(Process* process);
			static system::cpu_status_t* force_remove_process(Process* process);

			static Process* current_process();
			static Process* get_process(uint64_t pid);
			static uint64_t next_pid();

			static Thread* current_thread();
			static Thread* get_thread(uint64_t tid);
			static uint64_t next_tid();
	};

	/// Number of times the clock has to interrupt before switching to the next process/thread
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
			uintptr_t m_core_id;

			static system::cpu_status_t* load_process(Process* process, Thread* thread);

		public:
			Scheduler(uint64_t core_id);
			~Scheduler();

			system::cpu_status_t* schedule(system::cpu_status_t* cpu_state);
			system::cpu_status_t* schedule_next(system::cpu_status_t* status);

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
			[[nodiscard]] uint64_t core_id() const;

			void activate();
			void deactivate();
	};

}


#endif // MAXOS_PROCESSES_SCHEDULER_H
