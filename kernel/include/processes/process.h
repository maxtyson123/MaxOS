//
// Created by 98max on 25/02/2025.
//

#ifndef MAXOS_PROCESSES_PROCESS_H
#define MAXOS_PROCESSES_PROCESS_H

#include <system/cpu.h>
#include <common/vector.h>
#include <common/map.h>
#include <common/string.h>
#include <stdint.h>
#include <memory/virtual.h>
#include <memory/memorymanagement.h>
#include <memory/memoryIO.h>
#include <processes/elf.h>
#include <processes/resource.h>


namespace MaxOS
{
    namespace processes
    {

        typedef enum class ThreadState{
            NEW,
            RUNNING,
            READY,
            SLEEPING,
            WAITING,
            STOPPED
        } thread_state_t;


        // Forward declaration
        class Process;

        /**
         * @class Thread
         * @brief The execution context of a sub-process thread
         */
        class Thread{

            private:

              uintptr_t m_stack_pointer;
              uintptr_t m_tss_stack_pointer;

              char m_sse_save_region[512] __attribute__((aligned(16)));

              static const uint64_t s_stack_size = 0x10000;

            public:
              Thread(void (*_entry_point)(void *) , void* args, int arg_amount, Process* parent);
              ~Thread();

	          void sleep(size_t milliseconds);

              uint64_t tid;
              uint64_t parent_pid;

              system::cpu_status_t* execution_state;
              thread_state_t thread_state;

              size_t ticks;
              size_t wakeup_time;

              [[nodiscard]] uintptr_t tss_pointer() const { return m_tss_stack_pointer; }

              void save_sse_state();
              void restore_sse_state();
        };

        /**
         * @class Process
         * @brief A process that can be scheduled by the Scheduler, wraps & manages threads as well as its own address space and resources.
         */
        class Process
        {

            private:
              common::Vector<Thread*> m_threads;

              uint64_t m_pid = 0;
			  common::Spinlock m_lock;

            public:
                Process(const string& name, bool is_kernel = false);
                Process(const string& name, void (*_entry_point)(void *), void *args, int arg_amount, bool is_kernel = false);
                Process(const string& name, void *args, int arg_amount, Elf64* elf, bool is_kernel = false);
                ~Process();

                common::Vector<Thread*> threads();
                void add_thread(Thread* thread);
                void remove_thread(uint64_t tid);

                void set_pid(uint64_t pid);
                uint64_t pid() const;
                uint64_t total_ticks();

                bool is_kernel;

                string name;
				string working_directory = "/";

                memory::MemoryManager* memory_manager = nullptr;
				ResourceManager resource_manager;


        };
    }
}

#endif // MAXOS_PROCESSES_PROCESS_H
