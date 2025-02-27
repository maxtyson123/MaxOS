//
// Created by 98max on 25/02/2025.
//
#include <processes/process.h>
#include <common/kprint.h>

using namespace MaxOS;
using namespace MaxOS::system;
using namespace MaxOS::memory;
using namespace MaxOS::processes;
using namespace MaxOS::common;


/**
 * @brief Constructor for the Thread class
 */
Thread::Thread(void (*_entry_point)(void *), void *args, Process* parent) {

    // Basic setup
    thread_state = ThreadState::NEW;
    wakeup_time = 0;
    ticks = 0;

    // Create the stack
    m_stack_pointer = (uintptr_t)MemoryManager::s_active_memory_manager ->malloc(s_stack_size);
    ASSERT(m_stack_pointer != 0, "Failed to allocate stack for thread\n");

    // Setup the execution state
    execution_state = new cpu_status_t();
    execution_state->rip = (uint64_t)_entry_point;
    execution_state->rsp = (uint64_t)args;
    execution_state->ss = 0x10;     // TEMP KERNEL 0x23
    execution_state->cs = 0x08;     // TEMP KERNEL 0x1B
    execution_state->rflags = 0x202;
    execution_state->interrupt_number = 0;
    execution_state->error_code = 0;
    execution_state->rsp = (uint64_t)m_stack_pointer;
    execution_state->rbp = 0;

    // Begin scheduling this thread
    parent_pid = parent->get_pid();
    tid = Scheduler::get_system_scheduler() -> add_thread(this);


}

/**
 * @brief Destructor for the Thread class
 */
Thread::~Thread() {

}


void Thread::sleep(size_t milliseconds) {

  // Update the vars
  thread_state = ThreadState::SLEEPING;
  wakeup_time = Scheduler::get_system_scheduler() -> get_ticks() + milliseconds;


  // Yield
  Scheduler::get_system_scheduler() -> yield();

}

/**
 * @brief Constructor for the Process class (from a function)
 * @param name The name of the process
 * @param _entry_point The entry point of the process
 * @param args The arguments to pass to the process
 */
Process::Process(string p_name, void (*_entry_point)(void *), void *args) {

  // Pause interrupts while creating the process
  asm("cli");

  // Basic setup
  name = p_name;
  m_pid = Scheduler::get_system_scheduler() ->add_process(this);
  m_virtual_memory_manager = new VirtualMemoryManager(false);
  memory_manager = new MemoryManager(m_virtual_memory_manager);

  // Create the main thread
  Thread* main_thread = new Thread(_entry_point, args, this);

  // Add the thread
  add_thread(main_thread);

  // Can now resume interrupts
  asm("sti");

}

/**
 * @brief Destructor for the Process class
 */
Process::~Process() {

  _kprintf("Cleaning up process %s\n", name.c_str());

  // Free the threads
  for (auto thread : m_threads)
      delete thread;

  // Free the memory manager
  delete m_virtual_memory_manager;
  delete memory_manager;

}

/**
 * @brief Adds a thread to the process
 * @param thread The thread to add
 */
void Process::add_thread(Thread *thread) {

  // Pause interrupts while adding the thread
  asm("cli");

  // Store the thread
  m_threads.push_back(thread);

  // Set the pid
  thread->parent_pid = m_pid;

  // Can now resume interrupts
  asm("sti");

}

/**
 * @brief Finds a thread by its tid
 * @param tid
 */
void Process::remove_thread(uint64_t tid) {

  // Find the thread
  for (uint16_t i = 0; i < m_threads.size(); i++) {
      if (m_threads[i]->tid == tid) {

        // Get the thread
        Thread* thread = m_threads[i];

        // Delete the thread
        delete thread;

        // Remove the thread from the list
        m_threads.erase(m_threads.begin() + i);


        // If there are no more threads then delete the process
        if (m_threads.empty()) {
          Scheduler::get_system_scheduler() -> remove_process(this);
          delete this;
        }

        return;
    }
  }
}

/**
 * @brief Sets the pid of the process once added to the queue
 * @param pid
 */
void Process::set_pid(uint64_t pid) {

  // Check if the pid is already set
  if (m_pid != 0)
        return;

  // Set the pid
  m_pid = pid;

  // Assign the pid to the threads
  for (auto thread : m_threads)
        thread->parent_pid = pid;


}

/**
 * @brief Gets the threads of the process
 */
Vector<Thread*> Process::get_threads() {

  // Return the threads
  return m_threads;

}

/**
 * @brief Gets the pid of the process
 * @return The pid of the process
 */
uint64_t Process::get_pid() {
  return m_pid;
}
