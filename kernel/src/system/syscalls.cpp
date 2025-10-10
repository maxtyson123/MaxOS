//
// Created by 98max on 27/10/2022.
//

#include <system/syscalls.h>
#include <common/logger.h>

using namespace syscore;
using namespace MaxOS;
using namespace MaxOS::common;
using namespace MaxOS::hardwarecommunication;
using namespace MaxOS::system;
using namespace MaxOS::processes;
using namespace MaxOS::memory;


SyscallManager::SyscallManager()
: InterruptHandler(0x80)
{

	// Clear the args
	Logger::INFO() << "Setting up Syscalls \n";
	m_current_args = new syscall_args_t;

	// Register the handlers
	set_syscall_handler(SyscallType::CLOSE_PROCESS, syscall_close_process);
	set_syscall_handler(SyscallType::KLOG, syscall_klog);
	set_syscall_handler(SyscallType::ALLOCATE_MEMORY, syscall_allocate_memory);
	set_syscall_handler(SyscallType::FREE_MEMORY, syscall_free_memory);
	set_syscall_handler(SyscallType::RESOURCE_CREATE, syscall_resource_create);
	set_syscall_handler(SyscallType::RESOURCE_OPEN, syscall_resource_open);
	set_syscall_handler(SyscallType::RESOURCE_CLOSE, syscall_resource_close);
	set_syscall_handler(SyscallType::RESOURCE_WRITE, syscall_resource_write);
	set_syscall_handler(SyscallType::RESOURCE_READ, syscall_resource_read);
	set_syscall_handler(SyscallType::THREAD_YIELD, syscall_thread_yield);
	set_syscall_handler(SyscallType::THREAD_SLEEP, syscall_thread_sleep);
	set_syscall_handler(SyscallType::THREAD_CLOSE, syscall_thread_close);

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
	m_current_args->arg0 = status->rdi;
	m_current_args->arg1 = status->rsi;
	m_current_args->arg2 = status->rdx;
	m_current_args->arg3 = status->r10;
	m_current_args->arg4 = status->r8;
	m_current_args->arg5 = status->r9;
	m_current_args->return_value = 0;
	m_current_args->return_state = status;

	// Call the handler
	uint64_t syscall = status->rax;
	if (m_syscall_handlers[syscall] != nullptr)
		m_current_args = m_syscall_handlers[syscall](m_current_args);
	else
		Logger::ERROR() << "Syscall " << syscall << " not found\n";

	// If there is a specific return state, use that
	if (m_current_args->return_state != status)
		return m_current_args->return_state;

	// Update the cpu state
	status->rdi = m_current_args->arg0;
	status->rsi = m_current_args->arg1;
	status->rdx = m_current_args->arg2;
	status->r10 = m_current_args->arg3;
	status->r8 = m_current_args->arg4;
	status->r9 = m_current_args->arg5;
	status->rax = m_current_args->return_value;


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

	m_syscall_handlers[(uint8_t) syscall] = handler;
}

/**
 * @brief Removes a syscall handler from the manager
 *
 * @param syscall The syscall ID number
 */
void SyscallManager::remove_syscall_handler(SyscallType syscall) {

	m_syscall_handlers[(uint8_t) syscall] = nullptr;
}


/**
 * @brief System call to close a process
 *
 * @param args Arg0 = pid Arg1 = exit code
 * @return Nothing
 */
syscall_args_t* SyscallManager::syscall_close_process(system::syscall_args_t* args) {

	// Get the args
	uint64_t pid = args->arg0;
	int exit_code = (int) args->arg1;

	// Close the process
	Process* process = pid == 0 ? GlobalScheduler::current_process() : GlobalScheduler::get_process(pid);
	GlobalScheduler::system_scheduler()->remove_process(process);

	// Schedule the next process
	cpu_status_t* next_process = GlobalScheduler::core_scheduler()->schedule_next(args->return_state);
	args->return_state = next_process;

	// Done
	return args;
}

syscall_args_t* SyscallManager::syscall_klog(syscall_args_t* args) {

	s_lock.lock();

	char* message = (char*) args->arg0;

	// If the first two characters are %h then no header
	if (message[0] == '%' && message[1] == 'h')
		Logger::Out() << message + 2;
	else
		Logger::INFO() << message;

	s_lock.unlock();

	return args;
}

/**
 * @brief System call to free a block of memory
 *
 * @param args Arg0 = size
 * @return The address of the memory block or null if failed
 */
syscall_args_t* SyscallManager::syscall_allocate_memory(syscall_args_t* args) {

	// Malloc the memory
	size_t size = args->arg0;
	void* address = MemoryManager::malloc(size);

	// Return the address
	args->return_value = (uint64_t)address;
	return args;
}

/**
 * @brief System call to free a block of memory
 *
 * @param args Arg0 = address
 * @return Nothing
 */
syscall_args_t* SyscallManager::syscall_free_memory(syscall_args_t* args) {

	// Free the memory
	void* address = (void*) args->arg0;
	MemoryManager::free(address);

	return args;
}

/**
 * @brief System call to create a resource
 *
 * @param args Arg0 = ResourceType Arg1 = Name Arg2 = Flags
 * @return 1 if resource was created sucessfully, 0 otherwise
 */
syscall_args_t* SyscallManager::syscall_resource_create(syscall_args_t* args) {

	// Parse params
	auto type 	= (ResourceType)args->arg0;
	auto name 	= (char*)args->arg1;
	auto flags 	= (size_t)args->arg2;

	// Try to create the resource
	auto resource = GlobalResourceRegistry::get_registry(type)->create_resource(name, flags);

	// Handle response
	args->return_value = resource ? 1 : 0;
	return args;
}

/**
 * @brief System call to open a resource
 *
 * @param args Arg0 = ResourceType Arg1 = Name Arg2 = Flags
 * @return The handle of the resource or 0 if failed
 */
syscall_args_t* SyscallManager::syscall_resource_open(syscall_args_t* args) {

	// Parse params
	auto type 	= (ResourceType)args->arg0;
	auto name 	= (char*)args->arg1;
	auto flags 	= (size_t)args->arg2;

	// Open the resource
	args->return_value = GlobalScheduler::current_process()->resource_manager.open_resource(type, name, flags);
	return args;
}

/**
 * @brief System call to close a resource
 *
 * @param args Arg0 = Handle Arg1 = Flags
 * @return Nothing
 */
syscall_args_t* SyscallManager::syscall_resource_close(syscall_args_t* args) {

	// Parse params
	auto handle	= (uint64_t )args->arg0;
	auto flags 	= (size_t)args->arg1;

	// Close the resource
	GlobalScheduler::current_process()->resource_manager.close_resource(handle, flags);
	return args;
}

/**
 * @brief System call to write to a resource
 *
 * @param args Arg0 = Handle Arg1 = Buffer Arg2 = Size Arg3 = Flags
 * @return The number of bytes written or 0 if failed
 */
syscall_args_t* SyscallManager::syscall_resource_write(syscall_args_t* args) {

	// Parse params
	auto handle	= (uint64_t )args->arg0;
	auto buffer = (void*)args->arg1;
	auto size 	= (size_t)args->arg2;
	auto flags 	= (size_t)args->arg3;

	// Open the resource
	auto resource = GlobalScheduler::current_process()->resource_manager.get_resource(handle);
	if(!resource){
		args->return_value = 0;
		return args;
	}

	// Write to the resource
	args->return_value = resource->write(buffer,size,flags);
	return args;
}

/**
 * @brief System call to read from a resource
 *
 * @param args Arg0 = Handle Arg1 = Buffer Arg2 = Size Arg3 = Flags
 * @return The number of bytes read or 0 if failed
 */
syscall_args_t* SyscallManager::syscall_resource_read(syscall_args_t* args) {

	// Parse params
	auto handle	= (uint64_t )args->arg0;
	auto buffer = (void*)args->arg1;
	auto size 	= (size_t)args->arg2;
	auto flags 	= (size_t)args->arg3;

	// Open the resource
	auto resource = GlobalScheduler::current_process()->resource_manager.get_resource(handle);
	if(!resource){
		args->return_value = 0;
		return args;
	}

	// Write to the resource
	args->return_value = resource->read(buffer,size,flags);
	return args;
}

/**
 * @brief System call to yield the current process
 *
 * @param args Nothing
 * @return Nothing
 */
syscall_args_t* SyscallManager::syscall_thread_yield(syscall_args_t* args) {

	// Yield
	args->return_state = GlobalScheduler::yield(args->return_state);

	return args;
}

/**
 * @brief System call to sleep the current process
 *
 * @param args Arg0 = milliseconds
 * @return Nothing
 */
syscall_args_t* SyscallManager::syscall_thread_sleep(syscall_args_t* args) {

	// Get the milliseconds
	size_t milliseconds = args->arg0;

	// Sleep the thread
	GlobalScheduler::current_thread()->sleep(milliseconds);
	args->return_state = GlobalScheduler::yield(args->return_state);

	// Done
	return args;
}

/**
 * @brief System call to close a thread
 *
 * @param args Arg0 = tid Arg1 = exit code
 * @return Nothing
 */
syscall_args_t* SyscallManager::syscall_thread_close(syscall_args_t* args) {

	// Get the args
	uint64_t tid = args->arg0;
	int exit_code = (int) args->arg1;

	// Get the thread if it is 0 then it is the current thread
	Thread* thread = tid == 0 ? GlobalScheduler::current_thread() : GlobalScheduler::get_thread(tid);
	thread->thread_state = ThreadState::STOPPED;

	// Schedule the next thread
	if(tid == 0){
		cpu_status_t* next_thread = GlobalScheduler::core_scheduler()->schedule_next(args->return_state);
		args->return_state = next_thread;
	}

	// Done
	return args;
}