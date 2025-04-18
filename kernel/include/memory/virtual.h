//
// Created by 98max on 2/11/2024.
//

#ifndef MAXOS_VIRTUAL_H
#define MAXOS_VIRTUAL_H

#include <stdint.h>
#include <stddef.h>
#include <memory/physical.h>
#include <common/string.h>


namespace MaxOS {
  namespace memory {

   // Flags for the virtual memory chunks (not using enum class for bitwise operations)
   typedef enum VirtualFlags {
    // 0 - (1 << 8) are reserved for the page flags
    Reserve = (1 << 9),
    Shared = (1 << 10),

  } virtual_flags_t;

    typedef struct VirtualMemoryChunk{
      uintptr_t start_address;
      size_t size;
      size_t flags;

    } virtual_memory_chunk_t;

    typedef struct FreeChunk{
      uintptr_t start_address;
      struct FreeChunk* next;
      size_t size;
    } free_chunk_t;

    typedef struct VirtualMemoryRegion{
      virtual_memory_chunk_t chunks[(PhysicalMemoryManager::s_page_size / sizeof(virtual_memory_chunk_t) - 1)];
      struct VirtualMemoryRegion* next;

    } __attribute__((packed)) virtual_memory_region_t;

    // NOTE: Have to use a linked list as the VMM is not fully setup and thus cannot use the vector class
    /**
     * @class VirtualMemoryManager
     * @brief Manages the virtual memory of the system and provides functions to allocate and free memory in the virtual address space
     */
    class VirtualMemoryManager{

      private:
        uint64_t * m_pml4_root_address;
        uint64_t * m_pml4_root_physical_address;

        virtual_memory_region_t* m_first_region;
        virtual_memory_region_t* m_current_region;
        size_t m_current_chunk = 0;
        size_t m_next_available_address;

        static const size_t s_chunks_per_page = (PhysicalMemoryManager::s_page_size / sizeof(virtual_memory_chunk_t) - 1);
        static const size_t s_reserved_space = 0x138000000;

        free_chunk_t* m_free_chunks = nullptr;
        void add_free_chunk(uintptr_t start_address, size_t size);
        free_chunk_t* find_and_remove_free_chunk(size_t size);

        void new_region();
        void fill_up_to_address(uintptr_t address, size_t flags, bool mark_used);


      public:
        VirtualMemoryManager();
        ~VirtualMemoryManager();

        void* allocate(size_t size, size_t flags);
        void* allocate(uint64_t address, size_t size, size_t flags);
        void free(void* address);

        void* load_physical_into_address_space(uintptr_t physical_address, size_t size, size_t flags);

        void* load_shared_memory(const string& name);
        void* load_shared_memory(uintptr_t physical_address, size_t size);

        uint64_t* pml4_root_address();
        uint64_t* pml4_root_address_physical();

        size_t memory_used();

    };
  }
}


#endif // MAXOS_VIRTUAL_H
