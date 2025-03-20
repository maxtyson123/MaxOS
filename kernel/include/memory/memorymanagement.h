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
              static MemoryManager* s_current_memory_manager;
              static MemoryManager* s_kernel_memory_manager;

              static const uint64_t s_higher_half_kernel_offset {  0xFFFFFFFF80000000 };
              static const uint64_t s_higher_half_mem_offset    {  0xFFFF800000000000 };
              static const uint64_t s_higher_half_mem_reserved  {  0x280000000 };
              static const uint64_t s_higher_half_offset        { s_higher_half_mem_offset + s_higher_half_mem_reserved};
              static const uint64_t s_hh_direct_map_offset      { s_higher_half_offset + PhysicalMemoryManager::s_page_size };

              // Each chunk is aligned to 16 bytes
              static const size_t s_chunk_alignment { 0x10 };

              MemoryManager* previous_memory_manager;

              MemoryManager(VirtualMemoryManager* virtual_memory_manager);
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

              // Utility Functions
              int memory_used();
              size_t align(size_t size);
              static void switch_active_memory_manager(MemoryManager* manager);


              // Higher Half Memory Management
              static void* to_higher_region(uintptr_t physical_address);
              static void* to_lower_region(uintptr_t virtual_address);
              static void* to_io_region(uintptr_t physical_address);
              static void* to_dm_region(uintptr_t physical_address);
              static void* from_dm_region(uintptr_t physical_address);
              static bool in_higher_region(uintptr_t virtual_address);
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
