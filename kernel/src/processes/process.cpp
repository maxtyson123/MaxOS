//
// Created by 98max on 25/02/2025.
//
#include <processes/process.h>
#include <common/logger.h>

using namespace MaxOS;
using namespace MaxOS::system;
using namespace MaxOS::memory;
using namespace MaxOS::processes;
using namespace MaxOS::common;

// TODO: Dont pause interrupts?


/**
 * @brief Constructor for the Thread class
 */
Thread::Thread(void (* _entry_point)(void*), void* args, int arg_amount, Process* parent) {

	// Basic setup
	thread_state = ThreadState::NEW;
	wakeup_time = 0;
	ticks = 0;

	// Create the stack
	m_stack_pointer = (uintptr_t) MemoryManager::malloc(STACK_SIZE);

	// Create the TSS stack
	if (parent->is_kernel) {

		// Use the kernel stack
		m_tss_stack_pointer = CPU::executing_core() -> tss.rsp0;

	} else {
		m_tss_stack_pointer = (uintptr_t) MemoryManager::kmalloc(STACK_SIZE) + STACK_SIZE;
	}

	// Mak sure there is a stack
	ASSERT(m_stack_pointer != 0 && m_tss_stack_pointer != 0, "Failed to allocate stack for thread");

	// Set up the execution state
	execution_state = {};
	execution_state.rip = (uint64_t) _entry_point;
	execution_state.ss = parent->is_kernel ? 0x10 : 0x23;
	execution_state.cs = parent->is_kernel ? 0x8 : 0x1B;
	execution_state.rflags = 0x202;
	execution_state.interrupt_number = 0;
	execution_state.error_code = 0;
	execution_state.rsp = m_stack_pointer;
	execution_state.rbp = 0;

	// Copy the args into userspace
	uint64_t argc = arg_amount;
	void* argv = MemoryManager::malloc(arg_amount * sizeof(void*));
	memcpy(argv, args, arg_amount * sizeof(void*));

	execution_state.rdi = argc;
	execution_state.rsi = (uint64_t) argv;
	//execution_state->rdx = (uint64_t)env_args;

	parent_pid = parent->pid();
}

/**
 * @brief Destructor for the Thread class
 */
Thread::~Thread() = default;

/**
 * @brief Sleeps the thread for a certain amount of time
 *
 * @param milliseconds The amount of time to sleep for
 */
void Thread::sleep(size_t milliseconds) {

	// Update the state
	thread_state = ThreadState::SLEEPING;
	wakeup_time  = milliseconds;
}

/**
 * @brief Saves the SSE, x87 FPU, and MMX states from memory to the thread
 */
void Thread::save_sse_state() {

	// Ensure the state saving is enabled
	if (!CPU::executing_core()->xsave_enabled)
		return;

	// Save the state
	asm volatile("fxsave %0" : "=m" (m_sse_save_region));
}

/**
 * @brief Restores the SSE, x87 FPU, and MMX states from the thread to memory
 */
void Thread::restore_sse_state() {

	// Ensure the state saving is enabled
	if (!CPU::executing_core()->xsave_enabled)
		return;

	// Restore the state
	asm volatile("fxrstor %0" : : "m" (m_sse_save_region));
}

void Thread::save_cpu_state() {
	asm volatile(
			// Store return address before stack is modified
			"movq (%%rsp), %%rax\n"

			// Store RDI before stack is modified
			"pushq %%rdi\n"
			"movq %0, %%rdi\n"

			// Store general purpose
			"movq %%r15, 0x00(%%rdi)\n"
			"movq %%r14, 0x08(%%rdi)\n"
			"movq %%r13, 0x10(%%rdi)\n"
			"movq %%r12, 0x18(%%rdi)\n"
			"movq %%r11, 0x20(%%rdi)\n"
			"movq %%r10, 0x28(%%rdi)\n"
			"movq %%r9,  0x30(%%rdi)\n"
			"movq %%r8,  0x38(%%rdi)\n"
			"movq %%rdi, 0x40(%%rdi)\n"
			"movq %%rsi, 0x48(%%rdi)\n"
			"movq %%rbp, 0x50(%%rdi)\n"
			"movq %%rdx, 0x58(%%rdi)\n"
			"movq %%rcx, 0x60(%%rdi)\n"
			"movq %%rbx, 0x68(%%rdi)\n"
			"movq %%rax, 0x70(%%rdi)\n"

			// Reserved
			"movq $0,    0x78(%%rdi)\n"
			"movq $0,    0x80(%%rdi)\n"

			// RIP from earlier
			"movq %%rax, 0x88(%%rdi)\n"

			// Get cs
			"xorq %%rax, %%rax\n"
			"movw %%cs, %%ax\n"
			"movq %%rax, 0x90(%%rdi)\n"

			// Store flags
			"pushfq\n"
			"popq %%rax\n"
			"movq %%rax, 0x98(%%rdi)\n"

			// Get RSP and SS
			"movq %%rsp, 0xA0(%%rdi)\n"
			"xorq %%rax, %%rax\n"
			"movw %%ss, %%ax\n"
			"movq %%rax, 0xA8(%%rdi)\n"

			// Restore RDI
			"popq %%rdi\n"
			:
			: "r" (&execution_state)
			: "rax", "cc", "memory"
			);
}

/**
 * @brief Base Constructor for the Process
 *
 * @param p_name The name of the process
 * @param is_kernel If the process is a kernel process
 */
Process::Process(const string& p_name, bool is_kernel)
: is_kernel(is_kernel),
  name(p_name)
{
	// If it is a kernel process then don't need a new memory manager
	memory_manager = is_kernel ? MemoryManager::s_kernel_memory_manager : new MemoryManager();
}

/**
 * @brief Constructor for the Process class (from a function)
 *
 * @param p_name The name of the process
 * @param _entry_point The entry point of the process
 * @param args The arguments to pass to the process
 * @param arg_amount The amount of arguments
 * @param is_kernel If the process is a kernel process
 */
Process::Process(const string& p_name, void (* _entry_point)(void*), void* args, int arg_amount, bool is_kernel)
: Process(p_name, is_kernel)
{

	// Create the main thread
	auto* main_thread = new Thread(_entry_point, args, arg_amount, this);

	// Add the thread
	add_thread(main_thread);
}

/**
 * @brief Constructor for the Process class (from an elf, will free the elf memory after loading)
 *
 * @param p_name  The name of the process
 * @param args  The arguments to pass to the process
 * @param arg_amount  The amount of arguments
 * @param elf  The elf file to load the process from
 * @param is_kernel  If the process is a kernel process
 */
Process::Process(const string& p_name, void* args, int arg_amount, Elf64* elf, bool is_kernel)
: Process(p_name, is_kernel)
{

	// Get the entry point
	elf->load();
	auto* entry_point = (void (*)(void*)) elf->header()->entry;

	// Create the main thread
	auto* main_thread = new Thread(entry_point, args, arg_amount, this);
	add_thread(main_thread);
}


/**
 * @brief Destructor for the Process class
 */
Process::~Process() {

	uint64_t pages = PhysicalMemoryManager::s_current_manager->memory_used();

	// Free the threads
	for (auto thread: m_threads)
		delete thread;

	// Free the memory manager (only if it was created)
	if (!is_kernel)
		delete memory_manager;

	// Log the cleanup
	Logger::DEBUG() << "Process " << name.c_str() << " cleaned up, memory before: " << pages << " bytes, after cleanup: " << PhysicalMemoryManager::s_current_manager->memory_used() << " bytes\n";
}

/**
 * @brief Adds a thread to the process
 *
 * @param thread The thread to add
 */
void Process::add_thread(Thread* thread) {

	m_lock.lock();

	// Store the thread
	m_threads.push_back(thread);
	thread->parent_pid = m_pid;

	m_lock.unlock();
}

/**
 * @brief Remove a thread by its tid (NOTE: this will not remove it from the scheduler)
 *
 * @param tid
 */
void Process::remove_thread(uint64_t tid) {

	// Find the thread
	for (uint32_t i = 0; i < m_threads.size(); i++) {

		// Thread is not what is being removed
		if (m_threads[i]->tid != tid)
			continue;

		// Delete the thread
		Thread* thread = m_threads[i];
		delete thread;

		// Remove the thread from the list
		m_threads.erase(m_threads.begin() + i);
		return;
	}
}

/**
 * @brief Sets the pid of the process once added to the queue
 *
 * @param pid
 */
void Process::set_pid(uint64_t pid) {

	// Check if the pid is already set
	if (m_pid != 0)
		return;

	// Set the pid
	m_pid = pid;

	// Assign the pid to the threads
	for (auto thread: m_threads)
		thread->parent_pid = pid;

}

/**
 * @brief Gets the threads of the process
 */
Vector<Thread*> Process::threads() {

	return m_threads;
}

/**
 * @brief Gets the pid of the process
 *
 * @return The pid of the process
 */
uint64_t Process::pid() const {

	return m_pid;
}

/**
 * @brief Gets the total ticks of the threads in the process (not this does not include any past killed threads)
 *
 * @return The total ticks of the threads in the process
 */
uint64_t Process::total_ticks() {

	uint64_t total_ticks = 0;
	for (auto thread: m_threads)
		total_ticks += thread->ticks;

	return total_ticks;
}
