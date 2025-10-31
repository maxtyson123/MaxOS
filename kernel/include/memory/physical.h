/**
 * @file physical.h
 * @brief Defines a PhysicalMemoryManager class for managing physical memory allocation and deallocation of pages
 *
 * @date 30th January 2024
 * @author Max Tyson
 */

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

		/**
		 * @enum PageFlags
		 * @brief Flags for page table entries
		 *
		 * @todo: fix. this is stupid. Leave as enum not enum class for bitwise operations
		 */
		typedef enum PageFlags {
			NONE            = 0,                ///< No flags
			PRESENT         = (1 << 0),         ///< The page is present in memory
			WRITE           = (1 << 1),         ///< Memory in this page is writable
			USER            = (1 << 2),         ///< This page is accessible from user mode
			WRITE_THROUGH   = (1 << 3),         ///< Write through caching is enabled
			CACHE_DISABLED  = (1 << 4),         ///< Dont let the CPU cache this page
			ACCESSED        = (1 << 5),         ///< This page has been read from or written to
			DIRTY           = (1 << 6),         ///< This page has been written to
			HUGE_PAGE       = (1 << 7),         ///< This page is not 4Kib (2MB or 1GB)
			GLOBAL          = (1 << 8),         ///< The page can be shared  between processes
			NO_EXECUTE      = (1ULL << 63)      ///< Dont let the CPU execute code on this page
		} page_flags_t;

		/**
		 * @struct PageTableEntry
		 * @brief Struct for a page table entry
		 */
		typedef struct PageTableEntry {
			bool present: 1;                    ///< *copydoc PageFlags::PRESENT
			bool write: 1;                      ///< *copydoc PageFlags::WRITE
			bool user: 1;                       ///< *copydoc PageFlags::USER
			bool write_through: 1;              ///< *copydoc PageFlags::WRITE_THROUGH
			bool cache_disabled: 1;             ///< *copydoc PageFlags::CACHE_DISABLED
			bool accessed: 1;                   ///< *copydoc PageFlags::ACCESSED
			bool dirty: 1;                      ///< *copydoc PageFlags::DIRTY
			bool huge_page: 1;                  ///< *copydoc PageFlags::HUGE_PAGE
			bool global: 1;                     ///< *copydoc PageFlags::GLOBAL
			uint8_t available: 3;               ///< Extra metadata bytes available for OS use
			uint64_t physical_address: 52;      ///< The address the page represents in memory
		} __attribute__((packed)) pte_t;

		/**
		 * @struct PageMapLevel
		 * @brief Struct for a page map level (PML4, PDPT, PD, PT)
		 */
		typedef struct PageMapLevel {
			pte_t entries[512];             ///< The entries in this page map level. If it is a PT then these are page table entries, otherwise they are pointers to the next level. Indexed by the relevant bits in the virtual address.
		} __attribute__((packed)) pml_t;


		constexpr uint64_t PAGE_SIZE = 0x1000;      ///< The size of a page (4KB)
		constexpr uint8_t  ROW_BITS = 64;           ///< The number of bits in the bitmap row

		constexpr uint64_t HIGHER_HALF_KERNEL_OFFSET = 0xFFFFFFFF80000000;                                  ///< Where the kernel is mapped in higher half memory
		constexpr uint64_t HIGHER_HALF_MEM_OFFSET = 0xFFFF800000000000;                                     ///< Where higher half memory starts
		constexpr uint64_t HIGHER_HALF_MEM_RESERVED = 0x280000000;                                          ///< Reserved higher half memory for kernel use (10GB)
		constexpr uint64_t HIGHER_HALF_OFFSET = HIGHER_HALF_MEM_OFFSET + HIGHER_HALF_MEM_RESERVED;          ///< Where higher half memory usable space starts
		constexpr uint64_t HIGHER_HALF_DIRECT_MAP = HIGHER_HALF_OFFSET + PAGE_SIZE;                         ///< Where the map of physical memory to higher half starts

		/**
		 * @class PhysicalMemoryManager
		 * @brief Manages the physical memory of the system such as what pages are allocated/free and mapping of virtual to physical addresses
		 *
		 * @todo Global paging so dont have map in every process
		 */
		class PhysicalMemoryManager {

			private:

				uint64_t* m_bit_map = nullptr;
				uint32_t m_total_entries;
				uint32_t m_bitmap_size;
				uint32_t m_used_frames = 0;
				uint32_t m_setup_frames = 0;
				uint64_t m_memory_size;

				uint64_t m_kernel_start_page;
				uint64_t m_kernel_end;

				system::Multiboot* m_multiboot;
				multiboot_mmap_entry* m_mmap;
				multiboot_tag_mmap* m_mmap_tag;

				uint64_t* m_pml4_root_address;
				pte_t* m_pml4_root;

				bool m_initialized;
				bool m_nx_allowed;

				common::Spinlock m_lock;

				// Table Management
				pml_t* get_or_create_table(pml_t* table, size_t index, size_t flags);
				pml_t* get_and_create_table(pml_t* parent_table, uint64_t table_index, pml_t* table);
				pte_t create_page_table_entry(uintptr_t address, size_t flags) const;

				static uint64_t physical_address_of_entry(pte_t* entry);
				pte_t* get_entry(virtual_address_t* virtual_address, pml_t* pml4_root);
				static pml_t* get_higher_half_table(uint64_t index, uint64_t index2 = 510, uint64_t index3 = 510);

				void initialise_bit_map();

			public:

				PhysicalMemoryManager(system::Multiboot* multiboot);
				~PhysicalMemoryManager();

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
				virtual_address_t* map(physical_address_t* physical, virtual_address_t* virtual_address, size_t flags,
				                       uint64_t* pml4_root);
				void map_area(virtual_address_t* virtual_address_start, size_t length, size_t flags);
				void map_area(physical_address_t* physical_address_start, virtual_address_t* virtual_address_start,
				              size_t length, size_t flags);
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

				inline static PhysicalMemoryManager* s_current_manager = nullptr;   ///< The current physical memory manager in use (todo: private with getter, maybe make mapping static?)
				static void clean_page_table(uint64_t* table);

				void reserve(uint64_t address);
				void reserve(uint64_t address, size_t size, const char* = "Unknown");
				void reserve_kernel_regions(system::Multiboot* multiboot);

				physical_address_t* get_physical_address(virtual_address_t* virtual_address, uint64_t* pml4_root);
				bool is_mapped(uintptr_t physical_address, uintptr_t virtual_address, uint64_t* pml4_root);

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