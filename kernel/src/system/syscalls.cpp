//
// Created by 98max on 27/10/2022.
//

#include <system/syscalls.h>
#include <common/kprint.h>

using namespace MaxOS;
using namespace MaxOS::common;
using namespace MaxOS::hardwarecommunication;
using namespace MaxOS::system;
using namespace MaxOS::processes;
using namespace MaxOS::memory;


SyscallManager::SyscallManager(InterruptManager*interrupt_manager)
:    InterruptHandler(0x80, interrupt_manager)
{

  // Clear the args
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
  set_syscall_handler(SyscallType::PROCESS_YIELD, syscall_process_yield);
  set_syscall_handler(SyscallType::PROCESS_SLEEP, syscall_process_sleep);

}

SyscallManager::~SyscallManager()
{
    // Delete the args
    delete m_current_args;

}


cpu_status_t* SyscallManager::handle_interrupt(cpu_status_t* status) {

  // Get the args from the cpu state
  m_current_args -> arg0 = status -> rdi;
  m_current_args -> arg1 = status -> rsi;
  m_current_args -> arg2 = status -> rdx;
  m_current_args -> arg3 = status -> r10;
  m_current_args -> arg4 = status -> r8;
  m_current_args -> arg5 = status -> r9;

  // Call the handler
  uint64_t syscall = status -> rax;
  if(m_syscall_handlers[syscall] != 0)
    m_current_args = m_syscall_handlers[syscall](m_current_args);
  else
    _kprintf("Syscall %d not found\n", syscall);

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

void SyscallManager::set_syscall_handler(uint8_t syscall, syscall_func_t handler) {
  m_syscall_handlers[syscall] = handler;
}

void SyscallManager::remove_syscall_handler(uint8_t syscall) {
    m_syscall_handlers[syscall] = 0;
}



syscall_args_t *SyscallManager::syscall_klog(syscall_args_t *args) {
  _kprintf("%h%s[%s:%d]%s %s", ANSI_COLOURS[FG_Blue], Scheduler::get_current_process() -> name.c_str(), Scheduler::get_current_thread() -> tid,  ANSI_COLOURS[Reset], (char*)args -> arg0);
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
  if(size == 0 || name == 0)
    return nullptr;

  // Create the memory block
  ipc_shared_memory_t* new_block = Scheduler::get_ipc() ->alloc_shared_memory(size, name);

  // Load the block
  void* virtual_address = MemoryManager::s_current_memory_manager -> get_vmm() ->load_shared_memory(new_block -> physical_address, size);

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

  // Get the block (dont care if null as that is caught in the load_shared_memory function)
  ipc_shared_memory_t* block = Scheduler::get_ipc() -> get_shared_memory((char*)name);

  // Load the block
  void* virtual_address = MemoryManager::s_current_memory_manager -> get_vmm() ->load_shared_memory(block -> physical_address, block -> size);

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
  if(name == 0)
    return nullptr;

  // Create the endpoint
  ipc_message_endpoint_t* endpoint = Scheduler::get_ipc() -> create_message_endpoint(name);

  // Return the endpoint
  args -> return_value = (uint64_t)endpoint -> queue;
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
  if(endpoint == 0 || message == 0 || size == 0)
    return nullptr;

  // Send the message
  Scheduler::get_ipc() -> send_message(endpoint, message, size);

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
  Scheduler::get_ipc() -> free_message_endpoint((char*)args -> arg0);

  // Done
  return args;

}


/**
 * @brief System call to yield the current process
 *
 * @param args Nothing
 * @return Nothing
 */
system::syscall_args_t* SyscallManager::syscall_process_yield(system::syscall_args_t *args) {

  // Yield
  Scheduler::get_system_scheduler() -> yield();
  return args;
}

/**
 * @brief System call to sleep the current process
 *
 * @param args Arg0 = milliseconds
 * @return Nothing
 */
system::syscall_args_t* SyscallManager::syscall_process_sleep(system::syscall_args_t *args) {

  // Get the milliseconds
  size_t milliseconds = args -> arg0;

  // Sleep the thread
  Scheduler::get_current_thread() -> sleep(milliseconds);

  // Done
  return args;

}

/**
 * @brief System call to close a process
 *
 * @param args Arg0 = pid Arg1 = exit code
 * @return
 */
system::syscall_args_t* SyscallManager::syscall_close_process(system::syscall_args_t *args) {

  // Get the args
  uint64_t pid = args -> arg0;
  int exit_code = (int)args -> arg1;

  // Get the process if it is 0 then it is the current process
  Process* process = pid == 0 ? Scheduler::get_current_process() : Scheduler::get_process(pid);

  // Close the process
  Scheduler::get_system_scheduler() -> remove_process(process);

  // Done
  return args;
}
