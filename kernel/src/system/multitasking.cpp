//
// Created by 98max on 10/18/2022.
//
#include <system/multitasking.h>

using namespace maxOS;
using namespace maxOS::common;
using namespace maxOS::system;


///__TASK__

Task::Task(GlobalDescriptorTable *gdt, void entrypoint()) {
    //              start of stack + size of stack - size of entry
    cpuState = (CPUState_Task*)(stack + 4096 - sizeof(CPUState_Task));

    //Set phony entries

    cpuState -> eax = 0;
    cpuState -> ebx = 0;
    cpuState -> ecx = 0;
    cpuState -> edx = 0;

    cpuState -> esi = 0;
    cpuState -> edi = 0;
    cpuState -> ebp = 0;

    /*
    cpuState -> gs = 0;
    cpuState -> fs = 0;
    cpuState -> es = 0;
    cpuState -> ds = 0;
    */

    //cpuState -> error = 0;

    // cpuState -> esp = ;  //commented out becuase its only for user space and different security levels

    cpuState -> eip = (uint32_t)entrypoint;         // Instruction pointer is set to entry point  (note: ignore error, it compiles)
    cpuState -> cs = gdt->CodeSegmentSelector();    // Offset of the code segment
    //state segment not used unless security
    cpuState -> eflags = 0x202;
}

Task::~Task() {

}

///__TASK MANAGER__

TaskManager::TaskManager() {

    //Default Values
    numTasks = 0;
    currentTask = -1;

}

TaskManager::~TaskManager() {

}

/**
 * @details Adds a task to the task manager
 *
 * @param task The task to add
 */
bool TaskManager::AddTask(Task *task) {

    if(numTasks >= 256){ //Array IS full
        return false;
    }

    tasks[numTasks++] = task;
    return true;

}

/**
 * @details Switches to the next task
 *
 * @param cpuState The current CPU state
 * @return The new CPU state
 */
CPUState_Task *TaskManager::Schedule(CPUState_Task *cpuState) {

    if(numTasks <= 0){      //If there's no tasks yet
        return cpuState;    //Restore old cpu state
    }

    if(currentTask >= 0){
        tasks[currentTask] -> cpuState = cpuState;   //Restore the old value (put task back into list of tasks)
    }

    if(++currentTask >= numTasks){ //If current task exceeds size of tasks
        currentTask %= numTasks;   //Restart from beginning
    }

    /*
    if(tasks[currentTask] -> killMe){
        tasks[currentTask] = tasks[numTasks];
        numTasks - 1;
    }
     */

    return tasks[currentTask] -> cpuState;

}
