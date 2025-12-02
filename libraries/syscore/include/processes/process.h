/**
 * @file process.h
 * @brief Defines the abstraction for process management functions and structures
 *
 * @note PID means Process ID, not to be confused with handles.
 *
 * @date 2nd December 2025
 * @author Max Tyson
 */
#ifndef SYSCORE_PROCESSES_PROCESS_H
#define SYSCORE_PROCESSES_PROCESS_H

#include <cstdint>
#include <cstddef>

#include <syscalls.h>
#include <common.h>

namespace syscore::processes {

	/**
	 * @struct ProcessStats
	 * @brief Structure containing various statistics about a process
	 *
	 * @typedef process_stats_t
	 * @brief Alias for struct ProcessStats
	 *
	 * @note PID 0 here means process is not running as PID 0 is reserved for the system idle process and thus no point in user processes needing to access it.
	 */
	typedef struct ProcessStats {

		uint64_t pid;
		uint64_t parent_pid;
		uint64_t thread_count;
		uint64_t total_ticks;
		uint64_t memory_usage;
		uint64_t handle_count;

		uint64_t exit_code;

	} process_stats_t;

	typedef struct ProcessEnvironment {

		const char** args;
		size_t arg_count;
		const char** env_vars;
		size_t env_var_count;
		const char* working_directory;
		const char* executable_path;

	} process_environment_t;

	typedef struct handle_info {

		uint64_t handle;
		ResourceType type;
		const char* name;

	} handle_info_t;

	enum class ProcessFlags {
		READ_STATS,
		READ_ENVIRONMENT,
		READ_HANDLES,

		WRITE_CHANGE_DIRECTORY,
		WRITE_KILL
	};

	// General process functions

	uint64_t exec_file(const char* path, const char** args, size_t arg_amount, bool wait_for_completion);

	uint64_t get_process(uint64_t pid);
	void close_process_handle(uint64_t handle);

	void kill_process_handle(uint64_t handle, uint64_t exit_code, bool close_handle = true);
	void kill_process(uint64_t pid, uint64_t exit_code);

	process_stats_t get_process_stats_handle(uint64_t handle);
	process_stats_t get_process_stats(uint64_t pid);

	process_environment_t get_process_environment_handle(uint64_t handle);
	process_environment_t get_process_environment(uint64_t pid);

	//TODO: get_handles

	void change_working_directory_handle(uint64_t handle, const char* path);
	void change_working_directory_pid(uint64_t pid, const char* path);

	// Own process functions
	uint64_t get_current_process();
	inline uint64_t m_current_process_handle = 0;

	process_stats_t get_current_process_stats();
	inline process_stats_t m_current_process_stats = {};

	process_environment_t get_current_process_environment();
	inline process_environment_t m_current_process_environment = {};

	uint64_t pid();
	void exit(uint64_t exit_code);
	void change_working_directory(const char* path);

}

#endif //SYSCORE_PROCESSES_PROCESS_H
