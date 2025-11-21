//
// Created by 98max on 21/03/2025.
//
#include <stdint.h>
#include <stddef.h>
#include <ipc/messages.h>
#include <filesystem/directory.h>
#include <syscalls.h>

using namespace syscore;
using namespace syscore::ipc;
using namespace syscore::filesystem;

// Write using a syscall (int 0x80 with syscall 0x01 for write)
void write(const char* data)
{
  // don't care abt length for now
  asm volatile("int $0x80" : : "a" (0x01), "b" (data));
}

void close()
{
  // syscall 0, arg0 = pid (0 for current process), arg1 = exit code
  asm volatile(
      "mov $0, %%rdi\n\t"
      "mov $0, %%rsi\n\t"
      "mov $0x00, %%rax\n\t"
      "int $0x80\n\t"
      :
      :
      : "rax", "rdi", "rsi"
        );
}

extern "C" void _start(void)
{
	// Write to the console
	write("MaxOS Test Program v3\n");

	// Wait 0.5 seconds

	// Get lock

	// Write

	close();

}