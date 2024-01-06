//
// Created by 98max on 15/11/2022.
//
#include <system/multithreading.h>

using namespace maxOS;
using namespace maxOS::system;



Thread::Thread() {

}

Thread::~Thread()
{
}

void Thread::init(void entrypoint())
{

  // Set up stack
  m_cpu_state = (CPUState*)(m_stack + 4096 - sizeof(CPUState));

  m_cpu_state->eax = 0;
  m_cpu_state->ebx = 0;
  m_cpu_state->ecx = 0;
  m_cpu_state->edx = 0;

  m_cpu_state->esi = 0;
  m_cpu_state->edi = 0;
  m_cpu_state->ebp = 0;

  // Set up function pointer
  m_cpu_state->eip = (uint32_t)entrypoint;
  m_cpu_state->eflags = 0x202;
  m_yield_status = false;
}

ThreadManager::ThreadManager()
{
}

ThreadManager::ThreadManager(GlobalDescriptorTable *gdt)
{ m_gdt = gdt;
}

ThreadManager::~ThreadManager()
{
}

/**
 * @brief Add a thread to an empty place in the array
 *
 * @param Thread thread to add
 * @return true if successfully added
 * @return false if error
 */
uint32_t ThreadManager::create_thread(void entrypoint())
{
    // Create a new thread
    Thread* new_thread = new Thread();
    new_thread -> init(entrypoint);
    new_thread -> m_tid = m_threads.size();
    new_thread -> m_cpu_state -> cs = m_gdt-> code_segment_selector();

    // Add the thread to the array
    m_threads.push_back(new_thread);
    return new_thread -> m_tid;
}

/**
 * @brief Schedules the next thread to be executed by checking its yieldsStatus.
 *
 * @param cpu_state state
 * @return CPUState* The state of the next thread to be executed
 */
CPUState* ThreadManager::schedule(CPUState*cpu_state)
{

    // If the eax register is 37, terminate the thread
    if(cpu_state-> eax == 37)
      terminate_thread(m_current_thread);

    // If there are no threads to schedule, return the current state
    if (m_threads.size() <= 0 || m_threads[m_current_thread] == nullptr)
        return cpu_state;

    // Save the current state
    m_threads[m_current_thread]->m_cpu_state = cpu_state;

    // Switch to the next thread
    if(++m_current_thread >= m_threads.size())
      m_current_thread %= m_threads.size();

    // If this thread is yielded then skip it
    if(m_threads[m_current_thread]->m_yield_status){
      m_threads[m_current_thread]->m_yield_status = false;
        return schedule(cpu_state);
    }

    // Return the next thread's state
    return m_threads[m_current_thread]->m_cpu_state;
}

/**
 * @brief Terminates a thread
 *
 * @param tid thread id to terminate
 * @return true if successfully terminated thread or false if error
 */
bool ThreadManager::terminate_thread(uint32_t tid)
{
    // Check if the thread is actually running
    if (tid >= m_threads.size())
        return false;

    // Delete the thread
    delete m_threads[tid];

    // Erase the thread from the array
    m_threads.erase(m_threads.begin() + tid);
    // TODO: Thread ID needs to be updated

    return true;
}