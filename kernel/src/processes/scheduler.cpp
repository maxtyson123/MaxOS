//
// Created by 98max on 25/02/2025.
//

#include <processes/scheduler.h>
#include <common/kprint.h>

using namespace MaxOS;
using namespace MaxOS::processes;
using namespace MaxOS::memory;
using namespace MaxOS::hardwarecommunication;
using namespace MaxOS::system;


Scheduler* Scheduler::s_instance = nullptr;

Scheduler::Scheduler(InterruptManager* interrupt_manager)
: InterruptHandler(0x20, interrupt_manager),
  m_current_thread_index(0),
  m_active(false),
  m_ticks(0),
  m_next_pid(-1),
  m_next_tid(-1)

{
  s_instance = this;

  // Create the IPC handler
  m_ipc = new IPC();
}

Scheduler::~Scheduler() {
  s_instance = nullptr;
  m_active = false;

  // Delete the IPC handler
  delete m_ipc;

}

/**
 * @brief Handles the interrupt 0x20
 * @param status The current CPU status
 * @return The new CPU status
 */
cpu_status_t* Scheduler::handle_interrupt(cpu_status_t *status) {

    // Schedule the next thread
    return schedule(status);

    /// Note: Could have set scheduler to just be the handle interrupt function,
    ////      but in the future there may be a need to schedule at other times
}


/**
 * @brief Schedules the next thread to run
 * @param cpu_state The current CPU state
 * @return The next CPU state
 */
cpu_status_t *Scheduler::schedule(cpu_status_t* cpu_state) {

  // If there are no threads to schedule or not active, return the current state
  if (m_threads.empty() || !m_active)
      return cpu_state;


  // Thread that we are dealing with
  Thread* current_thread = m_threads[m_current_thread_index];

  // Ticked
  m_ticks++;
  current_thread->ticks++;

   // Wait for a bit so that the scheduler doesn't run too fast
   if (m_ticks % s_ticks_per_event != 0) return cpu_state;

   // Schedule the next thread
   return schedule_next(cpu_state);

}

/**
 * @brief Schedules the next thread to run
 * @param status The current CPU status of the thread
 * @return The next CPU status
 */
system::cpu_status_t *Scheduler::schedule_next(system::cpu_status_t* cpu_state) {

  // Get the current thread
  Thread* current_thread = m_threads[m_current_thread_index];

  // Save the current state
  current_thread->execution_state = cpu_state;
  if(current_thread->thread_state == ThreadState::RUNNING)
    current_thread->thread_state = ThreadState::WAITING;

  // Switch to the next thread
  m_current_thread_index++;
  m_current_thread_index %= m_threads.size();

  current_thread = m_threads[m_current_thread_index];

  // If the current thread is in the process then we can get the process
  Process* current_process = get_current_process();

  // Handle state changes
  switch (current_thread->thread_state) {

    case ThreadState::NEW:
      current_thread->thread_state = ThreadState::RUNNING;
      break;

    case ThreadState::SLEEPING:

      // If the wake-up time hasn't occurred yet, run the next thread
      if (current_thread->wakeup_time > m_ticks)
        return schedule_next(current_thread->execution_state);

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

      // Run the next thread
      return schedule_next(cpu_state);

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
 * @return The system scheduler or nullptr if not found
 */
Scheduler *Scheduler::get_system_scheduler() {

  if(s_instance)
    return s_instance;

  return nullptr;
}

/**
 * @brief Gets how long the system has been running for
 * @return The number of ticks
 */
uint64_t Scheduler::get_ticks() {
    return m_ticks;
}

/**
 * @brief Yield the current thread
 */
cpu_status_t* Scheduler::yield() {

  // If this is the only thread, can't yield
  if (m_threads.size() <= 1)
      return get_current_thread()->execution_state;

  // Set the current thread to waiting if running
  if (m_threads[m_current_thread_index]->thread_state == ThreadState::RUNNING)
      m_threads[m_current_thread_index]->thread_state = ThreadState::WAITING;


  // Schedule the next thread
  return schedule_next(get_current_thread()->execution_state);

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
 * @param force If true, the process will be removed and so will all threads
 * @return -1 if the process has threads, 0 otherwise
 */
uint64_t Scheduler::remove_process(Process *process) {

  // Check if the process has no threads
  if (!process->get_threads().empty()) {

    // Set the threads to stopped or remove them if forced
    for (auto thread : process->get_threads())
        thread->thread_state = ThreadState::STOPPED;

    // Need to wait until the threads are stopped before removing the process (this will be called again when all threads are stopped)
    return -1;

  }

  // Remove the process
  for (uint16_t i = 0; i < m_processes.size(); i++) {
    if (m_processes[i] == process) {
        m_processes.erase(m_processes.begin() + i);

        // Delete the process mem
        delete process;
        return 0;
    }
  }

  // Process not found
  return -1;

}

/**
 * @brief Removes a process from the scheduler and deletes all threads, begins running the next process
 *
 * @param process The process to remove
 * @return The status of the CPU for the next process to run or nullptr if the process was not found
 */
cpu_status_t* Scheduler::force_remove_process(Process *process) {

  // If there is no process, fail
  if (!process)
      return nullptr;

  // Remove all the threads
  for (auto thread : process->get_threads()){

    // Remove the thread from the scheduler
    int index = m_threads.find(thread) - m_threads.begin();
    m_threads.erase(m_threads.begin() + index);

    // Delete the thread
    process->remove_thread(thread->tid);

  }


  // Process will be dead now so run the next process (don't care about the execution state being outdated as we are removing it anyway)
  return schedule_next(get_current_thread()->execution_state);
}

/**
 * @brief Gets the current process
 * @return The current process, or nullptr if not found
 */
Process *Scheduler::get_current_process() {

  Process* current_process = nullptr;

  // Find the process that has the thread being executed
  for (auto process : s_instance -> m_processes)
    if (process->get_pid() == get_current_thread() -> parent_pid) {
      current_process = process;
      break;
    }

  return current_process;
}

/**
 * @brief Gets a process by its PID
 * @param pid The process ID
 * @return The process or nullptr if not found
 */
Process *Scheduler::get_process(uint64_t pid) {

  // Try to find the process
  for (auto process : s_instance->m_processes)
    if (process->get_pid() == pid)
      return process;

  // Not found
  return nullptr;
}


/**
 * @brief Gets the currently executing thread
 * @return The currently executing thread
 */
Thread *Scheduler::get_current_thread() {

  return s_instance -> m_threads[s_instance -> m_current_thread_index];

}

/**
 * @brief Deactivates the scheduler
 */
void Scheduler::deactivate() {
    m_active = false;

}

/**
 * @brief Loads any valid ELF files from the multiboot structure
 *
 * @param multiboot The multiboot structure
 */
void Scheduler::load_multiboot_elfs(system::Multiboot *multiboot) {

  for(multiboot_tag* tag = multiboot -> get_start_tag(); tag->type != MULTIBOOT_TAG_TYPE_END; tag = (struct multiboot_tag *) ((multiboot_uint8_t *) tag + ((tag->size + 7) & ~7))) {
    if(tag -> type != MULTIBOOT_TAG_TYPE_MODULE)
      continue;

    // Get the module tag
    struct multiboot_tag_module* module = (struct multiboot_tag_module*)tag;

    // Create the elf
    Elf64* elf = new Elf64((uintptr_t)MemoryManager::to_dm_region((uintptr_t )module->mod_start));
    if(!elf->is_valid())
      continue;

    _kprintf("Creating process from multiboot module for %s (at 0x%x)\n", module->cmdline, module->mod_start);

    // Create an array of args for the process
    char* args[1] = {module->cmdline};

    // Create the process
    Process* process = new Process(module->cmdline, args, 1, elf);

    _kprintf("Elf loaded to pid %d\n", process->get_pid());
  }

}

/**
 * @brief Gets the IPC handler
 * @return The IPC handler or nullptr if not found
 */
IPC *Scheduler::get_ipc() {

  return s_instance -> m_ipc;

}

/**
 * @brief Gets a thread by its TID
 *
 * @param tid The thread ID
 * @return The thread or nullptr if not found
 */
Thread *Scheduler::get_thread(uint64_t tid) {

  // Try to find the thread
  for (auto thread : s_instance -> m_threads)
    if (thread -> tid == tid)
      return thread;

  return nullptr;
}
