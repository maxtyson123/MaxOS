//
// Created by 98max on 25/02/2025.
//

#include <processes/scheduler.h>
#include <common/logger.h>

using namespace MaxOS;
using namespace MaxOS::processes;
using namespace MaxOS::memory;
using namespace MaxOS::hardwarecommunication;
using namespace MaxOS::system;

Scheduler::Scheduler(Multiboot& multiboot)
: InterruptHandler(0x20),
  m_next_thread_index(0),
  m_active(false),
  m_ticks(0),
  m_next_pid(-1),
  m_next_tid(-1),
  m_shared_memory_registry(resource_type_t::SHARED_MEMORY),
  m_shared_messages_registry(resource_type_t::MESSAGE_ENDPOINT)
{

	// Set up the basic scheduler
	Logger::INFO() << "Setting up Scheduler \n";
	s_instance = this;

	// Set up the core map
	for (const auto& core : CPU::cores){
		auto id =core->id;

		// Create the core's idle
		auto* idle = new Process("Core Idle", nullptr, nullptr, 0, true);
		idle->memory_manager = MemoryManager::s_kernel_memory_manager;

		// Make sure it's the first thing scheduled
		m_core_map.insert(id, id);
		idle->set_pid(id);
	}


	// Load the elfs
	load_multiboot_elfs(&multiboot);

}

Scheduler::~Scheduler() {

	// Deactivate this scheduler
	s_instance = nullptr;
	m_active = false;
}

/**
 * @brief Handles the interrupt 0x20
 *
 * @param status The current CPU status
 * @return The new CPU status
 */
cpu_status_t* Scheduler::handle_interrupt(cpu_status_t* status) {

	return schedule(status);
}


/**
 * @brief Schedules the next thread to run
 *
 * @param cpu_state The current CPU state
 * @return The next CPU state
 */
cpu_status_t* Scheduler::schedule(cpu_status_t* cpu_state) {

	m_lock.lock();

	// Scheduler cant schedule anything
	if (m_threads.empty() || !m_active){
		m_lock.unlock();
		return cpu_state;
	}

	// Thread that we are dealing with
	Thread* current_thread = m_threads[m_core_map[CPU::executing_core()->id]];

	// Ticked
	m_ticks++;
	current_thread->ticks++;
	m_lock.unlock();

	// Thread hasn't used its time slot yet
	if(current_thread->ticks % s_ticks_per_event)
		return cpu_state;

	// Schedule the next thread
	return schedule_next(cpu_state);
}

/**
 * @brief Schedules the next thread to run
 *
 * @param status The current CPU status of the thread
 * @return The next CPU status
 */
cpu_status_t* Scheduler::schedule_next(cpu_status_t* cpu_state) {

	// Get the current state
	uint64_t core_id = CPU::executing_core()->id;
	Thread* current_thread = m_threads[m_core_map[core_id]];

	// Save the executing thread state
	current_thread->execution_state = cpu_state;
	current_thread->save_sse_state();
	if (current_thread->thread_state == ThreadState::RUNNING)
		current_thread->thread_state = ThreadState::READY;

	// Find a free thread to run (note: cant use modulos as don't want overlap)
	if (m_core_map[core_id] + m_core_map.size() < m_threads.size())
		m_core_map[core_id] += m_core_map.size();
	else
		m_core_map[core_id] = core_id;
	uint64_t thread_index = m_core_map[core_id];

	// Get the current thread
	current_thread = m_threads[thread_index];
	Process* owner_process = current_process();

	// Handle state changes
	switch (current_thread->thread_state) {

		case ThreadState::SLEEPING:

			// If the wake-up time hasn't occurred yet, run the next thread
			if (current_thread->wakeup_time > m_ticks){
				return schedule_next(current_thread->execution_state);
			}

			break;

		case ThreadState::STOPPED:

			// Find the process that has the thread and remove it
			for (auto thread: owner_process->threads()) {
				if (thread == current_thread) {
					owner_process->remove_thread(thread_index);
					break;
				}
			}

			// Remove the thread
			m_lock.lock();
			m_threads.erase(m_threads.begin() + thread_index);
			m_lock.unlock();

			// Run the next thread
			return schedule_next(cpu_state);

		default:
			break;
	}

	// Load the thread's state
	return load_process(owner_process, current_thread);
}

/**
 * @brief Load the thread specific state for the thread
 *
 * @param process The process that owns the thread
 * @param thread The thread to load
 * @return The cpu state to run with the next thread
 */
cpu_status_t* Scheduler::load_process(Process* process, Thread* thread) {

	// Prepare the next thread to run
	thread->thread_state = ThreadState::RUNNING;
	thread->restore_sse_state();

	// Load the thread's memory manager and task state
	MemoryManager::switch_active_memory_manager(process->memory_manager);
	CPU::executing_core() -> tss.rsp0 = thread->tss_pointer();

	return thread->execution_state;
}

/**
 * @brief Adds a process to the scheduler
 *
 * @param process The process to add
 * @return The process ID
 */
uint64_t Scheduler::add_process(Process* process) {

	m_lock.lock();

	// Get the next process ID
	m_next_pid++;

	// Add the process to the list
	m_processes.push_back(process);
	Logger::DEBUG() << "Adding process " << m_next_pid << ": " << process->name << "\n";

	// Return the process ID
	m_lock.unlock();
	return m_next_pid;
}

/**
 * @brief Adds a thread to the scheduler
 *
 * @param thread The thread to add
 * @return The thread ID
 */
uint64_t Scheduler::add_thread(Thread* thread) {

	m_lock.lock();

	// Get the next thread ID
	m_next_tid++;

	// Add the thread to the list
	m_threads.push_back(thread);
	Logger::DEBUG() << "Adding thread  " << m_next_tid << " to process " << thread->parent_pid << "\n";

	// Return the thread ID
	m_lock.unlock();
	return m_next_tid;
}

/**
 * @brief Gets the system scheduler
 *
 * @return The system scheduler or nullptr if not found
 */
Scheduler* Scheduler::system_scheduler() {

	return s_instance;
}

/**
 * @brief Gets how long the system has been running for
 *
 * @return The number of ticks
 */
uint64_t Scheduler::ticks() const {

	return m_ticks;
}

/**
 * @brief Pass execution to the next thread
 */
cpu_status_t* Scheduler::yield() {

	// If this is the only thread, can't yield
	if (m_threads.size() <= 1)
		return current_thread()->execution_state;

	// Set the current thread to waiting if running
	auto thread = current_thread();
	if (thread->thread_state == ThreadState::RUNNING)
		thread->thread_state = ThreadState::READY;

	// Schedule the next thread
	return schedule_next(thread->execution_state);
}

/**
 * @brief Activates the scheduler
 */
void Scheduler::activate() {

	m_active = true;
}

/**
 * @brief Removes a process from the scheduler if the process has no threads, if it does then the threads are stopped but the process is not removed (this will be done automatically when all threads are stopped)
 *
 * @param process The process to remove
 * @param force If true, the process will be removed and so will all threads
 * @return -1 if the process has threads, 0 otherwise
 */
uint64_t Scheduler::remove_process(Process* process) {

	m_lock.lock();

	// Check if the process has no threads
	if (!process->threads().empty()) {

		// Set the threads to stopped or remove them if forced
		for (auto thread: process->threads())
			thread->thread_state = ThreadState::STOPPED;

		// Need to wait until the threads are stopped before removing the process (this will be called again when all threads are stopped)
		m_lock.unlock();
		return -1;

	}

	// Remove the process
	for (uint32_t i = 0; i < m_processes.size(); i++) {
		if (m_processes[i] == process) {
			m_processes.erase(m_processes.begin() + i);

			// Delete the process mem
			delete process;
			m_lock.unlock();
			return 0;
		}
	}

	// Process not found
	m_lock.unlock();
	return -1;
}

/**
 * @brief Removes a process from the scheduler and deletes all threads, begins running the next process
 *
 * @param process The process to remove
 * @return The status of the CPU for the next process to run or nullptr if the process was not found
 */
cpu_status_t* Scheduler::force_remove_process(Process* process) {

	// If there is no process, fail
	if (!process)
		return nullptr;

	m_lock.lock();

	// Remove all the threads
	for (auto thread: process->threads()) {

		// Remove the thread from the scheduler
		int index = m_threads.find(thread) - m_threads.begin();
		m_threads.erase(m_threads.begin() + index);

		// Delete the thread
		process->remove_thread(thread->tid);

	}

	// Process will be dead now so run the next process (don't care about the execution state being outdated as it is being
	// removed regardless)
	m_lock.unlock();
	return schedule_next(current_thread()->execution_state);
}

/**
 * @brief Gets the current process
 *
 * @return The current process, or nullptr if not found
 */
Process* Scheduler::current_process() {

	Process* current_process = nullptr;

	// Make sure there is something with process attached
	if (!s_instance)
		return nullptr;

	// Find the process that has the thread being executed
	for (auto process: s_instance->m_processes)
		if (process->pid() == current_thread()->parent_pid) {
			current_process = process;
			break;
		}

	return current_process;
}

/**
 * @brief Gets a process by its PID
 *
 * @param pid The process ID
 * @return The process or nullptr if not found
 */
Process* Scheduler::get_process(uint64_t pid) {

	// Try to find the process
	for (auto process: s_instance->m_processes)
		if (process->pid() == pid)
			return process;

	// Not found
	return nullptr;
}


/**
 * @brief Gets the currently executing thread
 *
 * @return The currently executing thread
 */
Thread* Scheduler::current_thread() {

	if(!s_instance)
		return nullptr;

	s_instance->m_lock.lock();
	auto thread = s_instance->m_threads[s_instance->m_core_map[CPU::executing_core()->id]];
	s_instance->m_lock.unlock();

	return thread;
}

/**
 * @brief Deactivates the scheduler
 */
void Scheduler::deactivate() {

	m_active = false;
}

/**
 * @brief Loads any valid ELF files from the multiboot structure
 *
 * @param multiboot The multiboot structure
 */
void Scheduler::load_multiboot_elfs(Multiboot* multiboot) {

	for (multiboot_tag* tag = multiboot->start_tag(); tag->type != MULTIBOOT_TAG_TYPE_END; tag = (struct multiboot_tag*) ((multiboot_uint8_t*) tag + ((tag->size + 7) & ~7))) {

		// Tag is not an ELF
		if (tag->type != MULTIBOOT_TAG_TYPE_MODULE)
			continue;

		// Try to create the elf from the module
		auto* module = (struct multiboot_tag_module*) tag;
		auto* elf = new Elf64((uintptr_t) PhysicalMemoryManager::to_dm_region(module->mod_start));
		if (!elf->is_valid())
			continue;

		Logger::DEBUG() << "Creating process from multiboot module for " << module->cmdline << " (at 0x" << (uint64_t) module->mod_start << ")\n";

		// Create an array of args for the process TODO: handle multiple args ("" & spaces)
		char* args[1] = {module->cmdline};

		// Create the process
		auto* process = new Process(module->cmdline, args, 1, elf);

		Logger::DEBUG() << "Elf loaded to pid " << process->pid() << "\n";
		delete elf;
	}
}

/**
 * @brief Gets a thread by its TID
 *
 * @param tid The thread ID
 * @return The thread or nullptr if not found
 */
Thread* Scheduler::get_thread(uint64_t tid) {

	// Try to find the thread
	for (auto thread: s_instance->m_threads)
		if (thread->tid == tid)
			return thread;

	return nullptr;
}

/**
 * @brief Get the header for the running processes
 *
 * @return a string in the form ({name}:t{tid}c{core})
 */
void Scheduler::print_running_header() {

	// No threads or processes to get
	if(!s_instance || !s_instance->m_active)
		return;

	auto process = current_process();
	auto thread   = current_thread();

	Logger::Out() << "(" << process->name << ":t" << thread->tid  << "c" << CPU::executing_core()->id << ") ";
}
