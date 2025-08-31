//
// Created by 98max on 21/03/2025.
//
#include <stdint.h>
#include <stddef.h>
#include <ipc/messages.h>

using namespace IPC;

// Write using a syscall (int 0x80 with syscall 0x01 for write)
void write(const char* data, uint64_t length = 0)
{
  // don't care abt length for now
  asm volatile("int $0x80" : : "a" (0x01), "b" (data));
}

void write_hex(uint64_t data)
{
  // Convert to hex
  char buffer[20];
  buffer[0] = '0';
  buffer[1] = 'x';
  buffer[18] = '\n';
  buffer[19] = '\0';

  // Convert to hex
  for(int i = 0; i < 16; i++)
  {
  // Get the current nibble
  uint8_t nibble = (data >> (60 - i * 4)) & 0xF;

  // Convert to hex
  buffer[2 + i] = nibble < 10 ? '0' + nibble : 'A' + nibble - 10;
  }

  // Write the hex
  write(buffer);
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

void wait(uint64_t ms)
{
  // syscall 0x010, arg0 = milliseconds
  asm volatile(
      "mov %[ms], %%rdi\n\t"
      "mov $0x0A, %%rax\n\t"
      "int $0x80\n\t"
      :
      : [ms] "r"(ms)
      : "rax", "rdi"
  );
}

extern "C" int _start(int argc, char** argv)
{

  // Print the args
  write("Args:\n");
  for(int i = 0; i < argc; i++)
  {

    // Write the arg
    write(argv[i]);
  }
  write("%h\n");

  // Write to the console
  write("MaxOS Test Program v3.1\n");

  const char* message = "Message from process 2\n";

  // Wait 4seconds
  wait(1000);
  write("Waited 1 seconds\n");

  // Send a message via IPC
  uint64_t endpoint = open_endpoint("TestQueue");
  send_message(endpoint, (void*)message, 24);

  // Close the process
  write("Closing process\n");
  close();

}