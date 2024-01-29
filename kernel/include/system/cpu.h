//
// Created by 98max on 17/01/2024.
//

#ifndef MAXOS_SYSTEM_CPU_H
#define MAXOS_SYSTEM_CPU_H

#include <cpuid.h>
#include <stddef.h>
#include <stdint.h>

namespace MaxOS{

  namespace system{

    typedef struct {
      uint64_t r15;
      uint64_t r14;
      uint64_t r13;
      uint64_t r12;
      uint64_t r11;
      uint64_t r10;
      uint64_t r9;
      uint64_t r8;
      uint64_t rdi;
      uint64_t rsi;
      uint64_t rbp;
      uint64_t rdx;
      uint64_t rcx;
      uint64_t rbx;
      uint64_t rax;

      uint64_t interrupt_number;
      uint64_t error_code;

      uint64_t rip;
      uint64_t cs;
      uint64_t rflags;
      uint64_t rsp;
      uint64_t ss;
    } __attribute__((__packed__)) cpu_status_t ;

    struct StackFrame{
      StackFrame* next;
      uintptr_t rip;
    };

    class CPU {
      public:
        static void halt();
        static void get_status(cpu_status_t* status);
        static void set_status(cpu_status_t* status);
        static uint64_t read_msr(uint32_t msr);
        static void write_msr(uint32_t msr, uint64_t value);
        static void cpuid(uint32_t leaf, uint32_t* eax, uint32_t* ebx, uint32_t* ecx, uint32_t* edx);
        static void stack_trace(size_t);
    };

  }

}



#endif // MAXOS_SYSTEM_CPU_H
