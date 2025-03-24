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

// Create a shared memory block (int 0x80 with syscall 0x02 for create shared memory, result in rax)
void* create_shared_memory(uint64_t size, const char* name)
{
  void* result = nullptr;
  asm volatile("int $0x80" : "=a" (result) : "a" (0x02), "b" (size), "c" (name));
  return result;
}

typedef struct TestSharedMemoryBlock
{
  char message[100];

} TestSharedMemoryBlock;

void setstring(char* str, const char* message)
{
  while(*message != '\0')
    *str++ = *message++;
  *str = '\0';
}

bool strequal(const char* str1, const char* str2)
{
  while(*str1 != '\0' && *str2 != '\0')
  {
    if(*str1++ != *str2++)
      return false;
  }
  return *str1 == *str2;
}

extern "C" void _start(void)
{

  // Write to the console
  write("MaxOS Test Program v3\n");

  // Create a shared memory block
  TestSharedMemoryBlock* shared_memory = (TestSharedMemoryBlock*)create_shared_memory(sizeof(TestSharedMemoryBlock), "TestBlock");
  setstring(shared_memory->message, "Hello from shared memory!\n");

  write("Shared memory block created: \n");
  write_hex((uint64_t)shared_memory);

  while (strequal(shared_memory->message, "Hello from shared memory!\n"))
    asm("nop");

  write("Shared memory block message changed: \n");
  write(shared_memory->message);



  // For now loop forever
  while(true)
    asm("nop");
}