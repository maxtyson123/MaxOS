//
// Created by 98max on 8/07/2026.
//

#include <memory/allocator/bump.h>

using namespace MaxOS;
using namespace MaxOS::memory;
using namespace MaxOS::memory::allocator;

BumpAllocator::BumpAllocator() = default;
BumpAllocator::~BumpAllocator() = default;

/**
 * @brief Expands the memory region by a given size
 *
 * @param size The size to expand the region by
 * @return The new space of memory
 */
void* BumpAllocator::allocate_extra_space(size_t size) {
	return nullptr;
}

/**
 * @brief Expands the heap by a given size
 *
 * @param size The size to expand the heap by
 * @return The new chunk of memory
 */
void* BumpAllocator::expand_heap(size_t size) {

	// Create a new chunk of memory
	auto* chunk = (MemoryChunk*)allocate_extra_space(size);
	if(chunk == nullptr)
		return nullptr;

	// Set the chunk's properties
	chunk->allocated = false;
	chunk->size = size;
	chunk->next = nullptr;

	// Insert the chunk into the linked list
	m_last_memory_chunk->next = chunk;
	chunk->prev = m_last_memory_chunk;
	m_last_memory_chunk = chunk;

	// If it is possible to merge the new chunk with the previous chunk then do so (note: this happens if the
	// previous chunk is free but cant contain the size required)
	if(!chunk->prev->allocated)
		unallocate((void*) ((size_t) chunk + sizeof(MemoryChunk)));

	return chunk;
}

void BumpAllocator::setup_region(uintptr_t address, size_t length) {

	// Create a free chunk that covers the entire region
	this->m_first_memory_chunk = (MemoryChunk*)address;
	m_first_memory_chunk->allocated = false;
	m_first_memory_chunk->prev = nullptr;
	m_first_memory_chunk->next = nullptr;
	m_first_memory_chunk->size = length;

	m_last_memory_chunk = m_first_memory_chunk;
	m_setup = true;

}


/**
 * @brief Allocates a block of memory
 *
 * @param size The size of the block to allocate
 * @return A pointer to the block, or nullptr if no block is available
 */
void* BumpAllocator::allocate(size_t size) {

	MemoryChunk* result = nullptr;

	// Nothing to allocate
	if(size == 0)
		return nullptr;

	// Add room to store the chunk information
	size = align(size + sizeof(MemoryChunk));

	// Find the next free chunk that is big enough
	for(MemoryChunk* chunk = m_first_memory_chunk; chunk != nullptr && result == nullptr; chunk = chunk->next) {
		if(chunk->size > size && !chunk->allocated)
			result = chunk;
	}

	// If there is no free chunk then make more room
	if(result == nullptr)
		result = (MemoryChunk*)expand_heap(size);

	// No space to expand heap
	if (result == nullptr)
		return nullptr;

	// If there is not left over space to store extra chunks there is no need to split the chunk
	if(result->size < size + sizeof(MemoryChunk) + 1) {
		result->allocated = true;
		void* p = (void*) (((size_t) result) + sizeof(MemoryChunk));
		return p;
	}

	// Split the chunk into: what was requested + free overflow space for future allocates
	//  - This prevents waste in the event that a big free chunk was found but the requested size would only use a portion of that
	auto* extra = (MemoryChunk*) ((size_t) result + sizeof(MemoryChunk) + size);
	extra->allocated = false;
	extra->size = result->size - size - sizeof(MemoryChunk);
	extra->prev = result;

	// Add to the linked list
	extra->next = result->next;
	if(extra->next != nullptr)
		extra->next->prev = extra;

	// Requested chunk is now allocated exactly to the size requested and points to the free (split) block of memory that
	// it did not use
	result->size = size;
	result->allocated = true;
	result->next = extra;

	// Update the last memory chunk if necessary
	if(result == m_last_memory_chunk)
		m_last_memory_chunk = extra;

	return (void*) (((size_t) result) + sizeof(MemoryChunk));
}


/**
 * @brief Frees a block of memory
 *
 * @param pointer A pointer to the block
 */
void BumpAllocator::unallocate(void* pointer) {

	// Cant free unallocated memory
	if(pointer == nullptr)
		return;

	// Check bounds
	if((uint64_t) pointer < (uint64_t) m_first_memory_chunk || (uint64_t) pointer > (uint64_t) m_last_memory_chunk)
		return;

	// Get the chunk information from the pointer
	auto* chunk = (MemoryChunk*) ((size_t) pointer - sizeof(MemoryChunk));
	chunk->allocated = false;

	// If there is a free chunk before this chunk then merge them
	if(chunk->prev != nullptr && !chunk->prev->allocated) {

		// Grow the chunk behind this one so that it now contains the freed one
		chunk->prev->size += chunk->size + sizeof(MemoryChunk);
		chunk->prev->next = chunk->next;

		if (chunk->prev->next == (void*)0x3000)
			asm("nop");

		// The chunk in front of the freed one now needs to point to the merged chunk
		if(chunk->next != nullptr)
			chunk->next->prev = chunk->prev;


		// Freed chunk doesn't exist anymore so now working with the merged chunk
		chunk = chunk->prev;

	}

	// If there is a free chunk after this chunk then merge them
	if(chunk->next != nullptr && !chunk->next->allocated) {

		// Grow this chunk so that it now contains the free chunk in front of the old (now freed) one
		chunk->size += chunk->next->size + sizeof(MemoryChunk);

		// Now that this chunk contains the next one, it has to point to the one in front of what has just been merged
		// and that has to point to this
		chunk->next = chunk->next->next;
		if(chunk->next != nullptr)
			chunk->next->prev = chunk;

	}
}

/**
 * @brief Returns the amount of memory used
 *
 * @return The amount of memory used
 */
size_t BumpAllocator::amount_used() {

	size_t result = 0;

	// Loop through all the chunks and add up the size of the allocated chunks
	for(MemoryChunk* chunk = m_first_memory_chunk; chunk != nullptr; chunk = chunk->next)
		if(chunk->allocated)
			result += chunk->size;

	return result;
}

/**
 * @brief Aligns the size to the chunk alignment
 *
 * @param size The size to align
 * @return The aligned size
 */
size_t BumpAllocator::align(size_t size) {

	return (size / CHUNK_ALIGNMENT + 1) * CHUNK_ALIGNMENT;
}