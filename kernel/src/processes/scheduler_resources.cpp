/**
 * @file scheduler_resources.h
 * @brief Implements scheduler resource classes for wrapping Processe and Thread objects as Resources in the Global Scheduler,
 * allowing scheduler operations to be performed through the Resource interface.
 *
 * @date 1st December 2025
 * @author Max Tyson
 */

#include <processes/scheduler_resources.h>
#include <processes/scheduler.h>

using namespace MaxOS;
using namespace MaxOS::processes;
using namespace MaxOS::memory;
using namespace syscore::processes;

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
				stats.memory_usage = process->memory_manager->memory_used();
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
Resource* ThreadResourceRegistry::create_resource(string const& name, size_t flags) {

	return ResourceRegistry::create_resource(name, flags);
}

Resource* ThreadResourceRegistry::get_resource(string const& name) {

	// Resource already opened
	auto resource = BaseResourceRegistry::get_resource(name);
	if(resource != nullptr)
		return resource;

	uint64_t tid = name.to_int();

	// Create the resource
	auto thread = new ThreadResource(name, 0, resource_type_t::PROCESS);
	thread->thread = name == "this" ? GlobalScheduler::current_thread() : GlobalScheduler::get_thread(tid);

	register_resource(resource);
	return resource;
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
 * @todo Implement opening executables
 */
Resource* ProcessResourceRegistry::create_resource(string const& name, size_t flags) {

	return ResourceRegistry::create_resource(name, flags);
}

/**
 * @brief The
 *
 * @param name
 * @return
 */
Resource* ProcessResourceRegistry::get_resource(string const& name) {

	// Resource already opened
	auto resource = BaseResourceRegistry::get_resource(name);
	if(resource != nullptr)
		return resource;

	uint64_t pid = name.to_int();

	// Create the resource
	auto process = new ProcessResource(name, 0, resource_type_t::PROCESS);
	process->process = name == "this" ? GlobalScheduler::current_process() : GlobalScheduler::get_process(pid);

	register_resource(resource);
	return resource;
}

ProcessResourceRegistry::~ProcessResourceRegistry() = default;