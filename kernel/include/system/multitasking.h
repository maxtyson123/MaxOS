//
// Created by 98max on 10/18/2022.
//

#ifndef MAXOS_SYSTEM_MULTITASKING_H
#define MAXOS_SYSTEM_MULTITASKING_H

#include <stdint.h>
#include <system/gdt.h>

namespace maxOS{

    namespace system{

        struct CPUState_Task
        {
            //Pushed by kernel in interuptstubs

            uint32_t eax;           // Accumulating Register
            uint32_t ebx;           // Base Register
            uint32_t ecx;           // Counting Register
            uint32_t edx;           // Data Register

            uint32_t esi;           // Stack Index
            uint32_t edi;           // Data Index
            uint32_t ebp;           // Stack Base Pointer

            //Elements that have been pushed so far
            /*
            uint32_t gs;
            uint32_t fs;
            uint32_t es;
            uint32_t ds;
            */
            uint32_t error;         //One int for an error code

            uint32_t eip;           // Instruction Pointer
            uint32_t cs;            // Code Segment
            uint32_t eflags;        // Flags
            uint32_t esp;           // Stack Pointer
            uint32_t ss;            // Stack Segment
        } __attribute__((packed));


        class Task
        {
            friend class TaskManager;   //Allow TaskManger class to acess private values of this class
        private:
            uint8_t stack[4096]; //Allocate 4kb for this tasks stack
            CPUState_Task* cpuState;
        public:
            Task(system::GlobalDescriptorTable *gdt, void entrypoint());
            ~Task();
            bool killMe = false;
        };


        class TaskManager
        {
        private:
            Task* tasks[256];
            int numTasks;
            int currentTask;            //Index of currently active task is used when the processor needs to go back to this task
        public:
            TaskManager();
            ~TaskManager();
            bool AddTask(Task* task);
            CPUState_Task* Schedule(CPUState_Task* cpuState);         //function that does the scheduling (round-robin [https://en.wikipedia.org/wiki/Round-robin_scheduling])s
        };

    }



}

#endif //MAXOS_SYSTEM_MULTITASKING_H
