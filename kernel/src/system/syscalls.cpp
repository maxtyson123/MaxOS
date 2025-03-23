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


SyscallManager::SyscallManager(InterruptManager*interrupt_manager)
:    InterruptHandler(0x80, interrupt_manager)
{

  // Clear the args
  m_current_args = new syscall_args_t;

  // Register the general syscalls
  set_syscall_handler(0x01, syscall_write);

}

SyscallManager::~SyscallManager()
{
    // Delete the args
    delete m_current_args;
}


system::cpu_status_t* SyscallManager::handle_interrupt(system::cpu_status_t* status) {

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


  // Return the status
  return status;
}

void SyscallManager::set_syscall_handler(uint8_t syscall, syscall_func_t handler) {
  m_syscall_handlers[syscall] = handler;
}

void SyscallManager::remove_syscall_handler(uint8_t syscall) {
    m_syscall_handlers[syscall] = 0;
}

syscall_args_t *SyscallManager::syscall_write(syscall_args_t *args) {
  _kprintf("%h%s[%s:%d]%s %s", ANSI_COLOURS[FG_Blue], Scheduler::get_current_process() -> name.c_str(), Scheduler::get_current_thread() -> tid,  ANSI_COLOURS[Reset], (char*)args -> arg0);
  return args;
}
