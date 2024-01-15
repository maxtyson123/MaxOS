//
// Created by 98max on 10/18/2022.
//
#include <system/multitasking.h>

using namespace MaxOS;
using namespace MaxOS::system;


///__TASK__

Task::Task(GlobalDescriptorTable *gdt, void entrypoint()) {


    //Set up stack
    m_cpu_state = (CPUState *)(m_stack + 4096 - sizeof(CPUState));

    //Set phony entries

    m_cpu_state-> eax = 0;
    m_cpu_state-> ebx = 0;
    m_cpu_state-> ecx = 0;
    m_cpu_state-> edx = 0;

    m_cpu_state-> esi = 0;
    m_cpu_state-> edi = 0;
    m_cpu_state-> ebp = 0;

    // Set up the function pointer
    m_cpu_state-> eip = (uint32_t)entrypoint;
    m_cpu_state-> cs = gdt->code_segment_selector();
    m_cpu_state-> eflags = 0x202;
}

Task::~Task() {

}

///__TASK MANAGER__

TaskManager::TaskManager() {

}

TaskManager::~TaskManager() {

}

/**
 * @brief Adds a task to the task manager
 *
 * @param task The task to add
 */
bool TaskManager::add_task(Task *task) {

    m_tasks.push_back(task);
    return true;

}

/**
 * @brief Switches to the next task
 *
 * @param cpuState The current CPU state
 * @return The new CPU state
 */
CPUState *TaskManager::schedule(CPUState *cpuState) {

    // If there's no tasks then don't schedule
    if(m_tasks.size() <= 0)
        return cpuState;

    // If there is a task running, save its state
    if(m_current_task >= 0)
      m_tasks[m_current_task] ->m_cpu_state = cpuState;

    // Switch to the next task (and loop back to the start if needed)
    if((uint32_t )++m_current_task >= m_tasks.size())
      m_current_task %= m_tasks.size();

    // Start the next task
    return m_tasks[m_current_task] ->m_cpu_state;

}