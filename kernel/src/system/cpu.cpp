//
// Created by 98max on 18/01/2024.
//
#include <system/cpu.h>
#include <common/kprint.h>

using namespace MaxOS;
using namespace MaxOS::system;
using namespace MaxOS::drivers;

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
    _kprintf("R15: 0x%x\n", status->r15);
    _kprintf("R14: 0x%x\n", status->r14);
    _kprintf("R13: 0x%x\n", status->r13);
    _kprintf("R12: 0x%x\n", status->r12);
    _kprintf("R11: 0x%x\n", status->r11);
    _kprintf("R10: 0x%x\n", status->r10);
    _kprintf("R9: 0x%x\n", status->r9);
    _kprintf("R8: 0x%x\n", status->r8);
    _kprintf("RDI: 0x%x\n", status->rdi);
    _kprintf("RSI: 0x%x\n", status->rsi);
    _kprintf("RBP: 0x%x\n", status->rbp);
    _kprintf("RDX: 0x%x\n", status->rdx);
    _kprintf("RCX: 0x%x\n", status->rcx);
    _kprintf("RBX: 0x%x\n", status->rbx);
    _kprintf("RAX: 0x%x\n", status->rax);
    _kprintf("INTERRUPT NUMBER: 0x%x\n", status->interrupt_number);
    _kprintf("ERROR CODE: 0x%x\n", status->error_code);
    _kprintf("RIP: 0x%x\n", status->rip);
    _kprintf("CS: 0x%x\n", status->cs);
    _kprintf("RFLAGS: 0x%x\n", status->rflags);
    _kprintf("RSP: 0x%x\n", status->rsp);
    _kprintf("SS: 0x%x\n", status->ss);

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
        _kprintf("(%d);\t at 0x%x\n", current_level, frame->rip);

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

