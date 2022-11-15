//
// Created by 98max on 10/18/2022.
//

#ifndef MAXOS_MULTITASKING_H
#define MAXOS_MULTITASKING_H

#include <common/types.h>
#include <gdt.h>

namespace maxOS{


    struct CPUState_Task
    {
        //Pushed by kernel in interuptstubs

        common::uint32_t eax;           // Accumulating Register
        common::uint32_t ebx;           // Base Register
        common::uint32_t ecx;           // Counting Register
        common::uint32_t edx;           // Data Register

        common::uint32_t esi;           // Stack Index
        common::uint32_t edi;           // Data Index
        common::uint32_t ebp;           // Stack Base Pointer

        //Elements that have been pushed so far
        /*
        common::uint32_t gs;
        common::uint32_t fs;
        common::uint32_t es;
        common::uint32_t ds;
        */
        common::uint32_t error;         //One int for an error code

        common::uint32_t eip;           // Instruction Pointer
        common::uint32_t cs;            // Code Segment
        common::uint32_t eflags;        // Flags
        common::uint32_t esp;           // Stack Pointer
        common::uint32_t ss;            // Stack Segment
    } __attribute__((packed));


    class Task
    {
        friend class TaskManager;   //Allow TaskManger class to acess private values of this class
    private:
        common::uint8_t stack[4096]; //Allocate 4kb for this tasks stack
        CPUState_Task* cpuState;
    public:
        Task(GlobalDescriptorTable *gdt, void entrypoint());
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
        CPUState_Task* Schedule(CPUState_Task* cpuState);         //method that does the scheduling (round-robin [https://en.wikipedia.org/wiki/Round-robin_scheduling])s
    };

}

#endif //MAXOS_MULTITASKING_H
