//
// Created by 98max on 18/01/2024.
//
#include <system/cpu.h>
#include <common/logger.h>
#include <processes/scheduler.h>
#include <drivers/console/vesaboot.h>
#include <memory/memorymanagement.h>
#include <drivers/clock/clock.h>

using namespace MaxOS;
using namespace MaxOS::system;
using namespace MaxOS::common;
using namespace MaxOS::drivers;
using namespace MaxOS::drivers::clock;
using namespace MaxOS::hardwarecommunication;
using namespace MaxOS::processes;
using namespace MaxOS::memory;

extern uint64_t stack[];
volatile extern __attribute__((aligned(4096))) unsigned char p4_table[];

extern "C" void  core_start();
extern "C" uint8_t core_boot_info[];

Core::Core(hardwarecommunication::madt_processor_apic_t* madt_item)
: m_madt(madt_item)
{

	id 		    = m_madt->processor_id;
	m_apic_id 	= m_madt->apic_id;

	m_enabled 		= (m_madt->flags & 0x1) != 0;
	m_can_enable 	= (m_madt->flags & 0x2) != 0;

	Logger::DEBUG() << "Found CPU ID: " << id << " with APIC ID: " << m_apic_id << " (enabled = " << (string)m_enabled << ", can be enabled = " <<  (string)m_can_enable  << ")\n";

}

Core::~Core() = default;

void Core::wake_up(CPU* cpu) {

	// Boot core is already setup
	if(m_bsp)
		return;

	Logger::DEBUG() << "Starting core: " << id << "\n";
	m_stack = (uint64_t)MemoryManager::kmalloc(s_stack_size);

	// Core specific boot info
	auto info = (core_boot_info_t*)(core_boot_info);
	info->activated = false;
	info->id = id;
	info->stack = m_stack + s_stack_size;

	// Send init IPI
	cpu->apic.local_apic()->send_init(m_apic_id, true);

	// Send de-assert IPI
	cpu->apic.local_apic()->send_init(m_apic_id, false);
	Clock::active_clock()->delay(10);

	// Send tow SIPIs
	for (int i = 0; i < 2; ++i) {

		// Send the start up IPI
		cpu->apic.local_apic()->send_startup(m_apic_id, 0x8);
		Clock::active_clock()->delay(100);

		// Check if core started
		if(info->activated){
			Logger::DEBUG() << "Core " << id << " started successfully \n";
			return;
		}
	}

	Logger::WARNING() << "Failed to start core: " << id << "\n";
}

void Core::init_tss() {

	// The reserved have to be 0
	tss.reserved0 = 0;
	tss.reserved1 = 0;
	tss.reserved2 = 0;
	tss.reserved3 = 0;
	tss.reserved4 = 0;

	// The stacks
	tss.rsp0 = (uint64_t)m_stack + s_stack_size;       // Kernel stack (scheduler will set the threads stack)
	tss.rsp1 = 0;
	tss.rsp2 = 0;

	// Interrupt stacks can all be 0
	tss.ist1 = 0;
	tss.ist2 = 0;
	tss.ist3 = 0;
	tss.ist4 = 0;
	tss.ist5 = 0;
	tss.ist6 = 0;
	tss.ist7 = 0;

	// Ports TODO when setting up userspace drivers come back to this
	tss.io_bitmap_offset = 0;

	// Split the base into 4 parts (16 bits, 8 bits, 8 bits, 32 bits)
	auto base = (uint64_t) &tss;
	uint16_t base_1 = base & 0xFFFF;
	uint8_t base_2 = (base >> 16) & 0xFF;
	uint8_t base_3 = (base >> 24) & 0xFF;
	uint32_t base_4 = (base >> 32) & 0xFFFFFFFF;

	uint16_t limit_low = (uint16_t)(sizeof(tss) - 1);

	// Flags: 1 - Type = 0x9, Descriptor Privilege Level = 0, Present = 1, 2 - Available = 0, Granularity = 0
	uint8_t flags_1 = 0x89;
	uint8_t flags_2 = 0;

	// Create the TSS descriptors
	uint64_t tss_descriptor_low = (uint64_t) base_3 << 56 | (uint64_t) flags_2 << 48 | (uint64_t) flags_1 << 40 | (uint64_t) base_2 << 32 | (uint64_t) base_1 << 16 | (uint64_t) limit_low;
	uint64_t tss_descriptor_high = base_4;

	// Store in the GDT
	gdt->table[5] = tss_descriptor_low;
	gdt->table[6] = tss_descriptor_high;
	gdt -> load();

	// Load the TSS
	Logger::DEBUG() << "Loading TSS: 0x0" << tss_descriptor_low << " 0x0" << tss_descriptor_high << " at 0x" << (uint64_t) &tss << "\n";
	asm volatile("ltr %%ax" : : "a" (0x28));

}


/**
 * @brief Initialises the SSE instructions
 */
void Core::init_sse() {

	// Get the CR0 register
	uint64_t cr0;
	asm volatile("mov %%cr0, %0" : "=r" (cr0));

	// Get the CR4 register
	uint64_t cr4;
	asm volatile("mov %%cr4, %0" : "=r" (cr4));

	// Check if FPU is supported
	ASSERT(CPU::check_cpu_feature(CPU_FEATURE_EDX::FPU), "FPU not supported - needed for SSE");

	// Clear the emulation flag, task switch flags and enable the monitor coprocessor, native exception bits
	cr0 |= (1 << 1);
	cr0 &= ~(1 << 2);
	cr0 &= ~(1 << 3);
	cr0 |= (1 << 5);
	asm volatile("mov %0, %%cr0" : : "r" (cr0));

	// Enable the FPU
	asm volatile("fninit");

	// Check if SSE is supported
	ASSERT(CPU::check_cpu_feature(CPU_FEATURE_EDX::SSE), "SSE not supported");

	// Enable FSAVE, FSTORE and SSE instructions
	cr4 |= (1 << 9);
	cr4 |= (1 << 10);
	asm volatile("mov %0, %%cr4" : : "r" (cr4));

	// Check if XSAVE is supported
	xsave_enabled = CPU::check_cpu_feature(CPU_FEATURE_ECX::XSAVE) && CPU::check_cpu_feature(CPU_FEATURE_ECX::OSXSAVE);
	Logger::DEBUG() << "XSAVE: " << (xsave_enabled ? "Supported" : "Not Supported") << "\n";
	if (!xsave_enabled) return;

	// Enable the XSAVE and XRESTORE instructions
	cr4 |= (1 << 18);
	asm volatile("mov %0, %%cr4" : : "r" (cr4));

	// Set the SSE and x87 bits
	uint64_t xcr0;
	asm volatile("xgetbv" : "=a" (xcr0) : "c" (0));
	xcr0 |= 0x7;
	asm volatile("xsetbv" : : "c" (0), "a" (xcr0));

	// Check if AVX is supported
	avx_enabled = CPU::check_cpu_feature(CPU_FEATURE_ECX::AVX);
	Logger::DEBUG() << "AVX: " << (avx_enabled ? "Supported" : "Not Supported") << "\n";
	if (!avx_enabled) return;

	// Enable the AVX instructions
	cr4 |= (1 << 14);
	asm volatile("mov %0, %%cr4" : : "r" (cr4));

	Logger::DEBUG() << "SSE Enabled\n";
}

void Core::init() {

	// Load the kernel IDT & GDT
	gdt = new GlobalDescriptorTable();
	InterruptManager::load_current();

	// Setup this core's clock
	local_apic = new LocalAPIC;
	Clock::active_clock()->setup_apic_clock(local_apic);

	// Delegate large initiation
	init_sse();
	init_tss();
}


/**
 * @brief Constructor for the CPU class
 */
CPU::CPU(GlobalDescriptorTable* gdt, Multiboot* multiboot)
: acpi(multiboot),
  apic(&acpi)
{

	Logger::INFO() << "Setting up CPU \n";
	find_cores();

	// Manually set up the BSP
	auto bsp = cores[0];
	bsp -> m_bsp = true;
	bsp -> gdt = gdt;
	bsp -> local_apic = apic.local_apic();
	bsp -> init_tss();
	bsp -> init_sse();

}

CPU::~CPU() = default;

[[noreturn]] void CPU::halt() {

	while (true)
		asm volatile("hlt");
}

void CPU::get_status(cpu_status_t* status) {

	// Get the registers
	asm volatile("mov %%r15, %0" : "=r" (status->r15));
	asm volatile("mov %%r14, %0" : "=r" (status->r14));
	asm volatile("mov %%r13, %0" : "=r" (status->r13));
	asm volatile("mov %%r12, %0" : "=r" (status->r12));
	asm volatile("mov %%r11, %0" : "=r" (status->r11));
	asm volatile("mov %%r10, %0" : "=r" (status->r10));
	asm volatile("mov %%r9, %0"  : "=r" (status->r9));
	asm volatile("mov %%r8, %0"  : "=r" (status->r8));
	asm volatile("mov %%rdi, %0" : "=r" (status->rdi));
	asm volatile("mov %%rsi, %0" : "=r" (status->rsi));
	asm volatile("mov %%rbp, %0" : "=r" (status->rbp));
	asm volatile("mov %%rdx, %0" : "=r" (status->rdx));
	asm volatile("mov %%rcx, %0" : "=r" (status->rcx));
	asm volatile("mov %%rbx, %0" : "=r" (status->rbx));
	asm volatile("mov %%rax, %0" : "=r" (status->rax));
}

void CPU::set_status(cpu_status_t* status) {

	// Set the registers
	asm volatile("mov %0, %%r15" : : "r" (status->r15));
	asm volatile("mov %0, %%r14" : : "r" (status->r14));
	asm volatile("mov %0, %%r13" : : "r" (status->r13));
	asm volatile("mov %0, %%r12" : : "r" (status->r12));
	asm volatile("mov %0, %%r11" : : "r" (status->r11));
	asm volatile("mov %0, %%r10" : : "r" (status->r10));
	asm volatile("mov %0, %%r9"  : : "r" (status->r9));
	asm volatile("mov %0, %%r8"  : : "r" (status->r8));
	asm volatile("mov %0, %%rdi" : : "r" (status->rdi));
	asm volatile("mov %0, %%rsi" : : "r" (status->rsi));
	asm volatile("mov %0, %%rbp" : : "r" (status->rbp));
	asm volatile("mov %0, %%rdx" : : "r" (status->rdx));
	asm volatile("mov %0, %%rcx" : : "r" (status->rcx));
	asm volatile("mov %0, %%rbx" : : "r" (status->rbx));
	asm volatile("mov %0, %%rax" : : "r" (status->rax));

}

void CPU::print_registers(cpu_status_t* status) {

	// Print the registers
	Logger::ERROR() << "R15: \t0x" << status->r15 << "\n";
	Logger::ERROR() << "R14: \t0x" << status->r14 << "\n";
	Logger::ERROR() << "R13: \t0x" << status->r13 << "\n";
	Logger::ERROR() << "R12: \t0x" << status->r12 << "\n";
	Logger::ERROR() << "R11: \t0x" << status->r11 << "\n";
	Logger::ERROR() << "R10: \t0x" << status->r10 << "\n";
	Logger::ERROR() << "R9: \t0x" << status->r9 << "\n";
	Logger::ERROR() << "R8: \t0x" << status->r8 << "\n";
	Logger::ERROR() << "RDI: \t0x" << status->rdi << "\n";
	Logger::ERROR() << "RSI: \t0x" << status->rsi << "\n";
	Logger::ERROR() << "RBP: \t0x" << status->rbp << "\n";
	Logger::ERROR() << "RDX: \t0x" << status->rdx << "\n";
	Logger::ERROR() << "RCX: \t0x" << status->rcx << "\n";
	Logger::ERROR() << "RBX: \t0x" << status->rbx << "\n";
	Logger::ERROR() << "RAX: \t0x" << status->rax << "\n";
	Logger::ERROR() << "INT: \t0x" << status->interrupt_number << "\n";
	Logger::ERROR() << "ERRCD: \t0x" << status->error_code << "\n";
	Logger::ERROR() << "RIP: \t0x" << status->rip << "\n";
	Logger::ERROR() << "CS: \t0x" << status->cs << "\n";
	Logger::ERROR() << "RFlGS: \t0x" << status->rflags << "\n";
	Logger::ERROR() << "RSP: \t0x" << status->rsp << "\n";
	Logger::ERROR() << "SS: \t0x" << status->ss << "\n";

}


uint64_t CPU::read_msr(uint32_t msr) {

	// Read the MSR
	uint32_t low, high;
	asm volatile("rdmsr" : "=a" (low), "=d" (high) : "c" (msr));

	// Return the value
	return (uint64_t) low | ((uint64_t) high << 32);
}

void CPU::write_msr(uint32_t msr, uint64_t value) {

	// Write the MSR
	asm volatile("wrmsr" : : "a" ((uint32_t) value), "d" ((uint32_t) (value >> 32)), "c" (msr));
}

void CPU::cpuid(uint32_t leaf, uint32_t* eax, uint32_t* ebx, uint32_t* ecx, uint32_t* edx) {

	// Call the cpuid instruction
	__get_cpuid(leaf, eax, ebx, ecx, edx);
}

void CPU::stack_trace(size_t level) {

	auto* frame = (stack_frame_t*) __builtin_frame_address(0);

	// Loop through the frames logging
	for (size_t current_level = 0; current_level < level; current_level++) {

		// Print the frame
		Logger::ERROR() << "(" << current_level << "):\t at 0x" << frame->rip << "\n";

		// Next frame
		frame = frame->next;
		if (frame == nullptr)
			break;

	}
}

void CPU::PANIC(char const* message, cpu_status_t* status) {

	// Get the current process
	Process* process = Scheduler::current_process();

	// Ensure ready to panic  - At this point it is not an issue if it is possible can avoid the panic as it is most
	// likely called by a place that cant switch to the avoidable state
	if (is_panicking)
		return;
	prepare_for_panic();

	// Print using the backend
	Logger::ERROR() << "-=-=-=-=-=-=-=-=-=-=-=-=-=-=-\n";
	Logger::ERROR() << "Kernel Panic: " << message << "\n";
	Logger::ERROR() << "On Core: " << panic_core->id << "\n";

	// Info about the running process
	Logger::ERROR() << "Process: " << (process ? process->name.c_str() : "Kernel") << "\n";
	if (process)
		Logger::ERROR() << "After running for " << process->total_ticks() << " ticks (system uptime: " << Scheduler::system_scheduler()->ticks() << " ticks)\n";

	// Stack trace
	Logger::ERROR() << "----------------------------\n";
	Logger::ERROR() << "Stack Trace:\n";
	stack_trace(10);

	// Register dump
	Logger::ERROR() << "----------------------------\n";
	Logger::ERROR() << "Register Dump:\n";

	// Log the regs
	cpu_status_t new_status{};
	if (!status) {
		get_status(&new_status);
		status = &new_status;
	}
	print_registers(status);

	// Print some text to the user
	Logger::ERROR() << "----------------------------\n";
	Logger::ERROR() << "There has been a fatal error in MaxOS and the system has been halted.\n";
	Logger::ERROR() << "Please restart the system.\n";

	// Print the logo
	Logger::ERROR() << "----------------------------\n";
	console::VESABootConsole::print_logo_kernel_panic();

	// Halt
	halt();
}

/**
 * @brief Ensure the CPU must panic and prepare for it if so
 *
 * @param status The status of the CPU (if available)
 * @return A CPU status to avoid having to panic or a nullptr if the CPU must panic
 */
cpu_status_t* CPU::prepare_for_panic(cpu_status_t* status) {


	// If it may have occurred in a process, switch to the avoidable state
	if (Scheduler::system_scheduler() != nullptr && Scheduler::current_process() != nullptr) {

		// Get the current process
		Process* process = Scheduler::current_process();

		// If the faulting address is in lower half just kill the process and move on
		if (status && !memory::PhysicalMemoryManager::in_higher_region(status->rip)) {
			Logger::ERROR() << "CPU Panicked in process " << process->name.c_str() << " at 0x" << status->rip << " - killing process\n";
			return Scheduler::system_scheduler()->force_remove_process(process);
		}

		// Otherwise occurred whilst the kernel was doing something for the process
	}

	// We are panicking
	is_panicking = true;
	panic_core = CPU::executing_core();
	return nullptr;
}

/**
 * @brief Search the madt for cores and store them
 */
void CPU::find_cores() {

	// Now that memory is set up the vector can be used
	cores.reserve(1);

	// Search and setup each core
	int index = 0;
	while (true){

		// Try to find a processor
		MADT_Item* processor_madt = apic.io_apic()->get_madt_item(MADT_TYPES::PROCESSOR_APIC, index);
		if(!processor_madt)
			break;

		// Create a cpu
		auto* processor_apic = (madt_processor_apic_t*) ((uint64_t) processor_madt + sizeof(MADT_Item));
		cores.push_back(new Core(processor_apic));
		index++;
	}

	// TODO: Also check x2
}

/**
 * @brief Wake up all the cores
 */
void CPU::init_cores() {

	Logger::INFO() << "Waking up cores: \n";

	// Make sure core_start is accessible
	ASSERT((void*)&core_start == (void*)0x8000, "Core start not at expected address");
	PhysicalMemoryManager::s_current_manager->identity_map((physical_address_t*)0x8000, Present | Write);

	// Identity map the pml4

	// Set up the boot info
	auto info = (core_boot_info_t*)(core_boot_info);
	info->p4_table = (uint64_t)PhysicalMemoryManager::to_lower_region((uintptr_t)p4_table);

	// Start each core
	for(const auto& core : cores)
		core->wake_up(this);
}


/**
 * @brief Checks if a CPU feature is supported (ECX Register)
 *
 * @param feature The feature to check
 * @return If the feature is supported
 */
bool CPU::check_cpu_feature(CPU_FEATURE_ECX feature) {

	// Get the CPUID
	uint32_t eax, ebx, ecx, edx;
	cpuid(0x1, &eax, &ebx, &ecx, &edx);

	// Check the feature
	return ecx & (uint32_t) feature;
}

/**
 * @brief Checks if a CPU feature is supported (EDX Register)
 *
 * @param feature The feature to check
 * @return True if the feature is supported
 */
bool CPU::check_cpu_feature(CPU_FEATURE_EDX feature) {

	// Get the CPUID
	uint32_t eax, ebx, ecx, edx;
	cpuid(0x1, &eax, &ebx, &ecx, &edx);

	// Check the feature
	return edx & (uint32_t) feature;
}

/**
 * @brief Checks if the No Execute page flag is supported
 * @return True if the NX flag is supported
 */
bool CPU::check_nx() {

	// Get the EFER MSR
	uint64_t efer = read_msr(0xC0000080);

	// Check if the NX flag is supported (bit 11)
	bool supported = efer & (1 << 11);
	Logger::DEBUG() << "NX: " << (supported ? "Supported" : "Not Supported") << "\n";

	// Return if the NX flag is supported
	return supported;
}

Core* CPU::executing_core() {

	// Get the id of this core
	uint32_t eax, ebx, ecx, edx;
	CPU::cpuid(1, &eax, &ebx, &ecx, &edx);
	uint32_t core_id = (ebx >> 24) & 0xFF;

	return cores[core_id];
}
