/**
 * @file memorymanagement.h
 * @brief Defines a MemoryManager class for handling memory allocation and deallocation
 *
 * @date 20th October 2022
 * @author Max Tyson
 */

#ifndef MAXOS_SYSTEM_MEMORYMANAGEMENT_H
#define MAXOS_SYSTEM_MEMORYMANAGEMENT_H

#include <cstddef>
#include <cstdint>
#include <system/multiboot.h>
#include <memory/virtual.h>
#include <assert.h>

namespace MaxOS::memory {

	/**
	 * @struct MemoryChunk
	 * @brief A span of memory in the heap, can be allocated or free. Used as a node in a doubly linked list.
	 */
	struct MemoryChunk {

		MemoryChunk* next;      ///< Pointer to the chunk after this one in the list
		MemoryChunk* prev;      ///< Pointer to the chunk before this one in the list

		bool allocated;         ///< Whether this chunk is in use or can be allocated
		size_t size;            ///< The size of this span of memory (not including the MemoryChunk struct itself)

	};

	constexpr size_t CHUNK_ALIGNMENT = 0x10;    ///< How many bytes the chunks should be a multiple of (round up to this)

	/**
	 * @class MemoryManager
	 * @brief Handles memory allocation and deallocation
	 */
	class MemoryManager {

		private:
			MemoryChunk* m_first_memory_chunk;
			MemoryChunk* m_last_memory_chunk;

			VirtualMemoryManager* m_virtual_memory_manager;

			MemoryChunk* expand_heap(size_t size);

		public:
			inline static MemoryManager* s_current_memory_manager = nullptr;            ///< The memory manager for the current process
			inline static MemoryManager* s_kernel_memory_manager = nullptr;             ///< The memory manager for any kernel processes and all kernel allocations

			explicit MemoryManager(VirtualMemoryManager* virtual_memory_manager = nullptr);
			~MemoryManager();

			// Public Memory Management
			static void* malloc(size_t size);
			static void free(void* pointer);

			// Kernel Memory Management
			static void* kmalloc(size_t size);
			static void kfree(void* pointer);

			// Internal Memory Management
			void* handle_malloc(size_t size);
			void handle_free(void* pointer);
			VirtualMemoryManager* vmm();

			// Utility Functions
			size_t memory_used();
			static size_t align(size_t size);
			static void switch_active_memory_manager(MemoryManager* manager);
	};
}

#endif //MAXOS_SYSTEM_MEMORYMANAGEMENT_H
