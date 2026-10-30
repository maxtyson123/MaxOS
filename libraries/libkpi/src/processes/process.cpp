/**
 * @file process.cpp
 * @brief Implements the abstraction for process management functions and structures
 *
 * @date 2nd December 2025
 * @author Max Tyson
 */

#include <libkpi/processes/process.h>
#include <libkpi/processes/thread.h>

#include <libfs/file.h>

using namespace MaxOS::KPI;
using namespace processes;

/**
 * @brief Executes a file as a new process
 *
 * @param name The name of the process
 * @param path The path to the executable file
 * @param wait_for_completion True to wait for the process to exit, false to return immediately
 * @return The process ID of the new process, or the exit code if wait_for_completion is true
 */
uint64_t processes::exec_file(const char* name, const char *path, bool wait_for_completion) {

	return exec_file(name, path, nullptr, 0, wait_for_completion);

}

/**
 * @brief Executes a file as a new process
 *
 * @param name The name of the process
 * @param path The path to the executable file
 * @param args The arguments to pass to the executable
 * @param arg_amount The number of arguments
 * @param wait_for_completion True to wait for the process to exit, false to return immediately
 * @return The process ID of the new process, or the exit code if wait_for_completion is true
 *
 * @todo What if someone exec_file but not_wait and then never close the handle, will resource leak - should auto close when process ends?
 */
uint64_t processes::exec_file(const char* name, const char* path, const char** args, size_t arg_amount, bool wait_for_completion) {

	// Add the path as the first argument
	const char** full_args =  new const char*[arg_amount + 1];
	full_args[0] = path;
	for (size_t i = 0; i < arg_amount; i++)
		full_args[i + 1] = args[i];

	// Open the file
	auto handle = LibFS::open_file(path);
	if (!handle)
		return 0;

	// Load the ELF blob
	size_t size = LibFS::file_size(handle);
	auto buffer = new uint8_t[size];
	LibFS::file_read(handle, buffer, size);

	// Create the process
	execute_command_t command = {
		.args = full_args,
		.args_count = arg_amount + 1,
		.file_data = buffer,
		.file_data_size = size,
	};
	uint64_t result = resource_create(ResourceType::PROCESS, name, 0, &command);

	// Clean up
	delete[] full_args;
	delete[] buffer;
	LibFS::close_file(handle);

	// Wait for completion if requested @todo move this the scheduler resource create place so that it can be blocked properly
	if (wait_for_completion && result > 0){

		process_stats_t stats = get_process_stats_handle(result);

		// If the process is killed it wont have a PID
		while(stats.pid != 0) {
			yield();
			stats = get_process_stats_handle(result);
		}

		close_process_handle(result);
		return stats.exit_code;
	}

	return result;
}

/**
 * @brief Gets a handle to a process by its PID
 *
 * @param pid The PID of the process
 * @return A handle to the process
 */
uint64_t processes::get_process(uint64_t pid) {

	char pid_str[21];
	//int to str
	return resource_open(ResourceType::PROCESS, pid_str, 0);
}

/**
 * @brief Closes a process handle. Will not terminate the process itself.
 *
 * @param handle The handle to close
 */
void processes::close_process_handle(uint64_t handle) {
	resource_close(handle, 0);
}

/**
 * @brief Kills a process by its handle
 *
 * @param handle The handle of the process to kill
 * @param exit_code The exit code to return to the parent process
 * @param close_handle True to close the handle after killing the process (default: true)
 */
void processes::kill_process_handle(uint64_t handle, uint64_t exit_code, bool close_handle) {

	resource_write(handle, &exit_code, sizeof(exit_code), (size_t)ProcessFlags::WRITE_KILL);

	if (close_handle)
		close_process_handle(handle);
}

/**
 * @brief Kills a process by its PID
 *
 * @param pid The PID of the process to kill
 * @param exit_code The exit code to return to the parent process
 * @param close_handle True to close the handle after killing the process (default: true)
 */
void processes::kill_process(uint64_t pid, uint64_t exit_code) {

	kill_process_handle(get_process(pid), exit_code);

}

/**
 * @brief Gets the statistics of a process by its handle
 *
 * @param handle The handle of the process
 * @return The statistics of the process
 */
process_stats_t processes::get_process_stats_handle(uint64_t handle) {

	process_stats_t stats;
	resource_read(handle, &stats, sizeof(process_stats_t), (size_t)ProcessFlags::READ_STATS);
	return stats;
}

/**
 * @brief Gets the statistics of a process by its PID
 *
 * @param pid The PID of the process
 * @return The statistics of the process
 */
process_stats_t processes::get_process_stats(uint64_t pid) {

	uint64_t handle = get_process(pid);
	process_stats_t stats = get_process_stats_handle(handle);
	close_process_handle(handle);
	return stats;

}

/**
 * @brief Gets the environment of a process by its handle
 *
 * @param handle The handle of the process
 * @return The environment of the process
 */
process_environment_t processes::get_process_environment_handle(uint64_t handle) {

	process_environment_t env;
	resource_read(handle, &env, sizeof(process_environment_t), (size_t)ProcessFlags::READ_ENVIRONMENT);
	return env;

}

/**
 * @brief Gets the environment of a process by its PID
 *
 * @param pid The PID of the process
 * @return The environment of the process
 */
process_environment_t processes::get_process_environment(uint64_t pid) {

	uint64_t handle = get_process(pid);
	process_environment_t env = get_process_environment_handle(handle);
	close_process_handle(handle);
	return env;

}

/**
 * @brief Changes the working directory of a process
 *
 * @param pid The PID of the process
 * @param path The new working directory path
 */
void processes::change_working_directory_handle(uint64_t handle, const char* path) {

	resource_write(handle, path, strlen(path) + 1, (size_t)ProcessFlags::WRITE_CHANGE_DIRECTORY);

}

/**
 * @brief Changes the working directory of a process by its PID
 *
 * @param pid The PID of the process
 * @param path The new working directory path
 */
void processes::change_working_directory_pid(uint64_t pid, const char* path) {

	uint64_t handle = get_process(pid);
	change_working_directory_handle(handle, path);
	close_process_handle(handle);
}


/**
 * @brief Gets a handle to the current process
 *
 * @return A handle to the current process
 */
uint64_t processes::get_current_process() {

	if (m_current_process_handle == 0)
		m_current_process_handle = resource_open(ResourceType::PROCESS, "this", 0);

	return m_current_process_handle;
}

/**
 * @brief Gets the statistics of the current process
 *
 * @return The statistics of the current process
 */
process_stats_t processes::get_current_process_stats() {

	m_current_process_stats = get_process_stats_handle(get_current_process());
	return m_current_process_stats;
}

/**
 * @brief Gets the environment of the current process
 *
 * @return The environment of the current process
 */
process_environment_t processes::get_current_process_environment() {

	m_current_process_environment = get_process_environment_handle(get_current_process());
	return m_current_process_environment;
}

/**
 * @brief Gets the PID of the current process
 *
 * @return The PID of the current process
 */
uint64_t processes::pid() {

	// Haven't got current process stats yet, get them
	if (m_current_process_stats.pid == 0)
		get_current_process_stats();

	return m_current_process_stats.pid;
}

/**
 * @brief Exits the current process
 *
 * @param exit_code The exit code to return to the parent process
 */
void processes::exit(uint64_t exit_code) {

	kill_process_handle(get_current_process(), exit_code);
}

/**
 * @brief Changes the working directory of the current process
 *
 * @param path The new working directory path
 */
void processes::change_working_directory(const char* path) {

	change_working_directory_handle(get_current_process(), path);
}