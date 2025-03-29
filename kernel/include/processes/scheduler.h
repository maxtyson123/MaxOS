//
// Created by 98max on 25/02/2025.
//

#ifndef MAXOS__PROCESSES_SCHEDULER_H
#define MAXOS__PROCESSES_SCHEDULER_H

#include <common/vector.h>
#include <system/cpu.h>
#include <hardwarecommunication/interrupts.h>
#include <processes/process.h>
#include <memory/memorymanagement.h>
#include <processes/ipc.h>

namespace MaxOS{

  namespace processes{

    /**
     * @class Scheduler
     * @brief Schedules processes to run on the CPU
     */
     class Scheduler : public hardwarecommunication::InterruptHandler {

      private:
        common::Vector<Process*> m_processes;
        common::Vector<Thread*> m_threads;

        uint64_t m_current_thread_index;
        bool m_active;

        uint64_t m_ticks;
        uint64_t m_next_pid;
        uint64_t m_next_tid;

        static Scheduler* s_instance;
        static const uint64_t s_ticks_per_event = { 3 };

        IPC* m_ipc;

      public:
        Scheduler(hardwarecommunication::InterruptManager* interrupt_manager);
        ~Scheduler();


        system::cpu_status_t* handle_interrupt(system::cpu_status_t* status) final;
        system::cpu_status_t* schedule(system::cpu_status_t* status);

        system::cpu_status_t* schedule_next(system::cpu_status_t* status);
        system::cpu_status_t* yield();

        uint64_t add_process(Process* process);
        uint64_t remove_process(Process* process);
        system::cpu_status_t* force_remove_process(Process* process);
        uint64_t add_thread(Thread* thread);

        static Scheduler* get_system_scheduler();
        static Process*   get_current_process();
        static Process*   get_process(uint64_t pid);
        static Thread*    get_current_thread();
        static Thread*    get_thread(uint64_t tid);
        static IPC*       get_ipc();

        uint64_t get_ticks();

        void load_multiboot_elfs(system::Multiboot* multiboot);

        void activate();
        void deactivate();
    };

  }

}

#endif // MAXOS__PROCESSES_SCHEDULER_H
