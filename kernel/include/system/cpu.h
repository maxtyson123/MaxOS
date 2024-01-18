//
// Created by 98max on 17/01/2024.
//

#ifndef MAXOS_SYSTEM_CPU_H
#define MAXOS_SYSTEM_CPU_H

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

  }

}

#endif // MAXOS_SYSTEM_CPU_H
