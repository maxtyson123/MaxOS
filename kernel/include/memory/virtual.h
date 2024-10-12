//
// Created by 98max on 2/11/2024.
//

#ifndef MAXOS_VIRTUAL_H
#define MAXOS_VIRTUAL_H

#include <stdint.h>
#include <stddef.h>
#include <memory/physical.h>

namespace MaxOS {
  namespace memory {


   typedef enum VirtualFlags {
    // 0 - (1 << 8) are reserved for the page flags
    Reserve = (1 << 9),

  } virtual_flags_t;

    typedef struct VirtualMemoryChunk{
      uintptr_t start_address;
      size_t size;
      size_t flags;

    } virtual_memory_chunk_t;

    typedef struct VirtualMemoryRegion{
      virtual_memory_chunk_t chunks[(PhysicalMemoryManager::PAGE_SIZE / sizeof(virtual_memory_chunk_t) - 1)];
      struct VirtualMemoryRegion* next;

    } __attribute__((packed)) virtual_memory_region_t;

    class VirtualMemoryManager{

      private:
        uint64_t * m_pml4_root_address;
        uint64_t * m_pml4_root_physical_address;
        PhysicalMemoryManager* m_physical_memory_manager;
        bool m_is_kernel;

        virtual_memory_region_t* m_first_region;
        virtual_memory_region_t* m_current_region;
        size_t m_current_chunk;
        size_t m_next_available_address;

        static const size_t s_chunks_per_page = (PhysicalMemoryManager::PAGE_SIZE / sizeof(virtual_memory_chunk_t) - 1);
        static const size_t s_reserved_space = 0x138000000;

        void new_region();

      public:
        VirtualMemoryManager(bool is_kernel);
        ~VirtualMemoryManager();

        void* allocate(size_t size, size_t flags);
        void* allocate(uint64_t address, size_t size, size_t flags);
        void free(void* address);

        size_t memory_used();

    };
  }
}


#endif // MAXOS_VIRTUAL_H
