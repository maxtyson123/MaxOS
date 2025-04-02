//
// Created by 98max on 18/01/2024.
//
#include <system/cpu.h>
#include <common/kprint.h>
#include <processes/scheduler.h>

using namespace MaxOS;
using namespace MaxOS::system;
using namespace MaxOS::drivers;
using namespace MaxOS::processes;

extern uint64_t gdt64[];
extern uint64_t stack[];

void CPU::halt() {
  asm volatile("hlt");
}

void CPU::get_status(cpu_status_t *status) {

    // Get the registers
    asm volatile("mov %%r15, %0" : "=r" (status->r15));
    asm volatile("mov %%r14, %0" : "=r" (status->r14));
    asm volatile("mov %%r13, %0" : "=r" (status->r13));
    asm volatile("mov %%r12, %0" : "=r" (status->r12));
    asm volatile("mov %%r11, %0" : "=r" (status->r11));
    asm volatile("mov %%r10, %0" : "=r" (status->r10));
    asm volatile("mov %%r9, %0" : "=r" (status->r9));
    asm volatile("mov %%r8, %0" : "=r" (status->r8));
    asm volatile("mov %%rdi, %0" : "=r" (status->rdi));
    asm volatile("mov %%rsi, %0" : "=r" (status->rsi));
    asm volatile("mov %%rbp, %0" : "=r" (status->rbp));
    asm volatile("mov %%rdx, %0" : "=r" (status->rdx));
    asm volatile("mov %%rcx, %0" : "=r" (status->rcx));
    asm volatile("mov %%rbx, %0" : "=r" (status->rbx));
    asm volatile("mov %%rax, %0" : "=r" (status->rax));

}
void CPU::set_status(cpu_status_t *status) {

  // Set the registers
  asm volatile("mov %0, %%r15" : : "r" (status->r15));
  asm volatile("mov %0, %%r14" : : "r" (status->r14));
  asm volatile("mov %0, %%r13" : : "r" (status->r13));
  asm volatile("mov %0, %%r12" : : "r" (status->r12));
  asm volatile("mov %0, %%r11" : : "r" (status->r11));
  asm volatile("mov %0, %%r10" : : "r" (status->r10));
  asm volatile("mov %0, %%r9" : : "r" (status->r9));
  asm volatile("mov %0, %%r8" : : "r" (status->r8));
  asm volatile("mov %0, %%rdi" : : "r" (status->rdi));
  asm volatile("mov %0, %%rsi" : : "r" (status->rsi));
  asm volatile("mov %0, %%rbp" : : "r" (status->rbp));
  asm volatile("mov %0, %%rdx" : : "r" (status->rdx));
  asm volatile("mov %0, %%rcx" : : "r" (status->rcx));
  asm volatile("mov %0, %%rbx" : : "r" (status->rbx));
  asm volatile("mov %0, %%rax" : : "r" (status->rax));

}

void CPU::print_registers(cpu_status_t *status) {

    // Print the registers
    _kpanicf("%hR15: \t0x%x\n", status->r15);
    _kpanicf("%hR14: \t0x%x\n", status->r14);
    _kpanicf("%hR13: \t0x%x\n", status->r13);
    _kpanicf("%hR12: \t0x%x\n", status->r12);
    _kpanicf("%hR11: \t0x%x\n", status->r11);
    _kpanicf("%hR10: \t0x%x\n", status->r10);
    _kpanicf("%hR9: \t0x%x\n", status->r9);
    _kpanicf("%hR8: \t0x%x\n", status->r8);
    _kpanicf("%hRDI: \t0x%x\n", status->rdi);
    _kpanicf("%hRSI: \t0x%x\n", status->rsi);
    _kpanicf("%hRBP: \t0x%x\n", status->rbp);
    _kpanicf("%hRDX: \t0x%x\n", status->rdx);
    _kpanicf("%hRCX: \t0x%x\n", status->rcx);
    _kpanicf("%hRBX: \t0x%x\n", status->rbx);
    _kpanicf("%hRAX: \t0x%x\n", status->rax);
    _kpanicf("%hINT: \t0x%x\n", status->interrupt_number);
    _kpanicf("%hERRCD: \t0x%x\n", status->error_code);
    _kpanicf("%hRIP: \t0x%x\n", status->rip);
    _kpanicf("%hCS: \t0x%x\n", status->cs);
    _kpanicf("%hRFlGS: \t0x%x\n", status->rflags);
    _kpanicf("%hRSP: \t0x%x\n", status->rsp);
    _kpanicf("%hSS: \t0x%x\n", status->ss);

}


uint64_t CPU::read_msr(uint32_t msr) {

  // Low and high parts of the MSR
  uint32_t low, high;

  // Read the MSR
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

    // Get the first stack frame
    auto* frame = (stack_frame_t*)__builtin_frame_address(0);
    size_t current_level = 0;

    // Loop through the frames logging
    while (current_level < level && frame != nullptr){

        // Print the frame
        _kpanicf("%h(%d):\t at 0x%x\n", current_level, frame->rip);

        // Next frame
        frame = frame -> next;
        current_level++;

    }
}

#include <memory/memorymanagement.h>

void CPU::PANIC(char const *message, cpu_status_t* status) {

  // Get the current process
  Process* process = Scheduler::get_current_process();

  // Ensure ready to panic  - At this point it is not an issue if it is possible can avoid the panic as it is most likely called by a place that cant switch to the avoidable state
  if(!is_panicking)
    prepare_for_panic();

  // Print using the backend
  _kpanicf("-=-=-=-=-=-=-=-=-=-=-=-=-=-=-\n");
  _kpanicf("Kernel Panic: %s\n", message);

  // Info about the running process
  _kpanicf("Process: %s\n", process ? process->name.c_str() : "Kernel");
  if(process)
    _kpanicf("After running for %d ticks (system uptime: %d ticks)\n", process -> get_total_ticks(), Scheduler::get_system_scheduler()->get_ticks());

  // Stack trace
  _kpanicf("----------------------------\n");
  _kpanicf("Stack Trace:\n");
  stack_trace(10);

  // Register dump
  _kpanicf("----------------------------\n");
  _kpanicf("Register Dump:\n");

  if(!status){
    auto* new_status = new cpu_status_t();                              // Who cares about freeing we're fucked anyway at this point
    get_status(new_status);
    status = new_status;
  }
  print_registers(status);

  // Print some text to the user
  _kpanicf("----------------------------\n");
  _kpanicf("%hThere has been a fatal error in MaxOS and the system has been halted.\n");
  _kpanicf("%hPlease restart the system. \n");


  // Print the logo
  _kpanicf("-=-=-=-=-=-=-=-=-=-=-=-=-=-=-\n");
  _kpanicf("print_logo_kernel_panic();\n\067");

  // Halt
  halt();

}

/**
 * @brief Initialises the TSS for interrupt handling
 */
void CPU::init_tss() {

  // The reserved have to be 0
  tss.reserved0 = 0;
  tss.reserved1 = 0;
  tss.reserved2 = 0;
  tss.reserved3 = 0;
  tss.reserved4 = 0;

  // The stacks
  tss.rsp0 = (uint64_t)stack + 16384;       // Kernel stack (scheduler will set the threads stack)
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
  auto base = (uint64_t)&tss;
  uint16_t base_1 = base & 0xFFFF;
  uint8_t base_2 = (base >> 16) & 0xFF;
  uint8_t base_3 = (base >> 24) & 0xFF;
  uint32_t base_4 = (base >> 32) & 0xFFFFFFFF;

  uint16_t limit_low = sizeof(tss);

  // Flags: 1 - Type = 0x9, Descriptor Privilege Level = 0, Present = 1
  //        2 - Available = 0, Granularity = 0
  uint8_t flags_1 = 0x89;
  uint8_t flags_2 = 0;


  // Create the TSS descriptors
  uint64_t tss_descriptor_low = (uint64_t) base_3 << 56 | (uint64_t) flags_2 << 48 | (uint64_t) flags_1 << 40 | (uint64_t) base_2 << 32 | (uint64_t) base_1 << 16 | (uint64_t) limit_low;
  uint64_t tss_descriptor_high = base_4;

  // Store in the GDT
  gdt64[5] = tss_descriptor_low;
  gdt64[6] = tss_descriptor_high;

  // Load the TSS
  _kprintf("Loading TSS: 0x0%x 0x0%x at 0x%x\n", tss_descriptor_low, tss_descriptor_high, &tss);
  asm volatile("ltr %%ax" : : "a" (0x28));

  //TODO: For smp - load the TSS for each core or find a better way to do this


}

/**
 * @brief Constructor for the CPU class
 */
CPU::CPU() {


  // TODO: Multicore

  // Setup cpu features
  init_tss();
  init_sse();
}

/**
 * @brief Destructor for the CPU class
 */
CPU::~CPU() = default;

/**
 * @brief Ensure the CPU must panic and prepare for it if so
 *
 * @param status The status of the CPU (if available)
 * @return A CPU status to avoid having to panic or a nullptr if the CPU must panic
 */
cpu_status_t* CPU::prepare_for_panic(cpu_status_t* status) {


  // If it may have occurred in a process, switch to the avoidable state
  if(Scheduler::get_system_scheduler() != nullptr && Scheduler::get_current_process() != nullptr){

    // Get the current process
    Process* process = Scheduler::get_current_process();

    // If the faulting address is in lower half just kill the process and move on
    if(status && !memory::PhysicalMemoryManager::in_higher_region(status->rip)){
      _kprintf("CPU Panicked in process %s at 0x%x - killing process\n", process->name.c_str(), status->rip);
      return Scheduler::get_system_scheduler()->force_remove_process(process);
    }
  }

  // We are panicking
  is_panicking = true;

  // Clear the first line
  _kpanicf("%h\n\n\n");

  return nullptr;

}

/**
 * @brief Initialises the SSE instructions
 */
void CPU::init_sse() {

  // Get the CR0 register
  uint64_t cr0;
  asm volatile("mov %%cr0, %0" : "=r" (cr0));

  // Get the CR4 register
  uint64_t cr4;
  asm volatile("mov %%cr4, %0" : "=r" (cr4));


  // Check if FPU is supported
  ASSERT(check_cpu_feature(CPU_FEATURE_EDX::FPU), "FPU not supported - needed for SSE");

  // Clear the emulation flag, task switch flags and enable the monitor coprocessor, native exception bits
  cr0 |=  (1 << 1);
  cr0 &= ~(1 << 2);
  cr0 &= ~(1 << 3);
  cr0 |=  (1 << 5);
  asm volatile("mov %0, %%cr0" : : "r" (cr0));

  // Enable the FPU
  asm volatile("fninit");

  // Check if SSE is supported
  ASSERT(check_cpu_feature(CPU_FEATURE_EDX::SSE), "SSE not supported");

  // Enable FSAVE, FSTORE and SSE instructions
  cr4 |= (1 << 9);
  cr4 |= (1 << 10);
  asm volatile("mov %0, %%cr4" : : "r" (cr4));


  // Check if XSAVE is supported
  s_xsave = check_cpu_feature(CPU_FEATURE_ECX::XSAVE) && check_cpu_feature(CPU_FEATURE_ECX::OSXSAVE);
  _kprintf("XSAVE: %s\n", s_xsave ? "Supported" : "Not Supported");
  if(!s_xsave) return;

  // Enable the XSAVE and XRESTORE instructions
  cr4 |= (1 << 18);
  asm volatile("mov %0, %%cr4" : : "r" (cr4));

  // Set the SSE and x87 bits
  uint64_t xcr0;
  asm volatile("xgetbv" : "=a" (xcr0) : "c" (0));
  xcr0 |= 0x7;
  asm volatile("xsetbv" : : "c" (0), "a" (xcr0));

  // Check if AVX is supported
  s_avx = check_cpu_feature(CPU_FEATURE_ECX::AVX);
  _kprintf("AVX: %s\n", s_avx ? "Supported" : "Not Supported");
  if(!s_avx) return;

  // Enable the AVX instructions
  cr4 |= (1 << 14);
  asm volatile("mov %0, %%cr4" : : "r" (cr4));

  _kprintf("SSE Enabled\n");
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
  return ecx & (uint32_t)feature;

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
    return edx & (uint32_t)feature;

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
  _kprintf("NX: %s\n", supported ? "Supported" : "Not Supported");

  // Return if the NX flag is supported
  return supported;
}
