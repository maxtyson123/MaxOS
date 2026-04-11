/**
 * @file scheduler_resources.h
 * @brief Implements scheduler resource classes for wrapping Processe and Thread objects as Resources in the Global Scheduler,
 * allowing scheduler operations to be performed through the Resource interface.
 *
 * @date 1st December 2025
 * @author Max Tyson
 */

#include <processes/resources/scheduler_resources.h>
#include <processes/scheduler.h>

using namespace MaxOS;
using namespace MaxOS::processes;
using namespace MaxOS::processes::resources;
using namespace MaxOS::memory;
using namespace MaxOS::common;
using namespace MaxOS::KPI::processes;

ProcessResource::ProcessResource(string const& name, size_t flags, resource_type_t type)
: Resource(name, flags, type)
{

}

/**
 * @brief Closes the process resource, killing the process and removing it from the scheduler
 *
 * @param flags N/A
 */
void ProcessResource::close(size_t flags) {

	if(process)
		GlobalScheduler::remove_process(process);
}

/**
 * @brief Reads data from the process resource
 *
 * @param buffer The buffer to read into
 * @param size The number of bytes to read
 * @param flags The flags to pass to the reading
 *
 * @return The number of bytes successfully read or -1 on error
 */
int ProcessResource::read(void* buffer, size_t size, size_t flags) {

	// Process not found (allow reading stats only to get exit code)
	if(!process && flags != (size_t)ProcessFlags::READ_STATS)
		return -1;

	switch ((ProcessFlags)flags) {

		case ProcessFlags::READ_STATS: {

			// Check bounds
			if(size < sizeof(process_stats_t))
				return -1;

			// Get the info
			process_stats_t stats = {};
			if(process) {
				stats.pid = process->pid();
				stats.parent_pid = 0; //TODO: store parent pid
				stats.thread_count = process->threads().size();
				stats.total_ticks = process->total_ticks();
				stats.memory_usage = 0; //process->memory_manager->memory_used(); TODO: boken?
				stats.handle_count = process->resource_manager.resources().size();
			}
			stats.exit_code = exit_code;

			// Copy across
			memcpy(buffer, &stats, sizeof(process_stats_t));
			return sizeof(process_stats_t);

		}

		case ProcessFlags::READ_ENVIRONMENT: {

			// Check bounds
			if(size < sizeof(process_environment_t))
				return -1;

			// Get the info
			process_environment_t info;
			info.args = 0;
			info.arg_count = 0; //TODO: store
			info.env_vars = 0;  //TODO: implement
			info.env_var_count = 0;
			info.working_directory = (const char*)MemoryManager::malloc(process->working_directory.length() + 1);
			info.executable_path = 0;// TODO: store

			// Copy the info
			// .. args
			// .. env vars
			memcpy((void*)info.working_directory, process->working_directory.c_str(), process->working_directory.length() + 1);
			// .. executable path

			// Copy across
			memcpy(buffer, &info, sizeof(process_environment_t));
			return sizeof(process_environment_t);

		}


		case ProcessFlags::READ_HANDLES: {

			// Check bounds
			size_t size_needed = sizeof(handle_info_t) * process->resource_manager.resources().size();
			if(size < size_needed)
				return -1;

			for(const auto& resource : process->resource_manager.resources()) {

				// Get the info
				handle_info_t handle_info;
				handle_info.handle = resource.first;
				handle_info.type = resource.second->type();
				handle_info.name = (const char*)MemoryManager::malloc(resource.second->name().length() + 1);
				memcpy((void*)handle_info.name, resource.second->name().c_str(), resource.second->name().length() + 1);

				// Copy across
				memcpy(buffer, &handle_info, sizeof(handle_info_t));
				buffer = (void*)((uintptr_t)buffer + sizeof(handle_info_t));
			}

			return (int)size_needed;

		}

		default:
			return -1;
	}
}

/**
 * @brief Writes data to the process resource
 *
 * @param buffer The buffer to write from
 * @param size The number of bytes to write
 * @param flags The flags to pass to the writing
 *
 * @return The number of bytes successfully written or -1 on error
 */
int ProcessResource::write(const void* buffer, size_t size, size_t flags) {

	// Process not found
	if(!process)
		return -1;

	switch ((ProcessFlags)flags) {

		case ProcessFlags::WRITE_CHANGE_DIRECTORY: {

			// Change the working directory
			string new_directory = string((const char*)buffer);
			process->working_directory = new_directory;
			return size;
		}

		case ProcessFlags::WRITE_KILL: {

			// Kill the process
			exit_code = *((uint64_t*)buffer);
			GlobalScheduler::remove_process(process);
			process->threads()[0]->yield();

			return size;
		}

		default:
			return -1;
	}
}

ProcessResource::~ProcessResource() = default;


ThreadResource::ThreadResource(string const& name, size_t flags, resource_type_t type)
: Resource(name, flags, type)
{

}

/**
 * @brief Closes the thread resource, marking the thread to be removed by the scheduler
 *
 * @param flags N/A
 */
void ThreadResource::close(size_t flags) {

	thread->thread_state = ThreadState::STOPPED;
}

/**
 * @brief Reads data from the thread resource
 *
 * @param buffer The buffer to read into
 * @param size The number of bytes to read
 * @param flags The flags to pass to the reading
 *
 * @return The number of bytes successfully read or -1 on error
 */
int ThreadResource::read(void* buffer, size_t size, size_t flags) {

	// Thread not found
	if(!thread && flags != (size_t)ThreadFlags::READ_STATS)
		return -1;

	switch ((ThreadFlags)flags) {

		case ThreadFlags::READ_STATS: {

			// Check bounds
			if(size < sizeof(thread_stats_t))
				return -1;

			// Get the info
			thread_stats_t stats = {};

			if(thread) {
				stats.tid = thread->tid;
				stats.pid = thread->parent_pid;
				stats.ticks = thread->ticks;
				stats.state = (size_t)thread->thread_state;
				stats.wakeup_time_ms = thread->wakeup_time;
			}
			stats.exit_code = exit_code;

			// Copy across
			memcpy(buffer, &stats, sizeof(thread_stats_t));
			return sizeof(thread_stats_t);

		}

		default:
			return -1;

	}

}

/**
 * @brief Writes data to the thread resource
 *
 * @param buffer The buffer to write from
 * @param size The number of bytes to write
 * @param flags The flags to pass to the writing
 *
 * @return The number of bytes successfully written or -1 on error
 */
int ThreadResource::write(const void* buffer, size_t size, size_t flags) {

	// Thread not found
	if(!thread)
		return -1;

	switch ((ThreadFlags)flags) {

		case ThreadFlags::WRITE_SLEEP_TIME: {

			// Set the sleep time
			uint64_t sleep_time = *((uint64_t*)buffer);
			thread->sleep(sleep_time);
			return size;
		}

		case ThreadFlags::WRITE_KILL: {

			// Kill the thread
			exit_code = *((uint64_t*)buffer);
			thread->thread_state = ThreadState::STOPPED;
			thread->yield();
			return size;
		}

		case ThreadFlags::WRITE_YIELD: {
			thread->yield();
			return size;
		}

		case ThreadFlags::WRITE_ENABLE_PORT_IO: {
			uint64_t port = *((uint64_t*)buffer);
			thread->enable_port(port);
			return size;
		}

		case ThreadFlags::WRITE_DISABLE_PORT_IO: {
			uint64_t port = *((uint64_t*)buffer);
			thread->disable_port(port);
			return size;
		}

		default:
			return -1;
	}

}

ThreadResource::~ThreadResource() = default;

ThreadResourceRegistry::ThreadResourceRegistry()
: ResourceRegistry<ThreadResource>(resource_type_t::THREAD)
{

}

/**
 * @brief td
 * @param name td
 * @param flags td
 * @return td
 *
 * @todo Implement opening threads
 */
Resource* ThreadResourceRegistry::create_resource(string const& name, size_t flags, uintptr_t data) {

	return ResourceRegistry::create_resource(name, flags, data);
}

Resource* ThreadResourceRegistry::get_resource(string const& name) {

	// Resource already opened
	auto resource = BaseResourceRegistry::get_resource(name);
	if(resource != nullptr)
		return resource;

	// Get the thread
	uint64_t tid = name.to_int();
	auto thread = name == "this" ? GlobalScheduler::current_thread() : GlobalScheduler::get_thread(tid);

	// Self refrence may already be open
	resource = BaseResourceRegistry::get_resource(string(thread -> tid));
	if(resource != nullptr)
		return resource;

	// Create the resource
	auto thread_resource = new ThreadResource(string(thread -> tid), 0, resource_type_t::PROCESS);
	thread_resource->thread = thread;

	register_resource(thread_resource);
	return thread_resource;
}

ThreadResourceRegistry::~ThreadResourceRegistry() = default;

ProcessResourceRegistry::ProcessResourceRegistry()
: ResourceRegistry<ProcessResource>(resource_type_t::PROCESS)
{

}

/**
 * @brief Opens an executable
 *
 * @param name The path to the executable to open
 * @param flags td
 * @return The Process resource created from the executable or nullptr if failed to open/execute
 *
 * @todo Process needs to own the Elf pointer and free it in destructor (do this when moving ELF as a superclass)
 */
Resource* ProcessResourceRegistry::create_resource(string const& name, size_t flags, uintptr_t data) {

	// Get the command
	auto command = (execute_command_t*)data;
	if (!command)
		return nullptr;

	// Kernel copy (@todo copy the data page into the new process so dont get unmapped error and also thus dont need a copy)
	auto exec_buffer = buffer_t(command->file_data_size);
	exec_buffer.copy_from(command->file_data, command->file_data_size);

	// Copy the args into userspace (@todo bad, two copies)
	uint64_t argc = command->args_count;
	size_t arg_size = argc * sizeof(void*);
	void* argv = MemoryManager::kmalloc(arg_size);
	for (int i = 0; i < argc; i++) {

		// Copy each argument
		size_t len = strlen(((char**)command->args)[i]) + 1;
		((char**)argv)[i] = (char*) MemoryManager::kmalloc(len);
		memcpy((void*) ((char**)argv)[i], (void*) ((char**)command->args)[i], len);
	}

	// Load the elf
	auto elf = new ELF64((uintptr_t)exec_buffer.raw());
	if (!elf->is_valid())
		return nullptr;

	// Create the processs
	auto process	= new Process(name, argv, argc, elf);
	GlobalScheduler::system_scheduler() -> add_process(process);

	// Create the resource
	auto resource 	= new ProcessResource(name, flags, resource_type_t::PROCESS);
	resource->process = process;

	// Clean up
	// for (int i = 0; i < argc; i++)
	// 	delete ((char**)argv)[i];
	// delete argv;


	// Switch back to the caller proc
	MemoryManager::switch_active_memory_manager(GlobalScheduler::current_process()->memory_manager);
	return resource;
}

/**
 * @brief The
 *
 * @param name
 * @return
 *
 * @todo 'this' is a bit mess
 */
Resource* ProcessResourceRegistry::get_resource(string const& name) {

	// Resource already opened
	auto resource = BaseResourceRegistry::get_resource(name);
	if(resource != nullptr)
		return resource;

	// Get the process
	uint64_t pid = name.to_int();
	auto process = name == "this" ? GlobalScheduler::current_process() : GlobalScheduler::get_process(pid);

	// Self refrence may already be open
	resource = BaseResourceRegistry::get_resource(string(process -> pid()));
	if(resource != nullptr)
		return resource;

	// Create the resource
	auto process_resource = new ProcessResource(string(process -> pid()), 0, resource_type_t::PROCESS);
	process_resource->process = process;

	register_resource(process_resource);
	return process_resource;
}

ProcessResourceRegistry::~ProcessResourceRegistry() = default;