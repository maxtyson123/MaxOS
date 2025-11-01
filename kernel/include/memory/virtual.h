/**
 * @file virtual.h
 * @brief Defines a VirtualMemoryManager class for managing virtual memory allocation and mapping to physical memory
 *
 * @date 11th February 2024
 * @author Max Tyson
 */

#ifndef MAXOS_VIRTUAL_H
#define MAXOS_VIRTUAL_H

#include <stdint.h>
#include <stddef.h>
#include <memory/physical.h>
#include <common/string.h>


namespace MaxOS {
	namespace memory {

		/**
		 * @enum VirtualFlags
		 * @brief Flags for chunks used in virtual memory allocation
		 *
		 * @typedef virtual_flags_t
		 * @brief Alias for VirtualFlags enum
		 *
		 * @see PageFlags
		 * @note 0 - (1 << 8) are reserved for the page flags
		 */
		typedef enum VirtualFlags {

			RESERVE = (1 << 9),         ///< Reserve the memory but do not map any physical memory to it
			SHARED = (1 << 10),         ///< The memory is shared between multiple processes

		} virtual_flags_t;

		/**
		 * @struct VirtualMemoryChunk
		 * @brief A container for a region of virtual memory that has been allocated by the Virtual Memory Manager
		 *
		 * @typedef virtual_memory_chunk_t
		 * @brief Alias for VirtualMemoryChunk struct
		 *
		 * @see VirtualFlags
		 */
		typedef struct VirtualMemoryChunk {

			uintptr_t start_address;    ///< The first virtual address that this chunk covers
			size_t size;                ///< The size of the chunk's memory region in bytes
			size_t flags;               ///< The flags for the memory (see VirtualFlags)

		} virtual_memory_chunk_t;

		/**
		 * @struct FreeChunk
		 * @brief A chunk of memory that has been freed and is available for allocation. A node in a linked list.
		 *
		 * @typedef free_chunk_t
		 * @brief Alias for FreeChunk struct
		 */
		typedef struct FreeChunk {

			uintptr_t start_address;    ///< *copydoc VirtualMemoryChunk::start_address
			size_t size;                ///< *copydoc VirtualMemoryChunk::size
			struct FreeChunk* next;     ///< Pointer to the next free chunk in the list (not sequential in memory)

		} free_chunk_t;

		static const size_t CHUNKS_PER_PAGE = PAGE_SIZE / sizeof(virtual_memory_chunk_t) -
		                                      1;    ///< Number of chunks per virtual memory region (limited to fit in one page)
		static const size_t VMM_RESERVED = 0x138000000;                                         ///< The starting address for the VMM to start allocating from when in higher half (otherwise can cause conflicts with kernel space)

		/**
		 * @struct VirtualMemoryRegion
		 * @brief A region of virtual memory containing multiple chunks (should fit in one page).
		 *
		 * @typedef virtual_memory_region_t
		 * @brief Alias for VirtualMemoryRegion struct
		 *
		 * @note This stores the metadata for the chunks, not the actual memory they represent. (ie chunks are not slices of this region)
		 */
		typedef struct PACKED VirtualMemoryRegion {

			virtual_memory_chunk_t chunks[CHUNKS_PER_PAGE];     ///< The metadata for the chunks in this region
			struct VirtualMemoryRegion* next;                   ///< Pointer to the next region in the list

		} virtual_memory_region_t;

		/**
		 * @class VirtualMemoryManager
		 * @brief Manages the virtual memory of the system and provides functions to allocate and free memory in the virtual address space
		 *
		 * @note Have to use a linked list as vector class depends on dynamic memory allocation (which depends on this)
		 */
		class VirtualMemoryManager {

			private:
				uint64_t* m_pml4_root_address;
				uint64_t* m_pml4_root_physical_address;

				virtual_memory_region_t* m_first_region;
				virtual_memory_region_t* m_current_region;
				size_t m_current_chunk = 0;
				size_t m_next_available_address;

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

				void* load_shared_memory(const string &name);
				void* load_shared_memory(uintptr_t physical_address, size_t size);

				uint64_t* pml4_root_address();
				uint64_t* pml4_root_address_physical();

				size_t memory_used();

		};
	}
}


#endif // MAXOS_VIRTUAL_H
