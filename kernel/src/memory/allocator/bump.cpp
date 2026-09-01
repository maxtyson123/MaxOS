//
// Created by 98max on 8/07/2026.
//

#include <memory/allocator/bump.h>

using namespace MaxOS;
using namespace MaxOS::memory;
using namespace MaxOS::memory::allocator;

BumpAllocator::BumpAllocator() {
	static_assert(CHUNK_HEADER_SIZE == sizeof(chunk_header_t));
};
BumpAllocator::~BumpAllocator() = default;

/**
 * @brief Expands the memory region by a given size
 *
 * @param requested_size The size to expand the region by
 * @param actual_size The size that the region was expaned to
 * @return The new space of memory
 */
void* BumpAllocator::allocate_extra_space(size_t requested_size, size_t& actual_size) {
	return nullptr;
}

/**
 * Validate that a given chunk is as expected
 *
 * @return True if the chunk is valid, false if any check fails
 */
bool ChunkHeader::is_valid() {
	return canary == EXPECTED_CANARY;
}

/**
 * Safely gets the prev chunk. Will crash the program (not the kernel) if invalid chunk
 *
 * @return The previous chunk node in the linked list or nullptr if this is the start node
 *
 * @todo PANIC here to make easier to debug, change to crash the prog isntead
 */
chunk_header_t* ChunkHeader::prev_chunk() {

	// Make sure the chunk is not poiting to garbage
	if (!is_valid())
		ASSERT(false, "Invalid memory chunk detected (possible buffer overflow)\n");

	return prev;
}


/**
 * Safely gets the next chunk. Will crash the program (not the kernel) if invalid chunk
 *
 * @return The next chunk node in the linked list or nullptr if this is the last node
 *
 * @todo PANIC here to make easier to debug, change to crash the prog isntead
 */
chunk_header_t* ChunkHeader::next_chunk() {

	// Make sure the chunk is not poiting to garbage
	if (!is_valid())
		ASSERT(false, "Invalid memory chunk detected (possible buffer overflow)\n");

	return next;
}

/**
 * @brief Expands the heap by a given size
 *
 * @param size The size to expand the heap by
 * @return The new chunk of memory
 */
void* BumpAllocator::expand_heap(size_t size) {

	// Create a new chunk of memory
	size_t actual_size;
	auto* chunk = (chunk_header_t*)allocate_extra_space(CHUNK_HEADER_SIZE + size, actual_size);
	if(chunk == nullptr)
		return nullptr;

	// Set the chunk's properties
	chunk->allocated = false;
	chunk->size = actual_size - CHUNK_HEADER_SIZE;
	chunk->next = nullptr;
	chunk->canary = EXPECTED_CANARY;

	// Insert the chunk into the linked list
	m_last_memory_chunk->next = chunk;
	chunk->prev = m_last_memory_chunk;
	m_last_memory_chunk = chunk;

	// If it is possible to merge the new chunk with the previous chunk then do so (note: this happens if the
	// previous chunk is free but cant contain the size required)
	if(!chunk->prev_chunk()->allocated)
		unallocate((void*) ((size_t) chunk + CHUNK_HEADER_SIZE));

	return chunk;
}

void BumpAllocator::setup_region(uintptr_t address, size_t length) {

	// Create a free chunk that covers the entire region
	this->m_first_memory_chunk = (chunk_header_t*)address;
	m_first_memory_chunk->allocated = false;
	m_first_memory_chunk->prev = nullptr;
	m_first_memory_chunk->next = nullptr;
	m_first_memory_chunk->size = length - CHUNK_HEADER_SIZE;
	m_first_memory_chunk->canary = EXPECTED_CANARY;

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

	chunk_header_t* result = nullptr;

	// Nothing to allocate
	if(size == 0)
		return nullptr;

	size = align(size);

	// Find the next free chunk that is big enough
	for(chunk_header_t* chunk = m_first_memory_chunk; chunk != nullptr && result == nullptr; chunk = chunk->next_chunk()) {
		if(chunk->size >= size && !chunk->allocated)
			result = chunk;
	}

	// If there is no free chunk then make more room
	if(result == nullptr)
		result = (chunk_header_t*)expand_heap(size);

	// No space to expand heap
	if (result == nullptr)
		return nullptr;

	// If there is not left over space to store extra chunks there is no need to split the chunk
	if(result->size < size + CHUNK_HEADER_SIZE + CHUNK_ALIGNMENT) {
		result->allocated = true;
		void* p = (void*) (((size_t) result) + CHUNK_HEADER_SIZE);
		return p;
	}

	// Split the chunk into: what was requested + free overflow space for future allocates
	//  - This prevents waste in the event that a big free chunk was found but the requested size would only use a portion of that
	auto* extra = (chunk_header_t*) ((size_t) result + CHUNK_HEADER_SIZE + size);
	extra->allocated = false;
	extra->size = result->size - size - CHUNK_HEADER_SIZE;
	extra->prev = result;
	extra->canary = EXPECTED_CANARY;

	// Add to the linked list
	extra->next = result->next;
	if(extra->next != nullptr)
		extra->next->prev = extra;

	// Requested chunk is now allocated exactly to the size requested and points to the free (split) block of memory that
	// it did not use
	result->size = size;
	result->allocated = true;
	result->next = extra;
	result->canary = EXPECTED_CANARY;

	// Update the last memory chunk if necessary
	if(result == m_last_memory_chunk)
		m_last_memory_chunk = extra;

	return (void*) (((size_t) result) + CHUNK_HEADER_SIZE);
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
	auto heap_start = (uintptr_t)m_first_memory_chunk;
	auto heap_end = (uintptr_t)m_last_memory_chunk + CHUNK_HEADER_SIZE + m_last_memory_chunk->size;
	if((uintptr_t) pointer < heap_start || (uintptr_t) pointer > heap_end)
		return;

	// Get the chunk information from the pointer
	auto* chunk = (chunk_header_t*) ((size_t) pointer - CHUNK_HEADER_SIZE);
	chunk->allocated = false;

	// If there is a free chunk before this chunk then merge them
	if(chunk->prev_chunk() != nullptr && !chunk->prev_chunk()->allocated) {

		// If this chunk is the last one then the one it merges into is now the last one
		if (chunk == m_last_memory_chunk)
			m_last_memory_chunk = chunk->prev_chunk();

		// Grow the chunk behind this one so that it now contains the freed one
		chunk->prev_chunk()->size += chunk->size + CHUNK_HEADER_SIZE;
		chunk->prev_chunk()->next = chunk->next_chunk();

		// The chunk in front of the freed one now needs to point to the merged chunk
		if(chunk->next_chunk() != nullptr)
			chunk->next_chunk()->prev = chunk->prev_chunk();

		// Freed chunk doesn't exist anymore so now working with the merged chunk
		chunk = chunk->prev_chunk();

	}

	// If there is a free chunk after this chunk then merge them
	if(chunk->next_chunk() != nullptr && !chunk->next_chunk()->allocated) {

		// Merging with the last mem chunk means this chunk is now the last mem chunk
		if (chunk->next_chunk() == m_last_memory_chunk)
			m_last_memory_chunk = chunk;

		// Grow this chunk so that it now contains the free chunk in front of the old (now freed) one
		chunk->size += chunk->next_chunk()->size + CHUNK_HEADER_SIZE;

		// Now that this chunk contains the next one, it has to point to the one in front of what has just been merged
		// and that has to point to this
		chunk->next = chunk->next_chunk()->next_chunk();
		if(chunk->next_chunk() != nullptr)
			chunk->next_chunk()->prev = chunk;

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
	for(chunk_header_t* chunk = m_first_memory_chunk; chunk != nullptr; chunk = chunk->next_chunk())
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

	return(size + CHUNK_ALIGNMENT - 1) / CHUNK_ALIGNMENT * CHUNK_ALIGNMENT;
}