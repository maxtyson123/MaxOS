//
// Created by 98max on 8/07/2026.
//

#include <memory/allocator/allocator.h>

using namespace MaxOS;
using namespace MaxOS::memory;
using namespace MaxOS::memory::allocator;

/**
 * Default constructor for a generic memory allocator
 */
MemoryAllocator::MemoryAllocator() = default;

/**
 * Default destructor for a generic memory allocator
 */
MemoryAllocator::~MemoryAllocator() = default;

/**
 * @brief Expands the heap by a given size
 *
 * @param size The size to expand the heap by
 * @return The new chunk of memory
 */
void* MemoryAllocator::expand_heap(size_t size) {
	return nullptr;
}

/**
 * @brief Expands the memory region by a given size
 *
 * @param requested_size The size to expand the region by
 * @param actual_size The size that the region was expaned to
 * @return The new space of memory
 */
void* MemoryAllocator::allocate_extra_space(size_t requested_size, size_t& actual_size) {
	return nullptr;
}

/**
 *  @brief Setup a region to be bump memory managed
 *
 * @param address The start of the region
 * @param length The length of the region
 */
void MemoryAllocator::setup_region(uintptr_t address, size_t length) {
}

/**
 * @brief Allocates a block of memory
 *
 * @param size The size of the block to allocate
 * @return A pointer to the block, or nullptr if no block is available
 */
void* MemoryAllocator::allocate(size_t size) {
	return nullptr;
}

/**
 * @brief Frees a block of memory
 *
 * @param pointer A pointer to the block to free
 */
void MemoryAllocator::unallocate(void* pointer) {
}

/**
 * @brief Returns the amount of memory used (btyes)
 *
 * @return The amount of memory used
 */
size_t MemoryAllocator::amount_used() {
	return 0;
}

/**
 * @brief Aligns the size to the chunk alignment
 *
 * @param size The size to align
 * @return The aligned size
 */
size_t MemoryAllocator::align(size_t size) {
	return 0;
}




