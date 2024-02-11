//
// Created by 98max on 1/30/2024.
//

#ifndef MAXOS_MEMORY_PHYSICAL_H
#define MAXOS_MEMORY_PHYSICAL_H

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

          multiboot_mmap_entry* m_mmap;

        public:

          PhysicalMemoryManager(unsigned long reserved, system::Multiboot* multiboot);
          ~PhysicalMemoryManager();

          void* allocate_frame();
          void free_frame(void* address);

          void* allocate_area(uint64_t start_address, size_t size);
          void free_area(uint64_t start_address, size_t size);

          // Tools
          size_t size_to_frames(size_t size) const;
          size_t align_to_page(size_t size) const;
          bool check_aligned(size_t size) const;
      };
  }

}

#endif // MAXOS_MEMORY_PHYSICAL_H
