//
// Created by 98max on 10/20/2022.
//

#ifndef MAXOS_SYSTEM_MEMORYMANAGEMENT_H
#define MAXOS_SYSTEM_MEMORYMANAGEMENT_H

#include <stddef.h>
#include <stdint.h>
#include <system/multiboot.h>

namespace maxOS{

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

          protected:
              MemoryChunk* m_first_memory_chunk;

          public:
              static MemoryManager* s_active_memory_manager;

              MemoryManager(multiboot_tag_basic_meminfo* meminfo);
              ~MemoryManager();

              void* malloc(size_t size);
              void free(void* pointer);
              int memory_used();
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
