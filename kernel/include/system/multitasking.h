//
// Created by 98max on 10/18/2022.
//

#ifndef MAXOS_SYSTEM_MULTITASKING_H
#define MAXOS_SYSTEM_MULTITASKING_H

#include <common/vector.h>
#include <stdint.h>
#include <system/gdt.h>

namespace maxOS{

    namespace system{

        /**
         * @struct CPUState
         * @brief Stores the state of the CPU registers
         */
        struct CPUState {
            //Pushed by kernel in interupt_stubs

            uint32_t eax;
            uint32_t ebx;
            uint32_t ecx;
            uint32_t edx;

            uint32_t esi;
            uint32_t edi;
            uint32_t ebp;

            uint32_t error;

            uint32_t eip;
            uint32_t cs;
            uint32_t eflags;
            uint32_t esp;
            uint32_t ss;
        } __attribute__((packed));

        /**
         * @class Task
         * @brief A task that can be scheduled by the TaskManager
         */
        class Task
        {
            friend class TaskManager;

            private:
                uint8_t m_stack[4096];
                CPUState * m_cpu_state;
            public:
                Task(system::GlobalDescriptorTable *gdt, void entrypoint());
                ~Task();
        };

        /**
         * @class TaskManager
         * @brief Manages the scheduling of m_tasks
         */
        class TaskManager
        {
            private:
                common::Vector<Task*> m_tasks;
                int m_current_task { -1 };
            public:
                TaskManager();
                ~TaskManager();
                bool add_task(Task* task);
                CPUState * schedule(CPUState * cpuState);
        };

    }



}

#endif //MAXOS_SYSTEM_MULTITASKING_H
