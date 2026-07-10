//
// Created by 98max on 8/07/2026.
//

#ifndef MAXOS_ALLOCATOR_BUMP_H
#define MAXOS_ALLOCATOR_BUMP_H

#include <cstddef>
#include <cstdint>

#include <libcommon/assert.h>

#include <memory/allocator/allocator.h>

namespace MaxOS::memory::allocator {


	constexpr uint8_t EXPECTED_CANARY = 0xC0DE;						/// Value that denotes a valid chunk canary value
	constexpr size_t CHUNK_ALIGNMENT = 0x10;						///< How many bytes the chunks should be a multiple of (round up to this)

	/**
	 * @struct ChunkHeader
	 * @brief A span of memory in the heap, can be allocated or free. Used as a node in a doubly linked list.
	 */
	typedef struct alignas(CHUNK_ALIGNMENT) ChunkHeader {

		uint8_t canary;			///< Trip value to detect buffer overflows

		ChunkHeader* next;      ///< Pointer to the chunk after this one in the list
		ChunkHeader* prev;      ///< Pointer to the chunk before this one in the list

		bool allocated;         ///< Whether this chunk is in use or can be allocated
		size_t size;            ///< The size of this span of memory (not including the MemoryChunk struct itself)

		bool is_valid();
		ChunkHeader* prev_chunk();
		ChunkHeader* next_chunk();

	} chunk_header_t;

	constexpr size_t CHUNK_HEADER_SIZE = sizeof(chunk_header_t);	///< How much space the chunk meta data takes up

	class BumpAllocator : public MemoryAllocator {

		private:
			chunk_header_t* m_first_memory_chunk = nullptr;
			chunk_header_t* m_last_memory_chunk = nullptr;

			void* expand_heap(size_t size) final;
			virtual void* allocate_extra_space(size_t requested_size, size_t& actual_size);

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
