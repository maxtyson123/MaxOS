//
// Created by 98max on 1/30/2024.
//

#ifndef MAXOS_MEMORY_PHYSICAL_H
#define MAXOS_MEMORY_PHYSICAL_H

#include <stdint.h>
#include <stddef.h>
#include <system/multiboot.h>

#include <common/spinlock.h>

namespace MaxOS {

  namespace memory {

    #define ENTRIES_TO_ADDRESS(pml4, pdpr, pd, pt)((pml4 << 39) | (pdpr << 30) | (pd << 21) |  (pt << 12))
    #define PMLX_GET_INDEX(ADDR, LEVEL) (((uint64_t)ADDR & ((uint64_t)0x1ff << (12 + LEVEL * 9))) >> (12 + LEVEL * 9))

    #define PML4_GET_INDEX(ADDR) PMLX_GET_INDEX(ADDR, 3)
    #define PML3_GET_INDEX(ADDR) PMLX_GET_INDEX(ADDR, 2)
    #define PML2_GET_INDEX(ADDR) PMLX_GET_INDEX(ADDR, 1)
    #define PML1_GET_INDEX(ADDR) PMLX_GET_INDEX(ADDR, 0)

      // Useful for readability
      typedef void virtual_address_t;
      typedef void physical_address_t;

      // Flags for the page table entries (leave as enum not enum class for bitwise operations)
      typedef enum PageFlags {
        None          = 0,
        Present       = (1    << 0),
        Write         = (1    << 1),
        User          = (1    << 2),
        WriteThrough  = (1    << 3),
        CacheDisabled = (1    << 4),
        Accessed      = (1    << 5),
        Dirty         = (1    << 6),
        HugePage      = (1    << 7),
        Global        = (1    << 8),
        NoExecute     = (1ULL << 63)

      } page_flags_t;


      // Struct for a page table entry
      typedef struct PageTableEntry {
        bool present : 1;
        bool write : 1;
        bool user : 1;
        bool write_through : 1;
        bool cache_disabled : 1;
        bool accessed : 1;
        bool dirty : 1;
        bool huge_page : 1;
        bool global : 1;
        uint8_t available : 3;
        uint64_t physical_address : 52;
      } __attribute__((packed)) pte_t;


      // Struct for a page map level
      typedef struct PageMapLevel {
        pte_t entries[512];
      } __attribute__((packed)) pml_t;

      //TODO: Global paging so dont have map in every process

      /**
       * @class PhysicalMemoryManager
       * @brief Manages the physical memory of the system such as what pages are allocated/free and mapping of virtual to physical addresses
       */
      class PhysicalMemoryManager{

        private:

          uint64_t* m_bit_map = nullptr;
          uint32_t m_total_entries;
          uint32_t m_bitmap_size;
          uint32_t m_used_frames = 0;
          uint64_t m_memory_size;
          uint64_t m_kernel_end;

          uint64_t m_anonymous_memory_physical_address;
          uint64_t m_anonymous_memory_virtual_address;

          system::Multiboot* m_multiboot;
          multiboot_mmap_entry* m_mmap;
          multiboot_tag_mmap*   m_mmap_tag;

          uint64_t* m_pml4_root_address;
          pte_t* m_pml4_root;

          bool m_initialized;
          bool m_nx_allowed;

          common::Spinlock m_lock;

          // Table Management
          pml_t* get_or_create_table(pml_t* table, size_t index, size_t flags);
          pml_t* get_and_create_table(pml_t* parent_table, uint64_t table_index, pml_t* table);
          pte_t create_page_table_entry(uintptr_t address, size_t flags);

          static uint64_t physical_address_of_entry(pte_t* entry);
          pte_t* get_entry(virtual_address_t* virtual_address, pml_t* pml4_root);
          static pml_t* get_higher_half_table(uint64_t index, uint64_t index2 = 510, uint64_t index3 = 510);

          void initialise_bit_map();

        public:

          PhysicalMemoryManager(system::Multiboot* multiboot);
          ~PhysicalMemoryManager();

          static const uint32_t s_page_size =  0x1000;
          static const uint8_t s_row_bits =  64;

          static const uint64_t s_higher_half_kernel_offset =  0xFFFFFFFF80000000;
          static const uint64_t s_higher_half_mem_offset    =  0xFFFF800000000000;
          static const uint64_t s_higher_half_mem_reserved  =  0x280000000;
          static const uint64_t s_higher_half_offset        = s_higher_half_mem_offset + s_higher_half_mem_reserved;
          static const uint64_t s_hh_direct_map_offset      = s_higher_half_offset + s_page_size;

          // Vars
          [[nodiscard]] uint64_t memory_size() const;
          [[nodiscard]] uint64_t memory_used() const;

          // Pml4
          uint64_t* pml4_root_address();
          static void unmap_lower_kernel();

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
          bool is_anonymous_available(size_t size);

          inline static PhysicalMemoryManager* s_current_manager = nullptr;
          static void clean_page_table(uint64_t* table);

          void reserve(uint64_t address);
          void reserve(uint64_t address, size_t size);
          void reserve_kernel_regions(system::Multiboot* multiboot);

          physical_address_t* get_physical_address(virtual_address_t* virtual_address,  uint64_t *pml4_root);
          bool is_mapped(uintptr_t physical_address, uintptr_t virtual_address, uint64_t *pml4_root);

          void change_page_flags(virtual_address_t* virtual_address, size_t flags, uint64_t* pml4_root);

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

#endif // MAXOS_MEMORY_PHYSICAL_H