/**
 * @file syscalls.cpp
 * @brief Implementation of System Call Manager and handlers
 *
 * @date 27th October 2022
 * @author Max Tyson
 */

#include <system/syscalls.h>
#include <common/logger.h>

using namespace MaxOS;
using namespace MaxOS::KPI;
using namespace MaxOS::common;
using namespace MaxOS::hardwarecommunication;
using namespace MaxOS::system;
using namespace MaxOS::processes;
using namespace MaxOS::memory;

/**
 * @brief Construct a new Syscall Manager object and register the syscall handlers. Registers to interrupt 0x80
 */
SyscallManager::SyscallManager()
: InterruptHandler(0x80)
{

	// Register the handlers
	Logger::INFO() << "Setting up Syscalls \n";

	set_syscall_handler(SyscallType::KLOG, syscall_klog);

	set_syscall_handler(SyscallType::ALLOCATE_MEMORY, syscall_allocate_memory);
	set_syscall_handler(SyscallType::FREE_MEMORY, syscall_free_memory);

	set_syscall_handler(SyscallType::RESOURCE_CREATE, syscall_resource_create);
	set_syscall_handler(SyscallType::RESOURCE_OPEN, syscall_resource_open);
	set_syscall_handler(SyscallType::RESOURCE_CLOSE, syscall_resource_close);
	set_syscall_handler(SyscallType::RESOURCE_WRITE, syscall_resource_write);
	set_syscall_handler(SyscallType::RESOURCE_READ, syscall_resource_read);

	set_syscall_handler(SyscallType::YEILD, syscall_yield);

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
	syscall_args_t args;
	args.arg0 = status->rdi;
	args.arg1 = status->rsi;
	args.arg2 = status->rdx;
	args.arg3 = status->r10;
	args.arg4 = status->r8;
	args.arg5 = status->r9;
	args.return_value = 0;
	args.return_state = status;

	// Call the handler
	uint64_t syscall = status->rax;
	if (m_syscall_handlers[syscall] != nullptr)
		args = *(m_syscall_handlers[syscall](&args));
	else
		Logger::ERROR() << "Syscall " << syscall << " not found\n";

	// If there is a specific return state, use that
	if (args.return_state != status)
		return args.return_state;

	// Update the cpu state
	status->rdi = args.arg0;
	status->rsi = args.arg1;
	status->rdx = args.arg2;
	status->r10 = args.arg3;
	status->r8 = args.arg4;
	status->r9 = args.arg5;
	status->rax = args.return_value;

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
 * @brief System call to log a message to the kernel log
 *
 * @param args Arg0 = message
 * @return The same args structure
 */
syscall_args_t* SyscallManager::syscall_klog(syscall_args_t* args) {

	s_lock.lock();

	// Ensure a message was provided
	char* message = (char*) args->arg0;
	if(!message){
		s_lock.unlock();
		return args;
	}

	// If the first two characters are %h then no header
	if (message[0] == '%' && message[1] == 'h')
		Logger::Out() << message + 2;
	else
		Logger::INFO() << message;

	s_lock.unlock();

	return args;
}

/**
 * @brief System call to allocate a block of memory
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
	auto type 	= (resource_type_t)args->arg0;
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
	auto type 	= (resource_type_t)args->arg0;
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
syscall_args_t* SyscallManager::syscall_yield(syscall_args_t* args) {

	// Yield
	args->return_state = GlobalScheduler::yield(args->return_state);

	return args;
}