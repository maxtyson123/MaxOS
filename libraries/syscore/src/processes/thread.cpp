/**
 * @file thread.cpp
 * @brief Implements the abstraction for thread management functions and structures
 *
 * @date 2nd December 2025
 * @author Max Tyson
 */

#include <processes/thread.h>

using namespace syscore;
using namespace syscore::processes;

/**
 * @brief Spawns a new thread in the specified process
 *
 * @param process_handle The process that will own the new thread
 * @param entry_point Where in the process the thread will start executing
 * @param args The arguments to pass to the thread entry point
 * @param arg_amount The number of arguments being passed
 * @return The handle of the newly created thread
 *
 * @todo Implement
 */
uint64_t syscore::processes::spawn_thread(uint64_t process_handle, void (* entry_point)(void*), void* args, size_t arg_amount) {

}

/**
 * @brief Gets a handle to a thread by its thread ID
 *
 * @param tid The thread ID of the thread to retrieve
 * @return The handle of the thread
 */
uint64_t syscore::processes::get_thread(uint64_t tid) {

	char tid_str[21];
	//int to str
	return resource_open(ResourceType::THREAD, tid_str, 0);

}

/**
 * @brief Closes a thread handle. Will not terminate the thread itself.
 *
 * @param handle The handle to close
 */
void syscore::processes::close_thread_handle(uint64_t handle) {

	resource_close(handle, 0);

}

/**
 * @brief Kills a thread by its handle
 *
 * @param handle The handle of the thread to kill
 * @param exit_code The exit code to return to the owning process
 * @param close_handle True to close the handle after killing the thread (default: true)
 */
void syscore::processes::kill_thread_handle(uint64_t handle, uint64_t exit, bool close_handle) {

	resource_write(handle, &exit, sizeof(exit), (size_t)ThreadFlags::WRITE_KILL);

	if (close_handle)
		close_thread_handle(handle);

}

/**
 * @brief Kills a thread by its thread ID
 *
 * @param tid The thread ID of the thread to kill
 * @param exit_code The exit code to return to the owning process
 */
void syscore::processes::kill_thread(uint64_t tid, uint64_t exit_code) {
	kill_thread_handle(get_thread(tid), exit_code);
}

/**
 * @brief Gets the statistics of a thread by its handle
 *
 * @param thread_handle The handle of the thread
 * @return The statistics of the thread
 */
thread_stats_t syscore::processes::get_thread_stats_handle(uint64_t thread_handle) {

	thread_stats_t stats;
	resource_read(thread_handle, &stats, sizeof(thread_stats_t), (size_t)ThreadFlags::READ_STATS);
	return stats;

}

/**
 * @brief Gets the statistics of a thread by its thread ID
 *
 * @param tid The thread ID of the thread
 * @return The statistics of the thread
 */
thread_stats_t syscore::processes::get_thread_stats(uint64_t tid) {

	uint64_t handle = get_thread(tid);
	thread_stats_t stats = get_thread_stats_handle(handle);
	close_thread_handle(handle);
	return stats;

}

/**
 * @brief Sets a thread to sleep for a specified amount of time by its handle
 *
 * @param thread_handle The handle of the thread
 * @param sleep_time_ms The amount of time to sleep in milliseconds
 */
void syscore::processes::thread_sleep_handle(uint64_t thread_handle, uint64_t sleep_time_ms) {

	resource_write(thread_handle, &sleep_time_ms, sizeof(sleep_time_ms), (size_t)ThreadFlags::WRITE_SLEEP_TIME);

}

/**
 * @brief Sets a thread to sleep for a specified amount of time by its thread ID
 *
 * @param tid The thread ID of the thread
 * @param sleep_time_ms The amount of time to sleep in milliseconds
 */
void syscore::processes::thread_sleep(uint64_t tid, uint64_t sleep_time_ms) {

	uint64_t handle = get_thread(tid);
	thread_sleep_handle(handle, sleep_time_ms);
	close_thread_handle(handle);

}

/**
 * @brief Gets a handle to the current thread
 *
 * @return A handle to the current thread
 */
uint64_t syscore::processes::get_current_thread() {

	if (m_current_thread_handle == 0)
		m_current_thread_handle = resource_open(ResourceType::THREAD, "this", 0);

	return m_current_thread_handle;

}

/**
 * @brief Gets the statistics of the current thread
 *
 * @return The statistics of the current thread
 */
thread_stats_t syscore::processes::get_current_thread_stats() {

	if (m_current_thread_stats.tid == 0)
		m_current_thread_stats = get_thread_stats_handle(get_current_thread());

	return m_current_thread_stats;
}

/**
 * @brief Gets the thread ID of the current thread
 *
 * @return The thread ID of the current thread
 */
uint64_t syscore::processes::tid() {

	// Ensure current thread stats are loaded
	if (m_current_thread_stats.tid == 0)
		get_current_thread_stats();

	return m_current_thread_stats.tid;

}

/**
 * @brief Exits the current thread
 *
 * @param exit_code The exit code to return to the owning process
 */
void syscore::processes::thread_exit(uint64_t exit_code) {

	resource_write(get_current_thread(), &exit_code, sizeof(exit_code), (size_t)ThreadFlags::WRITE_KILL);
	yeild();

}

/**
 * @brief Sets the current thread to sleep for a specified amount of time
 *
 * @param sleep_time_ms The amount of time to sleep in milliseconds
 */
void syscore::processes::sleep(uint64_t sleep_time_ms) {

	resource_write(get_current_thread(), &sleep_time_ms, sizeof(sleep_time_ms), (size_t)ThreadFlags::WRITE_SLEEP_TIME);

}