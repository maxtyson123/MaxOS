//
// Created by 98max on 25/02/2025.
//

#ifndef MAXOS__PROCESSES_SCHEDULER_H
#define MAXOS__PROCESSES_SCHEDULER_H

#include <common/vector.h>
#include <system/cpu.h>
#include <processes/process.h>
#include <memory/memorymanagement.h>

namespace MaxOS{

  namespace processes{

    /**
     * @class Scheduler
     * @brief Schedules processes to run on the CPU
     */
    class Scheduler{

      private:
        common::Vector<Process*> m_processes;
        common::Vector<Thread*> m_threads;

        uint64_t m_current_thread_index;
        bool m_active;

        uint64_t m_ticks;
        uint64_t m_next_pid;
        uint64_t m_next_tid;

        static Scheduler* s_instance;


      public:
        Scheduler();
        ~Scheduler();

        system::cpu_status_t* schedule(system::cpu_status_t* status);

        uint64_t add_process(Process* process);
        uint64_t remove_process(Process* process);
        uint64_t add_thread(Thread* thread);

        static Scheduler* get_system_scheduler();
        static Process*   get_current_process();

        uint64_t get_ticks();
        void yield();
        void activate();

    };

  }

}

#endif // MAXOS__PROCESSES_SCHEDULER_H
