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
  set_syscall_handler(SyscallType::KLOG, syscall_klog);
  set_syscall_handler(SyscallType::CREATE_SHARED_MEMORY, syscall_create_shared_memory);
  set_syscall_handler(SyscallType::OPEN_SHARED_MEMORY, syscall_open_shared_memory);
  set_syscall_handler(SyscallType::ALLOCATE_MEMORY, syscall_allocate_memory);
  set_syscall_handler(SyscallType::FREE_MEMORY, syscall_free_memory);

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
