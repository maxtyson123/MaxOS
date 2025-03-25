//
// Created by 98max on 21/03/2025.
//
#include <stdint.h>
#include <stddef.h>

// Write using a syscall (int 0x80 with syscall 0x01 for write)
void write(const char* data, uint64_t length = 0)
{
  // Dont care abt length for now
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

// Create a shared memory block (int 0x80 with syscall 0x03, taking a string as the name)
void* open_shared_memory(char* name)
{
  void* result = nullptr;

  while (result == nullptr)
    asm volatile("int $0x80" : "=a" (result) : "a" (0x03), "b" (name));
  return result;
}

void setstring(char* str, const char* message)
{
  while(*message != '\0')
    *str++ = *message++;
  *str = '\0';
}

typedef struct TestSharedMemoryBlock
{
  char message[100];

} TestSharedMemoryBlock;

typedef struct IPCMessage{
  void* message_buffer;
  size_t message_size;
  uintptr_t next_message;
} ipc_message_t;

void send_message(const char* endpoint, void* message, size_t size)
{
  asm volatile(
      "mov %[endpoint], %%rdi\n\t"  // arg0: endpoint
      "mov %[message], %%rsi\n\t"   // arg1: message
      "mov %[size], %%rdx\n\t"      // arg2: size
      "mov $0, %%r10\n\t"           // arg3: if not used, set to 0
      "mov $0, %%r8\n\t"            // arg4: if not used, set to 0
      "mov $0, %%r9\n\t"            // arg5: if not used, set to 0
      "mov $0x07, %%rax\n\t"        // syscall number for send_message
      "int $0x80\n\t"
      :
      : [endpoint] "r"(endpoint),
        [message] "r"(message),
        [size] "r"(size)
      : "rax", "rdi", "rsi", "rdx", "r10", "r8", "r9"
  );
}

extern "C" void _start(void)
{


  // Write to the console
  write("MaxOS Test Program v3.1\n");

  const char* message = "Message from process 2\n";
  const char* endpoint = "TestQueue";

  // Send a message via IPC
  send_message(endpoint, (void*)message, 24);

  // For now loop forever
  while(true)
    asm("nop");
}