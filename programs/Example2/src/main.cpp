//
// Created by 98max on 21/03/2025.
//
#include <stdint.h>

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

extern "C" void _start(void)
{


  // Write to the console
  write("MaxOS Test Program v3.1\n");

  // Try get a shared block
  TestSharedMemoryBlock* shared_memory = (TestSharedMemoryBlock*)open_shared_memory("TestBlock");

  write("Shared memory block loaded: \n");
  write_hex((uint64_t)shared_memory);
  write(shared_memory->message);

  // Update the message
  setstring(shared_memory->message, "Hello from process 2\n");

  // For now loop forever
  while(true)
    asm("nop");
}