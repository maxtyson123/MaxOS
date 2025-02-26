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

        class Thread{

            private:

              uintptr_t m_stack_pointer;

              static const uint64_t s_stack_size = 0x10000;

            public:
              Thread(void (*_entry_point)(void *) , void* args, Process* parent);
              ~Thread();

              void sleep(size_t milliseconds);

              uint64_t tid;
              uint64_t parent_pid;

              system::cpu_status_t* execution_state;
              thread_state_t thread_state;

              size_t ticks;
              size_t wakeup_time;


        };

        /**
         * @class Process
         * @brief A process that can be scheduled by the Scheduler
         */
        class Process
        {

            private:

              common::Vector<uint16_t> m_resource_ids;
              common::Vector<Thread*> m_threads;

              memory::VirtualMemoryManager* m_memory_manager;
              uint64_t m_pid;

            public:
                Process(string name, void (*_entry_point)(void *), void *args);
                //Process(string name, void *args, ELF STUFF);  // Task from ELF
                ~Process();


                common::Vector<Thread*> get_threads();
                void add_thread(Thread* thread);
                void remove_thread(uint64_t tid);

                void set_pid(uint64_t pid);
                uint64_t get_pid();
                uint64_t* get_page_directory();

                string name;


        };
    }
}

#endif // MAXOS_PROCESSES_PROCESS_H
