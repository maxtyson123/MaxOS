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

    #define VirtualPointer(addr) (void*)((uint64_t)(addr))

    #define PMLX_GET_INDEX(ADDR, LEVEL) (((uint64_t)ADDR & ((uint64_t)0x1ff << (12 + LEVEL * 9))) >> (12 + LEVEL * 9))

    #define PML4_GET_INDEX(ADDR) PMLX_GET_INDEX(ADDR, 3)
    #define PML3_GET_INDEX(ADDR) PMLX_GET_INDEX(ADDR, 2)
    #define PML2_GET_INDEX(ADDR) PMLX_GET_INDEX(ADDR, 1)

      // Useful for readability
      typedef void virtual_address_t;
      typedef void physical_address_t;

      typedef enum PageFlags {
        None = 0,
        Present = (1 << 0),
        Write = (1 << 1),
        User = (1 << 2),
        Address = (1 << 7),
        Stack = (1 << 8)
      } page_flags_t;

      typedef enum PageBits {
        WriteBit = (1 << 1),
        UserBit = (1 << 3),
      } page_bits_t;

      // Struct for a page table entry
        typedef struct PageTableEntry {
          uint64_t present : 1;
          uint64_t write : 1;
          uint64_t user : 1;
          uint64_t write_through : 1;
          uint64_t cache_disabled : 1;
          uint64_t accessed : 1;
          uint64_t dirty : 1;
          uint64_t huge_page : 1;
          uint64_t global : 1;
          uint64_t available : 3;
          uint64_t physical_address : 40;
        } __attribute__((packed)) pte_t;

      // Struct for a page map level
      typedef struct PageMapLevel {
        pte_t entries[512];
      } __attribute__((packed)) pml_t;

      class PhysicalMemoryManager{

        private:
          static const uint32_t PAGE_SIZE = { 0x200000 };    // 2MB
          static const uint64_t PAGE_TABLE_OFFSET = { 0xFFFF000000000000 }; // The offset for the page table (before the kernel hihg half)
          const uint8_t ROW_BITS = { 64 };

          uint64_t* m_bit_map;
          uint32_t m_total_entries;
          uint32_t m_bitmap_size;
          uint32_t m_used_frames;

          multiboot_mmap_entry* m_mmap;

          uint64_t* m_pml4_root_address;
          pte_t* m_pml4_root;

          uint16_t get_pml4_index(uintptr_t virtual_address);
          uint16_t get_page_directory_index(uintptr_t virtual_address);
          uint16_t get_page_table_index(uintptr_t virtual_address);
          uint16_t get_page_index(uintptr_t virtual_address);
          uint64_t get_table_address(uint16_t pml4_index, uint16_t page_directory_index, uint16_t page_table_index, uint16_t page_index);


          void clean_page_table(uint64_t* table);

          pml_t* get_or_create_table(pml_t* table, size_t index, size_t flags);
          pte_t create_page_table_entry(uintptr_t address, size_t flags);

        public:

          PhysicalMemoryManager(unsigned long reserved, system::Multiboot* multiboot, uint64_t pml4_root[512]);
          ~PhysicalMemoryManager();

          // Frame Management
          void* allocate_frame();
          void free_frame(void* address);

          void* allocate_area(uint64_t start_address, size_t size);
          void free_area(uint64_t start_address, size_t size);

          // Map
          virtual_address_t* map(physical_address_t* physical, virtual_address_t* virtual_address, size_t flags);
          virtual_address_t* map(virtual_address_t* virtual_address, size_t flags);
          void map_area(virtual_address_t* virtual_address_start, size_t length, size_t flags);
          void identity_map(physical_address_t* physical_address, size_t flags);

          void unmap(virtual_address_t* virtual_address);


          // Tools
          static size_t size_to_frames(size_t size);
          static size_t align_to_page(size_t size);
          static bool check_aligned(size_t size);
          bool is_mapped(uintptr_t physical_address, uintptr_t virtual_address);

          static PhysicalMemoryManager* current_manager;
      };
  }

}

#endif // MAXOS_MEMORY_PHYSICAL_H
