/**
 * @file scheduler.cpp
 * @brief Implementation of a Global Scheduler for managing processes and threads
 *
 * @date 25th February 2025
 * @author Max Tyson
 */

#include <processes/scheduler.h>
#include <common/logger.h>
#include <runtime/gdbstub.h>

using namespace MaxOS;
using namespace MaxOS::common;
using namespace MaxOS::processes;
using namespace MaxOS::processes::resources;
using namespace MaxOS::memory;
using namespace MaxOS::hardwarecommunication;
using namespace MaxOS::system;
using namespace MaxOS::runtime;
using namespace MaxOS::console;

/**
 * @brief Constructs a new Global Scheduler object. Registers as the interrupt handler for interrupt 0x20 and setups
 * the shared resource registries. Loads any ELF files from the multiboot structure.
 *
 * @param multiboot The multiboot structure that may contain ELF files to load
 */
GlobalScheduler::GlobalScheduler(Multiboot& multiboot)
: InterruptHandler(0x20),
  m_shared_memory_registry(resource_type_t::SHARED_MEMORY),
  m_shared_messages_registry(resource_type_t::MESSAGE_ENDPOINT),
  m_next_pid(0),
  m_next_tid(0)
{

	// Set up the global scheduler
	Logger::INFO() << "Setting up global Scheduler\n";
	s_instance = this;

	// Set up the per core scheduler
	for(const auto& core : CPU::cores)
		core -> scheduler = new Scheduler(core->id);

	// Load the elfs
	load_multiboot_elfs(&multiboot);

}

/**
 * @brief Gets the system scheduler
 *
 * @return The system scheduler or nullptr if not found
 */
GlobalScheduler* GlobalScheduler::system_scheduler() {

	return s_instance;
}


/**
 * @brief Handles the interrupt 0x20
 *
 * @param status The current CPU status
 * @return The new CPU status
 */
system::cpu_status_t* GlobalScheduler::handle_interrupt(system::cpu_status_t* status) {

	// Scheduler not active
	if(!s_instance || !s_instance->m_active)
		return status;

	auto s = core_scheduler()->schedule(status);
	ASSERT(s->rip != 0, "Cant run a empty state\n");
	return s;
}

/**
 * @brief Destroys the global scheduler and frees the per core schedulers
 */
GlobalScheduler::~GlobalScheduler(){

	// No longer the global instance
	s_instance = nullptr;

	// Free the per core scheduler
	for(const auto& core : CPU::cores)
		delete core->scheduler;

}

/**
 * @brief Activates each core's scheduler
 */
void GlobalScheduler::activate() {

	s_instance -> m_active = true;
	for(const auto& core : CPU::cores)
		core->scheduler->activate();

}

/**
 * @brief Deactivates each core's scheduler
 */
void GlobalScheduler::deactivate() {

	s_instance -> m_active = false;
	for(const auto& core : CPU::cores)
		core->scheduler->deactivate();

}

/**
 * @brief Moves processes and threads between cores to balance the load so that each core has a similar amount of threads
 * and processes
 *
 * @todo Implement
 */
void GlobalScheduler::balance() {

}

/**
 * @brief Finds the core with the least amount of threads/processes.
 *
 * @param check_threads If true also check for least amount of threads, defaults to false
 * @return The core with the lowest work load
 */
Core* GlobalScheduler::least_busy_core(bool check_threads) {

	// Find the scheduler with the lowest work load
	auto core = CPU::cores[0];
	for(const auto& current : CPU::cores)
		if((check_threads && current->scheduler->thread_amount()  < core->scheduler->thread_amount())
						 ||  current->scheduler->process_amount() < core->scheduler->process_amount())
			core = current;


	return core;

}


/**
 * @brief Loads any valid ELF files from the multiboot structure
 *
 * @param multiboot The multiboot structure
 */
void GlobalScheduler::load_multiboot_elfs(Multiboot* multiboot) {

	for (multiboot_tag* tag = multiboot->start_tag(); tag->type != MULTIBOOT_TAG_TYPE_END; tag = (struct multiboot_tag*) ((multiboot_uint8_t*) tag + ((tag->size + 7) & ~7))) {

		// Tag is not an module
		if (tag->type != MULTIBOOT_TAG_TYPE_MODULE)
			continue;

		// Get the tag
		auto* module = (struct multiboot_tag_module*) tag;
		auto start	= (uintptr_t)PhysicalMemoryManager::to_dm_region(module->mod_start);
		Logger::DEBUG() << "Creating process from multiboot module for " << module->cmdline << " (at 0x" << (uint64_t) module->mod_start << ")\n";

		// Handle the init ramdisk
		if (strcmp(module->cmdline, "initrd") == 0)
			prepare_initrd(module);

		// Try to create the elf from the module
		ELF64 elf(start);
		if (!elf.is_valid())
			continue;

		// Create an array of args for the process
		char* args[1] = {module->cmdline};

		// Create the process
		auto* process = new Process(module->cmdline, args, 1, &elf);
		system_scheduler() -> add_process(process);

		Logger::DEBUG() << "ELF loaded to pid " << process->pid() << "\n";
	}
}

/**
 * @brief Loads the initrd into the init program at 0xFFF000
 *
 * @param module The initrd module
 *
 * @todo dont hardcode pid (and maybe not rely on Init being ordered after module)
 */
void GlobalScheduler::prepare_initrd(multiboot_tag_module* module) {

	// Find the Init process
	Process* init_process = get_process(4);
	ASSERT(init_process != nullptr, "Can't find init process\n");

	// Extract info
	auto start	= (uintptr_t)PhysicalMemoryManager::to_dm_region(module->mod_start);
	uint32_t size = module->mod_end - module->mod_start;

	// Prepare
	MemoryManager::switch_active_memory_manager(init_process->memory_manager);
	init_process->memory_manager->vmm()->allocate(0xFFF000, size + sizeof(uint32_t), WRITE | PRESENT);

	// Copy
	memcpy((void*)0xFFF000, &size, sizeof(uint32_t));
	memcpy((void*)(0xFFF000 + sizeof(uint32_t)), (void*)start, size);
	MemoryManager::switch_active_memory_manager(MemoryManager::s_kernel_memory_manager);
}

/**
 * @brief Print the header for the running processes in the form ({name}:t{tid}c{core})
 */
void GlobalScheduler::print_running_header() {

	// No threads or processes to get
	if(!s_instance || !s_instance->m_active)
		return;

	auto process = current_process();
	auto thread   = current_thread();

	Logger::active_logger()->printf("(%s:t%dc%d) ",process->name.c_str(), thread->tid,CPU::executing_core()->id);
}


/**
 * @brief Adds a process to the least busy core
 *
 * @param process The process to add
 * @param scheduler Optional, specify a specific scheduler to add to
 * @return The pid of the new process
 */
uint64_t GlobalScheduler::add_process(Process* process, Scheduler* scheduler) {

	// No cores?
	if(CPU::cores.empty())
		return 0;

	// Find the core with the least processes
	if (!scheduler)
		scheduler = least_busy_core()->scheduler;
	auto core_id = scheduler->core_id();

	// Save the pid
	auto pid = scheduler->add_process(process);
	m_core_pids.insert(pid,core_id);

	//@TODO better way to do this
	if (process->name == DEBUG_TARGET && DO_DEBUGGING) {
		Logger::DEBUG() << "Starting PROG with GDB\n";
		auto serial = new SerialConsole();
		auto debugger = new GDBServer(serial);
		debugger->attach(process->threads()[0]);
	}

	Logger::DEBUG() << "Adding process " << pid << ": " << process->name << " to core " << core_id <<"\n";
	return pid;
}

/**
 * @brief Adds a thread to the least busy core
 *
 * @param thread The thread to add
 *  @param scheduler Optional, specify a specific scheduler to add to
 * @return The tid of the new thread
 */
uint64_t GlobalScheduler::add_thread(Thread* thread, Scheduler* scheduler) {

	// No cores?
	if(CPU::cores.empty())
		return 0;

	// Find the core with the least threads
	if (!scheduler)
		scheduler = least_busy_core(true)->scheduler;
	auto core_id = scheduler->core_id();

	// Save the tid
	auto tid = scheduler->add_thread(thread);
	m_core_tids.insert(tid,core_id);

	Logger::DEBUG() << "Adding thread " << tid << " to core " << core_id <<"\n";
	return tid;
}

/**
 * @brief Removes a process from the scheduler if the process has no threads, if it does then the threads are stopped but the process is not removed (this will be done automatically when all threads are stopped)
 *
 * @param process The process to remove
 * @return -1 if the process has threads, 0 otherwise
 */
uint64_t GlobalScheduler::remove_process(Process* process) {

	// Get the core running the process
	if(!s_instance)
		return 0;

	auto core = s_instance -> m_core_pids[process->pid()];
	return CPU::cores[core]->scheduler->remove_process(process);
}

/**
 * @brief Removes a process from the scheduler and deletes all threads, begins running the next process
 *
 * @param process The process to remove
 * @return The status of the CPU for the next process to run or nullptr if the process was not found
 */
system::cpu_status_t* GlobalScheduler::force_remove_process(Process* process) {

	// Get the core running the process
	if(!s_instance)
		return nullptr;

	auto core = s_instance -> m_core_pids[process->pid()];
	return CPU::cores[core]->scheduler->force_remove_process(process);

}

/**
 * @brief Gets the process on the currently executing core
 *
 * @return The current process, or nullptr if not found
 */
Process* GlobalScheduler::current_process() {
	if(!s_instance || !s_instance ->m_active)
		return nullptr;

	return core_scheduler()->current_process();
}

/**
 * @brief Gets a process on the currently executing core by its PID
 *
 * @param pid The process ID
 * @return The process or nullptr if not found
 */
Process* GlobalScheduler::get_process(uint64_t pid) {

	// Get the core running the pid
	if(!s_instance)
		return nullptr;

	auto core = s_instance -> m_core_pids[pid];
	return CPU::cores[core]->scheduler->get_process(pid);
}

/**
 * @brief Gets the next usable global PID
 * @return
 */
uint64_t GlobalScheduler::next_pid() {
	return s_instance -> m_next_pid++;
}

/**
 * @brief Gets the thread on the currently executing core
 *
 * @return The currently executing thread
 */
Thread* GlobalScheduler::current_thread() {

	if(!s_instance || !s_instance ->m_active)
		return nullptr;

	return core_scheduler()->current_thread();
}

/**
 * @brief Gets a thread on the currently executing core by its TID
 *
 * @param tid The thread ID
 * @return The thread or nullptr if not found
 */
Thread* GlobalScheduler::get_thread(uint64_t tid) {

	// Get the core running the tid
	if(!s_instance)
		return nullptr;

	auto core = s_instance -> m_core_tids[tid];
	return CPU::cores[core]->scheduler->get_thread(tid);
}

/**
 * @brief Gets the next usable global TID
 * @return
 */
uint64_t GlobalScheduler::next_tid() {
	return s_instance->m_next_tid++;
}

/**
 * @brief Gets the scheduler for the currently executing core
 *
 * @return The scheduler for this core
 */
Scheduler* GlobalScheduler::core_scheduler() {
	return CPU::executing_core()->scheduler;
}

/**
 * @brief Checks if the global scheduler is active
 *
 * @return True if the scheduler is active, false otherwise
 */
bool GlobalScheduler::is_active() {
	if (!s_instance)
		return false;

	return s_instance->m_active;
}

/**
 * @brief Constructs a new Scheduler object and creates the idle process
 */
Scheduler::Scheduler(uint64_t core_id)
: m_next_thread_index(0),
  m_active(false),
  m_ticks(0),
  m_core_id(core_id)
{

	// Name the idle
	string name = "Idle c";
	name += string(m_core_id);

	// Create this idle process
	auto* idle = new Process(name, nullptr, nullptr, 0, true);
	GlobalScheduler::system_scheduler()->add_process(idle, this);

	// Idle must use the correct kernel memory
	idle->memory_manager = MemoryManager::s_kernel_memory_manager;
	idle->threads()[0]->set_tss_pointer(CPU::cores[core_id]->tss.rsp0);
}

Scheduler::~Scheduler() = default;


/**
 * @brief Schedules the next thread to run
 *
 * @param cpu_state The current CPU state
 * @return The next CPU state
 */
cpu_status_t* Scheduler::schedule(cpu_status_t* cpu_state) {

	// Scheduler cant schedule anything
	if (m_threads.empty() || !m_active)
		return cpu_state;

	// Thread that we are dealing with
	Thread* current_thread = m_threads[m_next_thread_index];

	// Ticked
	m_ticks++;
	current_thread->ticks++;

	// Thread hasn't used its time slot yet
	if(current_thread->ticks % TICKS_PER_EVENT)
		return cpu_state;

	// Schedule the next thread
	return schedule_next(cpu_state);
}

/**
 * @brief Schedules the next thread to run
 *
 * @param status The current CPU status of the thread
 * @return The next CPU status
 *
 * @todo Remove by reference where possible
 */
cpu_status_t* Scheduler::schedule_next(cpu_status_t* cpu_state) {

	// Get the current state
	Thread* current_thread = m_threads[m_next_thread_index];
	Process* owner_process = current_process();
	auto old_tid = m_next_thread_index;

	// Save the executing thread state
	current_thread->execution_state = *cpu_state;
	current_thread->save_sse_state();
	if (current_thread->thread_state == ThreadState::RUNNING)
		current_thread->thread_state = ThreadState::READY;

	// Find a free thread to run
	while ((++m_next_thread_index) != old_tid){
		m_next_thread_index %= m_threads.size();

		// Get the current thread
		current_thread = m_threads[m_next_thread_index];
		owner_process = current_process();

		// Handle state changes
		switch (current_thread->thread_state) {

			case ThreadState::SLEEPING:

				// If the wake-up time hasn't occurred yet, run the next thread
				if (current_thread->wakeup_time > TICKS_PER_EVENT){
					current_thread->wakeup_time -= TICKS_PER_EVENT;
					continue;
				}

				break;

			case ThreadState::STOPPED:

				// Find the process that has the thread and remove it
				for (auto thread: owner_process->threads()) {
					if (thread == current_thread) {
						owner_process->remove_thread(thread->tid);
						break;
					}
				}

				// Remove the thread
				m_threads.erase(m_threads.begin() + m_next_thread_index);
				if(owner_process->threads().empty())
					remove_process(owner_process);
				continue;

			case ThreadState::WAITING:
				continue;

			default:
				break;
		}
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

	// Load into the threads memory region
	MemoryManager::switch_active_memory_manager(process->memory_manager);

	if (process->name == "DriverManager.elf")
		asm("nop");

	// Load the tss state for this thread
	CPU::executing_core() -> tss.rsp0 = thread->tss_pointer();
	thread->load_port_bitmap();

	return &thread->execution_state;
}

/**
 * @brief Adds a process to the scheduler
 *
 * @param process The process to add
 * @return The process ID (will be assigned during add)
 */
uint64_t Scheduler::add_process(Process* process) {

	// Get the next process ID
	auto pid = GlobalScheduler::next_pid();
	process->set_pid(pid);

	// Add the process to the list
	m_processes.push_back(process);

	// Add the threads to the list
	for(const auto& thread : process->threads())
		thread->tid = add_thread(thread);

	return pid;
}

/**
 * @brief Adds a thread to the scheduler
 *
 * @param thread The thread to add
 * @return The thread ID
 */
uint64_t Scheduler::add_thread(Thread* thread) {

	// Get the next thread ID
	auto tid = GlobalScheduler::next_tid();
	thread->tid = tid;

	// Add the thread to the list
	m_threads.push_back(thread);

	// Return the thread ID
	return tid;
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
 * @brief Gets the core that this scheduler handles processes for
 *
 * @return The id of the core
 */
uint64_t Scheduler::core_id() const {

	return m_core_id;
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
 * @return -1 if the process has threads, 0 otherwise
 */
uint64_t Scheduler::remove_process(Process* process) {

	// Check if the process has no threads
	if (!process->threads().empty()) {

		// Set the threads to stopped or remove them if forced
		for (auto thread: process->threads())
			thread->thread_state = ThreadState::STOPPED;

		// Need to wait until the threads are stopped before removing the process (this will be called again when all threads are stopped)
		return -1;

	}

	// Remove the process
	for (uint32_t i = 0; i < m_processes.size(); i++) {
		if (m_processes[i] == process) {
			m_processes.erase(m_processes.begin() + i);

			// Delete the process mem
			delete process;
			return 0;
		}
	}

	// Process not found
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

	// Remove all the threads
	for (auto thread: process->threads()) {

		// Remove the thread from the scheduler
		size_t index = m_threads.find(thread) - m_threads.begin();
		m_threads.erase(m_threads.begin() + index);

		// Delete the thread
		process->remove_thread(thread->tid);

	}

	// Process will be dead now so run the next process (don't care about the execution state being outdated as it is being
	// removed regardless)
	return schedule_next(&current_thread()->execution_state);
}

/**
 * @brief Gets the current process
 *
 * @return The current process, or nullptr if not found
 */
Process* Scheduler::current_process() {

	Process* current_process = nullptr;

	// Find the process that has the thread being executed
	for (auto process: m_processes)
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
	for (auto process: m_processes)
		if (process->pid() == pid)
			return process;

	// Not found
	return nullptr;
}

/**
 * @brief Gets how processes are running on this scheduler
 * @return The amount
 */
uint64_t Scheduler::process_amount() {
	return m_processes.size();
}

/**
 * @brief Gets the currently executing thread
 *
 * @return The currently executing thread
 */
Thread* Scheduler::current_thread() {

	return m_threads[m_next_thread_index];
}

/**
 * @brief Gets how many threads are running on this scheduler
 * @return The amount
 */
uint64_t Scheduler::thread_amount(){
	return m_threads.size();
}

/**
 * @brief Deactivates the scheduler
 */
void Scheduler::deactivate() {

	m_active = false;
}

/**
 * @brief Gets a thread by its TID
 *
 * @param tid The thread ID
 * @return The thread or nullptr if not found
 */
Thread* Scheduler::get_thread(uint64_t tid) {

	// Try to find the thread
	for (auto thread: m_threads)
		if (thread->tid == tid)
			return thread;

	return nullptr;
}