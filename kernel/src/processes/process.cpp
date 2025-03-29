//
// Created by 98max on 25/02/2025.
//
#include <processes/process.h>
#include <common/kprint.h>
#include <processes/scheduler.h>    //TODO: This is a circular dependency, need to fix, make the scheduler handle that sorta stuff

using namespace MaxOS;
using namespace MaxOS::system;
using namespace MaxOS::memory;
using namespace MaxOS::processes;
using namespace MaxOS::common;


/**
 * @brief Constructor for the Thread class
 */
Thread::Thread(void (*_entry_point)(void *), void *args, int arg_amount, Process* parent) {

    // Basic setup
    thread_state = ThreadState::NEW;
    wakeup_time = 0;
    ticks = 0;

    // Create the stack
    m_stack_pointer = (uintptr_t)MemoryManager::malloc(s_stack_size);

    // Create the TSS stack
    if(parent -> is_kernel) {

        // Use the kernel stack
        m_tss_stack_pointer = CPU::get_instance() -> tss.rsp0;

    } else{
        m_tss_stack_pointer = (uintptr_t)MemoryManager::kmalloc(s_stack_size) + s_stack_size;
    }

    // Mak sure there is a stack
    ASSERT(m_stack_pointer != 0 && m_tss_stack_pointer != 0, "Failed to allocate stack for thread");

    // Set up the execution state
    execution_state = new cpu_status_t();
    execution_state->rip = (uint64_t)_entry_point;
    execution_state->ss = parent -> is_kernel ? 0x10 : 0x23;
    execution_state->cs = parent -> is_kernel ? 0x8  : 0x1B;
    execution_state->rflags = 0x202;
    execution_state->interrupt_number = 0;
    execution_state->error_code = 0;
    execution_state->rsp = (uint64_t)m_stack_pointer;
    execution_state->rbp = 0;

    // Copy the args into user space using memcopy
    uint64_t  argc = arg_amount;
    void* argv = MemoryManager::malloc(arg_amount * sizeof(void*));
    memcpy(argv, args, arg_amount * sizeof(void*));


    execution_state->rdi = argc;
    execution_state->rsi = (uint64_t)argv;
    //execution_state->rdx = (uint64_t)env_args;

    // Begin scheduling this thread
    parent_pid = parent->get_pid();
    tid = Scheduler::get_system_scheduler() -> add_thread(this);

}

/**
 * @brief Destructor for the Thread class
 */
Thread::~Thread() {

}

/**
 * @brief Sleeps the thread for a certain amount of time (Yields the thread)
 *
 * @param milliseconds The amount of time to sleep for
 * @return The status of the CPU for the next thread to run
 */
cpu_status_t* Thread::sleep(size_t milliseconds) {

  // Update the vars
  thread_state = ThreadState::SLEEPING;
  wakeup_time = Scheduler::get_system_scheduler() -> get_ticks() + milliseconds;

  // Yield
  return Scheduler::get_system_scheduler() -> yield();

}

/**
 * @brief Constructor for the Process class (from a function)
 *
 * @param name The name of the process
 * @param _entry_point The entry point of the process
 * @param args The arguments to pass to the process
 * @param arg_amount The amount of arguments
 * @param is_kernel If the process is a kernel process
 */
Process::Process(string p_name, void (*_entry_point)(void *), void *args, int arg_amount, bool is_kernel)
: is_kernel(is_kernel),
  name(p_name)
{

  // Basic setup
  set_up();

  // Create the main thread
  Thread* main_thread = new Thread(_entry_point, args, arg_amount, this);

  // Add the thread
  add_thread(main_thread);

}

/**
 * @brief Constructor for the Process class (from an elf, will free the elf memory after loading)
 *
 * @param name  The name of the process
 * @param args  The arguments to pass to the process
 * @param arg_amount  The amount of arguments
 * @param elf  The elf file to load the process from
 * @param is_kernel  If the process is a kernel process
 */
Process::Process(string p_name, void *args, int arg_amount, Elf64* elf, bool is_kernel)
: is_kernel(is_kernel),
  name(p_name)
{

  // Basic setup
  set_up();

  // Load the elf
  elf -> load();

  // Get the entry point
  void (*entry_point)(void *) = (void (*)(void *))elf -> get_header() -> entry;

  // Create the main thread
  Thread* main_thread = new Thread(entry_point, args, arg_amount, this);

  // Add the thread
  add_thread(main_thread);

  // Free the elf
  delete elf;
}


/**
 * @brief Destructor for the Process class
 */
Process::~Process() {

  _kprintf("Cleaning up process %s\n", name.c_str());

  // Free the threads
  for (auto thread : m_threads)
      delete thread;

  // Free the memory manager (only if it was created)
  if(!is_kernel){
      delete memory_manager;
      delete m_virtual_memory_manager;
  }

}

/**
 * @brief Adds a thread to the process
 *
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
 *
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


        // If there are no more threads then delete the process (done on the scheduler side)
        if (m_threads.empty())
          Scheduler::get_system_scheduler() -> remove_process(this);

        return;
    }
  }
}

/**
 * @brief Sets the pid of the process once added to the queue
 *
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
 *
 * @return The pid of the process
 */
uint64_t Process::get_pid() {
  return m_pid;
}

/**
 * @brief Generic setup function for the process
 */
void Process::set_up() {

  // Pause interrupts while creating the process
  asm("cli");

  // Basic setup
  m_pid = Scheduler::get_system_scheduler() ->add_process(this);

  // If it is a kernel process then don't need a new memory manager
  if(!is_kernel){
    m_virtual_memory_manager = new VirtualMemoryManager(false);
    memory_manager = new MemoryManager(m_virtual_memory_manager);
  }else{
    memory_manager = MemoryManager::s_kernel_memory_manager;
  }

}

/**
 * @brief Gets the total ticks of the threads in the process (not this does not include any past killed threads)
 *
 * @return The total ticks of the threads in the process
 */
uint64_t Process::get_total_ticks() {

  uint64_t total_ticks = 0;
  for (auto thread : m_threads)
    total_ticks += thread->ticks;

  return total_ticks;
}
