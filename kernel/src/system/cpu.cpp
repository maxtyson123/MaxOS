//
// Created by 98max on 18/01/2024.
//
#include <system/cpu.h>
#include <common/kprint.h>

using namespace MaxOS;
using namespace MaxOS::system;
using namespace MaxOS::drivers;

extern uint64_t gdt64[];
extern uint64_t stack[];

CPU* CPU::s_instance = nullptr;


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
    _kprintf("%hR15: 0x%x\n", status->r15);
    _kprintf("%hR14: 0x%x\n", status->r14);
    _kprintf("%hR13: 0x%x\n", status->r13);
    _kprintf("%hR12: 0x%x\n", status->r12);
    _kprintf("%hR11: 0x%x\n", status->r11);
    _kprintf("%hR10: 0x%x\n", status->r10);
    _kprintf("%hR9: 0x%x\n", status->r9);
    _kprintf("%hR8: 0x%x\n", status->r8);
    _kprintf("%hRDI: 0x%x\n", status->rdi);
    _kprintf("%hRSI: 0x%x\n", status->rsi);
    _kprintf("%hRBP: 0x%x\n", status->rbp);
    _kprintf("%hRDX: 0x%x\n", status->rdx);
    _kprintf("%hRCX: 0x%x\n", status->rcx);
    _kprintf("%hRBX: 0x%x\n", status->rbx);
    _kprintf("%hRAX: 0x%x\n", status->rax);
    _kprintf("%hINTERRUPT NUMBER: 0x%x\n", status->interrupt_number);
    _kprintf("%hERROR CODE: 0x%x\n", status->error_code);
    _kprintf("%hRIP: 0x%x\n", status->rip);
    _kprintf("%hCS: 0x%x\n", status->cs);
    _kprintf("%hRFLAGS: 0x%x\n", status->rflags);
    _kprintf("%hRSP: 0x%x\n", status->rsp);
    _kprintf("%hSS: 0x%x\n", status->ss);

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
    stack_frame_t* frame = (stack_frame_t*)__builtin_frame_address(0);
    size_t current_level = 0;

    // Loop through the frames logging
    while (current_level < level && frame != nullptr){

        // Print the frame
        _kprintf("%h(%d);\t at 0x%x\n", current_level, frame->rip);

        // Next frame
        frame = frame -> next;
        current_level++;

    }
}


void CPU::PANIC(char const *message) {

  // Print using the backend
  _kpanicf("-=-=-=-=-=-=-=-=-=-=-=-=-=-=-\n");
  _kpanicf("Kernel Panic: %s\n", message);

  // Stack trace
  _kpanicf("----------------------------\n");
  _kpanicf("Stack Trace:\n");
  stack_trace(10);

  // Register dump
  _kpanicf("----------------------------\n");
  _kpanicf("Register Dump:\n");
  cpu_status_t status;
  get_status(&status);
  print_registers(&status);

  // Halt
  _kpanicf("-=-=-=-=-=-=-=-=-=-=-=-=-=-=-\n");
  halt();

}

/**
 * @brief Initialises the TSS for interrupt handling
 */
void CPU::init_tss() {

  // The reserved have to be 0
  m_tss.reserved0 = 0;
  m_tss.reserved1 = 0;
  m_tss.reserved2 = 0;
  m_tss.reserved3 = 0;
  m_tss.reserved4 = 0;

  // The stacks
  m_tss.rsp0 = (uint64_t)stack + 16384;       // Kernel stack (scheduler will set the threads stack)
  m_tss.rsp1 = 0;
  m_tss.rsp2 = 0;

  // Interrupt stacks can all be 0
  m_tss.ist1 = 0;
  m_tss.ist2 = 0;
  m_tss.ist3 = 0;
  m_tss.ist4 = 0;
  m_tss.ist5 = 0;
  m_tss.ist6 = 0;
  m_tss.ist7 = 0;

  // Ports TODO when setting up userspace drivers come back to this
  m_tss.io_bitmap_offset = 0;

  // Split the base into 4 parts (16 bits, 8 bits, 8 bits, 32 bits)
  uint64_t base = (uint64_t)&m_tss;
  uint16_t base_1 = base & 0xFFFF;
  uint8_t base_2 = (base >> 16) & 0xFF;
  uint8_t base_3 = (base >> 24) & 0xFF;
  uint32_t base_4 = (base >> 32) & 0xFFFFFFFF;

  uint16_t limit_low = sizeof(m_tss);

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
  _kprintf("Loading TSS: 0x0%x 0x0%x at 0x%x\n", tss_descriptor_low, tss_descriptor_high, &m_tss);
  asm volatile("ltr %%ax" : : "a" (0x28));

  //TODO: For smp - load the TSS for each core or find a better way to do this


}

/**
 * @brief Constructor for the CPU class
 */
CPU::CPU() {

  // Set the instance
  s_instance = this;

  // TODO = Multicore support

}

/**
 * @brief Destructor for the CPU class
 */
CPU::~CPU() {

  // Clear the instance
  s_instance = nullptr;

}
