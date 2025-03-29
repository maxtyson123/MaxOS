//
// Created by 98max on 25/02/2025.
//

#ifndef MAXOS_PROCESSES_PROCESS_H
#define MAXOS_PROCESSES_PROCESS_H

#include <system/cpu.h>
#include <common/vector.h>
#include <common/string.h>
#include <stdint.h>
#include <memory/virtual.h>
#include <memory/memorymanagement.h>
#include <memory/memoryIO.h>
#include <processes/elf.h>


namespace MaxOS
{
    namespace processes
    {

        typedef enum ThreadState{
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

              static const uint64_t s_stack_size = 0x10000;

            public:
              Thread(void (*_entry_point)(void *) , void* args, int arg_amount, Process* parent);
              ~Thread();

              system::cpu_status_t* sleep(size_t milliseconds);

              uint64_t tid;
              uint64_t parent_pid;

              system::cpu_status_t* execution_state;
              thread_state_t thread_state;

              size_t ticks;
              size_t wakeup_time;

              uintptr_t get_tss_pointer() const { return m_tss_stack_pointer; }


        };

        /**
         * @class Process
         * @brief A process that can be scheduled by the Scheduler, wraps & manages threads as well as its own address space and resources.
         */
        class Process
        {

            private:

              common::Vector<uint16_t> m_resource_ids;
              common::Vector<Thread*> m_threads;

              memory::VirtualMemoryManager* m_virtual_memory_manager;
              uint64_t m_pid;

            public:
                Process(string name, void (*_entry_point)(void *), void *args, int arg_amount, bool is_kernel = false);
                Process(string name, void *args, int arg_amount, Elf64* elf, bool is_kernel = false);
                ~Process();

                void set_up();

                common::Vector<Thread*> get_threads();
                void add_thread(Thread* thread);
                void remove_thread(uint64_t tid);

                void set_pid(uint64_t pid);
                uint64_t get_pid();
                uint64_t get_total_ticks();

                bool is_kernel;

                string name;
                memory::MemoryManager* memory_manager;


        };
    }
}

#endif // MAXOS_PROCESSES_PROCESS_H
