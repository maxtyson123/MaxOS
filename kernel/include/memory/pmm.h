//
// Created by 98max on 1/30/2024.
//

#ifndef MAXOS_MEMORY_PMM_H
#define MAXOS_MEMORY_PMM_H

#include <stdint.h>
#include <stddef.h>
#include <system/multiboot.h>

namespace MaxOS {

  namespace memory {

      class PhysicalMemoryManager{

        private:
          const uint32_t PAGE_SIZE = { 0x200000 };    // 2MB
          const uint8_t ROW_BITS = { 64 };

          uint64_t* m_bit_map;
          uint32_t m_total_entries;
          uint32_t m_bitmap_size;
          uint32_t m_used_frames;

        public:

          PhysicalMemoryManager(unsigned long reserved, multiboot_tag_basic_meminfo* meminfo);
          ~PhysicalMemoryManager();

          void allocate_frame(size_t size);
          void free_frame(void* address);

          void allocate_area(uint64_t start_address, size_t size);
          void free_area(uint64_t start_address, size_t size);
      };
  }

}

#endif // MAXOS_MEMORY_PMM_H
