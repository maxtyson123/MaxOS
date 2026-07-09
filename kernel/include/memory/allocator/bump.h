//
// Created by 98max on 8/07/2026.
//

#ifndef MAXOS_ALLOCATOR_BUMP_H
#define MAXOS_ALLOCATOR_BUMP_H

#include <cstddef>
#include <cstdint>
#include <memory/allocator/allocator.h>

namespace MaxOS::memory::allocator {

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

	class BumpAllocator : public MemoryAllocator {

		private:
			MemoryChunk* m_first_memory_chunk;
			MemoryChunk* m_last_memory_chunk;

			void* expand_heap(size_t size) final;
			void* allocate_extra_space(size_t size);

			bool m_setup = false;

		public:
			BumpAllocator();
			virtual ~BumpAllocator();

			void setup_region(uintptr_t address, size_t length) final;

			void* allocate(size_t size) final;
			void  unallocate(void* pointer) final;

			size_t amount_used() final;
			size_t align(size_t size) final;

	};

}

#endif //MAXOS_ALLOCATOR_BUMP_H
