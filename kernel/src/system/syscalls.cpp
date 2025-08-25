//
// Created by 98max on 27/10/2022.
//

#include <system/syscalls.h>
#include <common/logger.h>

using namespace MaxOS;
using namespace MaxOS::common;
using namespace MaxOS::hardwarecommunication;
using namespace MaxOS::system;
using namespace MaxOS::processes;
using namespace MaxOS::memory;


SyscallManager::SyscallManager()
:    InterruptHandler(0x80)
{

  // Clear the args
  Logger::INFO() << "Setting up Syscalls \n";
  m_current_args = new syscall_args_t;

  // Register the handlers
  set_syscall_handler(SyscallType::CLOSE_PROCESS, syscall_close_process);
  set_syscall_handler(SyscallType::KLOG, syscall_klog);
  set_syscall_handler(SyscallType::CREATE_SHARED_MEMORY, syscall_create_shared_memory);
  set_syscall_handler(SyscallType::OPEN_SHARED_MEMORY, syscall_open_shared_memory);
  set_syscall_handler(SyscallType::ALLOCATE_MEMORY, syscall_allocate_memory);
  set_syscall_handler(SyscallType::FREE_MEMORY, syscall_free_memory);
  set_syscall_handler(SyscallType::CREATE_IPC_ENDPOINT, syscall_create_ipc_endpoint);
  set_syscall_handler(SyscallType::SEND_IPC_MESSAGE, syscall_send_ipc_message);
  set_syscall_handler(SyscallType::REMOVE_IPC_ENDPOINT, syscall_remove_ipc_endpoint);
  set_syscall_handler(SyscallType::THREAD_YIELD, syscall_thread_yield);
  set_syscall_handler(SyscallType::THREAD_SLEEP, syscall_thread_sleep);

}

SyscallManager::~SyscallManager() = default;

/**
 * @brief Loads the args from the registers and delegates the syscall to the relevant handler if defined
 *
 * @param status The cpu state
 * @return The cpu state (may be modified with return value in RAX or a new state by the handler)
 */
cpu_status_t* SyscallManager::handle_interrupt(cpu_status_t* status) {

  // Get the args from the cpu state
  m_current_args -> arg0 = status -> rdi;
  m_current_args -> arg1 = status -> rsi;
  m_current_args -> arg2 = status -> rdx;
  m_current_args -> arg3 = status -> r10;
  m_current_args -> arg4 = status -> r8;
  m_current_args -> arg5 = status -> r9;
  m_current_args -> return_value = 0;
  m_current_args -> return_state = status;

  // Call the handler
  uint64_t syscall = status -> rax;
  if(m_syscall_handlers[syscall] != nullptr)
    m_current_args = m_syscall_handlers[syscall](m_current_args);
  else
    Logger::ERROR() << "Syscall " << syscall << " not found\n";

  // If there is a specific return state, use that
  if(m_current_args -> return_state != status)
    return m_current_args -> return_state;

  // Update the cpu state
  status -> rdi = m_current_args -> arg0;
  status -> rsi = m_current_args -> arg1;
  status -> rdx = m_current_args -> arg2;
  status -> r10 = m_current_args -> arg3;
  status -> r8  = m_current_args -> arg4;
  status -> r9  = m_current_args -> arg5;
  status -> rax = m_current_args -> return_value;


  // Return the status
  return status;
}

/**
 * @brief Loads a syscall handler into the manager
 *
 * @param syscall The syscall ID number
 * @param handler The handler to set
 */
void SyscallManager::set_syscall_handler(SyscallType syscall, syscall_func_t handler) {
  m_syscall_handlers[(uint8_t)syscall] = handler;
}

/**
 * @brief Removes a syscall handler from the manager
 *
 * @param syscall The syscall ID number
 */
void SyscallManager::remove_syscall_handler(SyscallType syscall) {
    m_syscall_handlers[(uint8_t)syscall] = nullptr;
}


/**
 * @brief System call to close a process
 *
 * @param args Arg0 = pid Arg1 = exit code
 * @return Nothing
 */
system::syscall_args_t* SyscallManager::syscall_close_process(system::syscall_args_t *args) {

  // Get the args
  uint64_t pid = args -> arg0;
  int exit_code = (int)args -> arg1;

  // Close the process
  Process* process = pid == 0 ? Scheduler::current_process() : Scheduler::get_process(pid);
  Scheduler::system_scheduler() -> remove_process(process);

  // Schedule the next process
  cpu_status_t* next_process =  Scheduler::system_scheduler() -> schedule_next(args -> return_state);
  args -> return_state = next_process;

  // Done
  return args;
}

syscall_args_t *SyscallManager::syscall_klog(syscall_args_t *args) {

  char* message = (char*)args -> arg0;

  // If the first two characters are %h then no header
  if(message[0] == '%' && message[1] == 'h')
    Logger::INFO() << message + 2;
  else
    Logger::INFO() << ANSI_COLOURS[FG_Blue] << "(" << Scheduler::current_process() -> name.c_str() << ":" << Scheduler::current_thread() -> tid << "): " << ANSI_COLOURS[Reset] << message;

  return args;
}



/**
 * @brief System call to create a shared memory block (To close, free the memory, it is automatically handled when the process exits)
 *
 * @param args Arg0 = size, Arg1 = name
 * @return The virtual address of the shared memory block or null if failed
 */
syscall_args_t* SyscallManager::syscall_create_shared_memory(syscall_args_t *args) {
  
  // Extract the arguments
  size_t size = args->arg0;
  char* name = (char*)args->arg1;

  // Ensure they are valid
  if(size == 0 || name == nullptr)
    return nullptr;

  // Create the memory block
  SharedMemory* new_block = Scheduler::scheduler_ipc() ->alloc_shared_memory(size, name);

  // Load the block
  void* virtual_address = MemoryManager::s_current_memory_manager -> vmm() ->load_shared_memory(new_block -> physical_address(), size);

  // Return to the user
  args -> return_value = (uint64_t)virtual_address;
  return args;

}

/**
 * @brief System call to open a shared memory block (To close, free the memory, it is automatically handled when the process exits)
 *
 * @param args Arg0 = name
 * @return The virtual address of the shared memory block or null if failed
 */
syscall_args_t * SyscallManager::syscall_open_shared_memory(syscall_args_t *args) {

  // Extract the arguments
  uint64_t name = args->arg0;

  // Ensure they are valid
  if(name == 0)
    return nullptr;

  // Get the block (don't care if null as that is caught in the load_shared_memory function)
  SharedMemory* block = Scheduler::scheduler_ipc() -> get_shared_memory((char*)name);

  // Load the block
  void* virtual_address = MemoryManager::s_current_memory_manager -> vmm() ->load_shared_memory(block -> physical_address(), block -> size());

  // Return to the user
  args -> return_value = (uint64_t)virtual_address;
  return args;

}

/**
 * @brief System call to free a block of memory
 *
 * @param args Arg0 = size
 * @return The address of the memory block or null if failed
 */
syscall_args_t* SyscallManager::syscall_allocate_memory(syscall_args_t *args) {

  // Get the size
  size_t size = args -> arg0;

  // Malloc the memory
  void* address = MemoryManager::malloc(size);

  // Return the address
  args -> return_value = (uint64_t)address;
  return args;

}

/**
 * @brief System call to free a block of memory
 *
 * @param args Arg0 = address
 * @return Nothing
 */
syscall_args_t* SyscallManager::syscall_free_memory(syscall_args_t *args) {

  // Get the address
  void* address = (void*)args -> arg0;

  // Free the memory
  MemoryManager::free(address);

  // Done
  return args;

}

/**
 * @brief System call to create an IPC endpoint
 *
 * @param args Arg0 = name
 * @return The IPC endpoint buffer linked list address
 */
system::syscall_args_t* SyscallManager::syscall_create_ipc_endpoint(system::syscall_args_t *args) {

  // Get the name
  char* name = (char*)args -> arg0;
  if(name == nullptr)
    return nullptr;

  // Create the endpoint
  SharedMessageEndpoint* endpoint = Scheduler::scheduler_ipc() -> create_message_endpoint(name);

  // Return the endpoint
  args -> return_value = (uint64_t)endpoint -> queue();
  return args;

}

/**
 * @brief System call to send an IPC message
 *
 * @param args Arg0 = endpoint name, Arg1 = message, Arg2 = size
 * @return Nothing
 */
system::syscall_args_t* SyscallManager::syscall_send_ipc_message(system::syscall_args_t *args) {

  // Get the args
  char* endpoint = (char*)args -> arg0;
  void* message = (void*)args -> arg1;
  size_t size = args -> arg2;

  // Validate the args
  if(endpoint == nullptr || message == nullptr || size == 0)
    return nullptr;

  // Send the message
  Scheduler::scheduler_ipc() ->get_message_endpoint(endpoint) -> queue_message(message, size);

  // All done
  return args;
}

/**
 * @brief System call to remove an IPC endpoint
 *
 * @param args Arg0 = endpoint name
 * @return Nothing
 */
system::syscall_args_t * SyscallManager::syscall_remove_ipc_endpoint(system::syscall_args_t *args) {

  // Remove the endpoint
  Scheduler::scheduler_ipc() -> free_message_endpoint((char*)args -> arg0);

  // Done
  return args;

}


/**
 * @brief System call to yield the current process
 *
 * @param args Nothing
 * @return Nothing
 */
system::syscall_args_t* SyscallManager::syscall_thread_yield(system::syscall_args_t *args) {

  // Yield
  cpu_status_t* next_process = Scheduler::system_scheduler() -> yield();
  args -> return_state = next_process;

  return args;
}

/**
 * @brief System call to sleep the current process
 *
 * @param args Arg0 = milliseconds
 * @return Nothing
 */
system::syscall_args_t* SyscallManager::syscall_thread_sleep(system::syscall_args_t *args) {

  // Get the milliseconds
  size_t milliseconds = args -> arg0;

  // Store the updated state in the thread as the scheduler will not have the updated state when switching to the next thread
  Scheduler::current_thread() -> execution_state = args -> return_state;

  // Sleep the thread
  cpu_status_t* next_thread = Scheduler::current_thread() -> sleep(milliseconds);
  args -> return_state = next_thread;

  // Done
  return args;

}

/**
 * @brief System call to close a thread
 *
 * @param args Arg0 = tid Arg1 = exit code
 * @return Nothing
 */
system::syscall_args_t* SyscallManager::syscall_thread_close(system::syscall_args_t *args) {


  // Get the args
  uint64_t tid = args -> arg0;
  int exit_code = (int)args -> arg1;

  // Get the thread if it is 0 then it is the current thread
  Thread* thread = tid == 0 ? Scheduler::current_thread() : Scheduler::get_thread(tid);

  // Close the thread
  Scheduler::get_process(thread -> parent_pid) -> remove_thread(tid);

  // Schedule the next thread
  cpu_status_t* next_thread =  Scheduler::system_scheduler() -> schedule_next(args -> return_state);
  args -> return_state = next_thread;

  // Done
  return args;

}
