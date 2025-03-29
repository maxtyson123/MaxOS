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

typedef struct IPCMessage{
  void* message_buffer;
  size_t message_size;
  uintptr_t next_message;
} ipc_message_t;

typedef struct IPCMessageQueue{
  ipc_message_t* messages;
} ipc_message_queue_t;

void* make_message_queue(char* name)
{
  void* result = nullptr;
  asm volatile("int $0x80" : "=a" (result) : "a" (0x06), "b" (name));
  return result;
}

void  yield()
{
    asm volatile("int $0x80" : : "a" (0x09));
}

extern "C" void _start(void)
{

  // Write to the console
  write("MaxOS Test Program v3\n");

  // Create a message endpoint
  ipc_message_queue_t* message_queue = (ipc_message_queue_t *)make_message_queue("TestQueue");
  write("Message queue created: \n");
  write_hex((uint64_t)message_queue);

  // Process events for ever:
  write("Waiting for messages\n");
  while(true)
    if(message_queue->messages == nullptr)
      yield();
    else{

      //TODO: Should:
      //      Copy message into a buffer
      //      Delete orginal message

      // Store the message
      ipc_message_t* message = message_queue->messages;
      write("Message received: \n");
      write_hex((uint64_t)message);
      write((char*)message->message_buffer);

      // Move to the next message
      message_queue->messages = (ipc_message_t*)message->next_message;

    }
}