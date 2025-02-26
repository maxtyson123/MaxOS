//
// Created by 98max on 25/02/2025.
//

#include <processes/scheduler.h>

using namespace MaxOS;
using namespace MaxOS::processes;


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

  // Ticked
  m_ticks++;

  // If there are no threads to schedule or not active, return the current state
  if (m_threads.empty() || !m_active)
      return cpu_state;


  // Thread that we are dealing with
  Thread* current_thread = m_threads[m_current_thread_index];

  // If there are no threads to schedule, return the current state
  if (m_threads.empty() || current_thread == nullptr)
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
  Process* current_process = nullptr;
  for (auto process : m_processes) {
    if (process->get_pid() == current_thread->parent_pid) {
      current_process = process;
      break;
    }
  }


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

  // Load the thread's page table
  asm volatile("mov %0, %%cr3" :: "r"((uint64_t)current_process->get_page_directory()) : "memory");

  // Return the next thread's state
  return current_thread->execution_state;
}

/**
 * @brief Adds a process to the scheduler
 * @param process The process to add
 * @return The process ID
 */
uint16_t Scheduler::add_process(Process *process) {

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
uint16_t Scheduler::add_thread(Thread *thread) {

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
