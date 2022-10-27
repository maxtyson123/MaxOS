//
// Created by 98max on 10/18/2022.
//
#include <multitasking.h>

using namespace maxOS;
using namespace maxOS::common;


///__TASK__

Task::Task(GlobalDescriptorTable *gdt, void entrypoint()) {
    //              start of stack + size of stack - size of entry
    cpuState = (CPUState*)(stack + 4096 - sizeof(CPUState));

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

bool TaskManager::AddTask(Task *task) {

    if(numTasks >= 256){ //Array IS full
        return false;
    }

    tasks[numTasks++] = task;
    return true;

}

CPUState *TaskManager::Schedule(CPUState *cpuState) {

    if(numTasks <= 0){      //If theres no tasks yet
        return cpuState;    //Restore old cpu state
    }

    if(currentTask >= 0){
        tasks[currentTask] -> cpuState = cpuState;   //Resetore the old value (put task back into list of tasks)
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
