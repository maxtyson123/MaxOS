/**
 * @file thread.h
 * @brief Defines the abstraction for thread management functions and structures
 *
 * @date 2nd December 2025
 * @author Max Tyson
 */

#ifndef MAXOS_KPI_PROCESSES_THREAD_H
#define MAXOS_KPI_PROCESSES_THREAD_H

#include <cstdint>
#include <cstddef>
#include <syscalls.h>

namespace MaxOS::KPI::processes {

	typedef struct ThreadStats {

		uint64_t tid;
		uint64_t pid;
		uint64_t ticks;
		size_t state;
		uint64_t wakeup_time_ms;

		uint64_t exit_code;

	} thread_stats_t;


	enum class ThreadFlags {
		READ_STATS,

		WRITE_SLEEP_TIME,
		WRITE_KILL,
	};

	// General thread functions

	uint64_t spawn_thread(uint64_t process_handle, void (* entry_point)(void*), void* args, size_t arg_amount);

	uint64_t get_thread(uint64_t tid);
	void close_thread_handle(uint64_t handle);

	void kill_thread_handle(uint64_t handle, uint64_t exit_code, bool close_handle = true);
	void kill_thread(uint64_t tid, uint64_t exit_code);

	thread_stats_t get_thread_stats_handle(uint64_t thread_handle);
	thread_stats_t get_thread_stats(uint64_t tid);

	void thread_sleep_handle(uint64_t thread_handle, uint64_t sleep_time_ms);
	void thread_sleep(uint64_t tid, uint64_t sleep_time_ms);

	// Current thread functions
	uint64_t get_current_thread();
	inline uint64_t m_current_thread_handle = 0;

	thread_stats_t get_current_thread_stats();
	inline thread_stats_t m_current_thread_stats = {};

	uint64_t tid();
	void thread_exit(uint64_t exit_code);
	void sleep(uint64_t sleep_time_ms);

} // MaxOS::KPI::processes

#endif //MAXOS_KPI_PROCESSES_THREAD_H
