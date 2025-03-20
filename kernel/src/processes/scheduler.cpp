//
// Created by 98max on 25/02/2025.
//

#include <processes/scheduler.h>

using namespace MaxOS;
using namespace MaxOS::processes;
using namespace MaxOS::memory;


Scheduler* Scheduler::s_instance = nullptr;

Scheduler::Scheduler()
: m_current_thread_index(0),
  m_active(false),
  m_ticks(0),
  m_next_pid(-1),
  m_next_tid(-1)

{
  s_instance = this;

}

Scheduler::~Scheduler() {
  s_instance = nullptr;
  m_active = false;

}

#include <common/kprint.h>
system::cpu_status_t *Scheduler::schedule(system::cpu_status_t* cpu_state) {

  // If there are no threads to schedule or not active, return the current state
  if (m_threads.empty() || !m_active)
      return cpu_state;

  // Ticked
  m_ticks++;

  // Wait for ticks to be div by 10 so it switches every 10 ticks
  if (m_ticks % 10 != 0) return cpu_state; //TODO: Make this automatic for debug builds maybe so that its easier to see errors with out them being overwritten

  // Thread that we are dealing with
  Thread* current_thread = m_threads[m_current_thread_index];

  // If there are no threads to schedule, return the current state
  if (current_thread == nullptr)
    return cpu_state;


  // Save the current state
  current_thread->execution_state = cpu_state;
  current_thread->thread_state = ThreadState::READY;
  current_thread->ticks++;

  // Switch to the next thread
  m_current_thread_index++;
  if (m_current_thread_index >= m_threads.size())
      m_current_thread_index = 0;
  current_thread = m_threads[m_current_thread_index];

  // If the current thread is in the process then we can get the process
  Process* current_process = get_current_process();

  // Handle state changes
  switch (current_thread->thread_state) {

      case ThreadState::NEW:
        current_thread->thread_state = ThreadState::RUNNING;
        break;

      case ThreadState::SLEEPING:

        // If the wake-up time hasn't occurred yet, return
        if (current_thread->wakeup_time > m_ticks)
          return cpu_state;

        // Wake up the thread
        current_thread->thread_state = ThreadState::RUNNING;
        break;

      case ThreadState::STOPPED:

        // Find the process that has the thread and remove it
        for (auto thread : current_process->get_threads()) {
          if (thread == current_thread) {
            current_process->remove_thread(m_current_thread_index);
            break;
          }
        }

        // Remove the thread
        m_threads.erase(m_threads.begin() + m_current_thread_index);

        break;

      default:
        break;
  }

  // Prepare the next thread to run
  current_thread -> thread_state = ThreadState::RUNNING;

  // Load the threads memory manager
  MemoryManager::switch_active_memory_manager(current_process->memory_manager);

  // Load the TSS for the thread
  system::CPU::get_instance() -> tss.rsp0 = current_thread->get_tss_pointer();

  // Return the next thread's state
  return current_thread->execution_state;
}

/**
 * @brief Adds a process to the scheduler
 * @param process The process to add
 * @return The process ID
 */
uint64_t Scheduler::add_process(Process *process) {

  // Get the next process ID
  m_next_pid++;

  // Add the process to the list
  m_processes.push_back(process);
  _kprintf("Adding process %d: %s\n", m_next_pid, process->name.c_str());

  // Return the process ID
  return m_next_pid;

}

/**
 * @brief Adds a thread to the scheduler
 * @param thread The thread to add
 * @return The thread ID
 */
uint64_t Scheduler::add_thread(Thread *thread) {

    // Get the next thread ID
    m_next_tid++;

    // Add the thread to the list
    m_threads.push_back(thread);
    _kprintf("Adding thread %d to process %d\n", m_next_tid, thread->parent_pid);

    // Return the thread ID
    return m_next_tid;

}

/**
 * @brief Gets the system scheduler
 * @return The system scheduler
 */
Scheduler *Scheduler::get_system_scheduler() {

  if(s_instance)
    return s_instance;

  return new Scheduler();
}

/**
 * @brief Gets how long the system has been running for
 * @return The number of ticks
 */
uint64_t Scheduler::get_ticks() {
    return m_ticks;
}

/**
 * @brief Yeild the current thread
 */
void Scheduler::yield() {

  // Interrupt
  asm("int $0x20");
}

/**
 * @brief Activates the scheduler
 */
void Scheduler::activate() {
    m_active = true;
}

/**
 * @brief Removes a process from the scheduler if the process has no threads, if it does then the threads are stopped but the process is not removed (this will be done automatically when all threads are stopped)
 * @param process The process to remove
 * @return -1 if the process has threads, 0 otherwise
 */
uint64_t Scheduler::remove_process(Process *process) {

  // Check if the process has no threads
  if (!process->get_threads().empty()) {

    // Set the threads to stopped
    for (auto thread : process->get_threads())
      thread->thread_state = ThreadState::STOPPED;

    // Return as we can't remove the process
    return -1;
  }

  // Remove the process
  for (uint16_t i = 0; i < m_processes.size(); i++) {
    if (m_processes[i] == process) {
        m_processes.erase(m_processes.begin() + i);
        return 0;
    }
  }

  // Process not found
  return -1;

}

/**
 * @brief Gets the current process
 * @return The current process, or nullptr if not found
 */
Process *Scheduler::get_current_process() {

  Process* current_process = nullptr;

  // Find the process that has the thread being executed
  for (auto process : s_instance -> m_processes) {
    if (process->get_pid() == s_instance -> m_threads[s_instance -> m_current_thread_index]->parent_pid) {
      current_process = process;
      break;
    }
  }

  return current_process;
}

/**
 * @brief Deactivates the scheduler
 */
void Scheduler::deactivate() {
    m_active = false;

}
