//
// Created by 98max on 10/20/2022.
//

#ifndef MAXOS_SYSTEM_MEMORYMANAGEMENT_H
#define MAXOS_SYSTEM_MEMORYMANAGEMENT_H

#include <stddef.h>
#include <stdint.h>
#include <system/multiboot.h>
#include <memory/virtual.h>

namespace MaxOS{

    namespace memory{

        /**
         * @struct MemoryChunk
         * @brief Stores information about a memory chunk in the memory manager linked list
         */
        struct MemoryChunk{

            MemoryChunk* next;
            MemoryChunk* prev;

            bool allocated;
            size_t size;

        };

        /**
         * @class MemoryManager
         * @brief Handles memory allocation and deallocation
         */
        class MemoryManager{

          private:
            MemoryChunk* m_first_memory_chunk;
            MemoryChunk* m_last_memory_chunk;

            VirtualMemoryManager* m_virtual_memory_manager;

            MemoryChunk* expand_heap(size_t size);

          public:
              inline static MemoryManager* s_current_memory_manager = nullptr;
              inline static MemoryManager* s_kernel_memory_manager = nullptr;

              // Each chunk is aligned to 16 bytes
              static const size_t s_chunk_alignment =  0x10;

              MemoryManager* previous_memory_manager = nullptr;

              MemoryManager(VirtualMemoryManager* virtual_memory_manager = nullptr);
              ~MemoryManager();

              // Public Memory Management
              static void* malloc(size_t size);
              static void free(void* pointer);

              // Kernel Memory Management
              static void* kmalloc(size_t size);
              static void kfree(void* pointer);

              // Internal Memory Management
              void* handle_malloc(size_t size);
              void handle_free(void* pointer);
              VirtualMemoryManager* vmm();

              // Utility Functions
              int memory_used();
              static size_t align(size_t size);
              static void switch_active_memory_manager(MemoryManager* manager);
        };
    }
}




void* operator new(size_t size) throw();
void* operator new[](size_t size) throw();

//Placement New
void* operator new(size_t size, void* pointer);
void* operator new[](size_t size, void* pointer);

void operator delete(void* pointer);
void operator delete[](void* pointer);

void operator delete(void* pointer, size_t size);
void operator delete[](void* pointer, size_t size);

#endif //MAXOS_SYSTEM_MEMORYMANAGEMENT_H
