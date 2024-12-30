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


    #define PMLX_GET_INDEX(ADDR, LEVEL) (((uint64_t)ADDR & ((uint64_t)0x1ff << (12 + LEVEL * 9))) >> (12 + LEVEL * 9))

    #define PML4_GET_INDEX(ADDR) PMLX_GET_INDEX(ADDR, 3)
    #define PML3_GET_INDEX(ADDR) PMLX_GET_INDEX(ADDR, 2)
    #define PML2_GET_INDEX(ADDR) PMLX_GET_INDEX(ADDR, 1)
    #define PML1_GET_INDEX(ADDR) PMLX_GET_INDEX(ADDR, 0)

      // Useful for readability
      typedef void virtual_address_t;
      typedef void physical_address_t;

      typedef enum PageFlags {
        None          = 0,
        Present       = (1 << 0),
        Write         = (1 << 1),
        User          = (1 << 2),
        WriteThrough  = (1 << 3),
        CacheDisabled = (1 << 4),
        Accessed      = (1 << 5),
        Dirty         = (1 << 6),
        HugePage      = (1 << 7),
        Global        = (1 << 8)

      } page_flags_t;


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
          uint64_t physical_address : 52;
        } __attribute__((packed)) pte_t;

      // Struct for a page map level
      typedef struct PageMapLevel {
        pte_t entries[512];
      } __attribute__((packed)) pml_t;

      class PhysicalMemoryManager{

        private:
          const uint8_t ROW_BITS = { 64 };

          uint64_t* m_bit_map;
          uint32_t m_total_entries;
          uint32_t m_bitmap_size;
          uint32_t m_used_frames;
          uint64_t m_memory_size;

          uint64_t m_anonymous_memory_physical_address;
          uint64_t m_anonymous_memory_virtual_address;

          multiboot_mmap_entry* m_mmap;
          multiboot_tag_mmap*   m_mmap_tag;

          uint64_t* m_pml4_root_address;
          pte_t* m_pml4_root;

          bool m_initialized;

          // Table Management
          void create_table(pml_t* table, pml_t* next_table, size_t index);
          pte_t create_page_table_entry(uintptr_t address, size_t flags);
          bool table_has_entry(pml_t* table, size_t index);
          uint64_t* get_or_create_table(uint64_t* table, size_t index, size_t flags);
          uint64_t* get_table_if_exists(uint64_t* table, size_t index);


          uint64_t* get_bitmap_address();

        public:

          PhysicalMemoryManager(unsigned long reserved, system::Multiboot* multiboot, uint64_t pml4_root[512]);
          ~PhysicalMemoryManager();


          // Vars
          static const uint32_t s_page_size = { 0x1000 };    // 4096 bytes
          uint64_t get_memory_size();
          uint64_t get_memory_used();

          // Pml4
          uint64_t* get_pml4_root_address();

          // Frame Management
          void* allocate_frame();
          void free_frame(void* address);

          void* allocate_area(uint64_t start_address, size_t size);
          void free_area(uint64_t start_address, size_t size);

          // Map
          virtual_address_t* map(virtual_address_t* virtual_address, size_t flags);
          virtual_address_t* map(physical_address_t* physical, virtual_address_t* virtual_address, size_t flags);
          virtual_address_t* map(physical_address_t* physical, virtual_address_t* virtual_address, size_t flags, uint64_t* pml4_root);
          void map_area(virtual_address_t* virtual_address_start, size_t length, size_t flags);
          void map_area(physical_address_t* physical_address_start, virtual_address_t* virtual_address_start, size_t length, size_t flags);
          void identity_map(physical_address_t* physical_address, size_t flags);

          void unmap(virtual_address_t* virtual_address);
          void unmap(virtual_address_t* virtual_address, uint64_t* pml4_root);
          void unmap_area(virtual_address_t* virtual_address_start, size_t length);

          // Tools
          static size_t size_to_frames(size_t size);
          static size_t align_to_page(size_t size);
          static size_t align_direct_to_page(size_t size);
          static size_t align_up_to_page(size_t size, size_t s_page_size);
          static bool check_aligned(size_t size);
          bool is_mapped(uintptr_t physical_address, uintptr_t virtual_address);
          bool is_anonymous_available(size_t size);
          bool is_multiboot_reserved(uint64_t address);

          static PhysicalMemoryManager* s_current_manager;
          void clean_page_table(uint64_t* table);
          void reserve(uint64_t address);
      };
  }

}

#endif // MAXOS_MEMORY_PHYSICAL_H
