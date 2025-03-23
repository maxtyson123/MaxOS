//
// Created by 98max on 21/03/2025.
//
#include <stdint.h>

// Write using a syscall (int 0x80 with syscall 0x01 for write)
void write(const char* data, uint64_t length = 0)
{
  // Dont care abt length for now
  asm volatile("int $0x80" : : "a" (0x01), "b" (data), "c" (length));
}

extern "C" void _start(void)
{

  // Write to the console
  write("The kernel is my bitch - Hi from user space\n");

  // For now loop forever
  while(true)
    asm("nop");
}