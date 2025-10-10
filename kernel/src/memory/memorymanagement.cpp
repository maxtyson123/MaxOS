//
// Created by 98max on 10/20/2022.
//

#include <memory/memorymanagement.h>
#include <common/logger.h>

using namespace MaxOS;
using namespace MaxOS::memory;
using namespace MaxOS::common;
using namespace MaxOS::system;

MemoryManager::MemoryManager(VirtualMemoryManager* vmm)
: m_virtual_memory_manager(vmm)
{

	// Create the VMM if not provided
	if (m_virtual_memory_manager == nullptr)
		m_virtual_memory_manager = new VirtualMemoryManager();

	// Enable the memory manager
	switch_active_memory_manager(this);

	// Setup the first chunk of memory
	this->m_first_memory_chunk = (MemoryChunk*) m_virtual_memory_manager->allocate(PAGE_SIZE + sizeof(MemoryChunk), 0);
	m_first_memory_chunk->allocated = false;
	m_first_memory_chunk->prev = nullptr;
	m_first_memory_chunk->next = nullptr;
	m_first_memory_chunk->size = PAGE_SIZE - sizeof(MemoryChunk);
	m_last_memory_chunk = m_first_memory_chunk;

	// First memory manager is the kernel memory manager
	if (s_kernel_memory_manager == nullptr)
		s_kernel_memory_manager = this;

}

MemoryManager::~MemoryManager() {

	// Free the VMM (if this is not the kernel memory manager)
	if (m_virtual_memory_manager != nullptr && s_current_memory_manager != s_kernel_memory_manager)
		delete m_virtual_memory_manager;

	// Remove the kernel reference to this
	if (s_kernel_memory_manager == this)
		s_kernel_memory_manager = nullptr;

	// Remove the active reference to this
	if (s_current_memory_manager == this)
		s_current_memory_manager = nullptr;
}

/**
 * @brief Allocates a block of memory in the current USERSPACE heap
 *
 * @param size size of the block
 * @return a pointer to the block, 0 if no block is available or no memory manager is set
 */
void* MemoryManager::malloc(size_t size) {

	// Make sure there is somthing to do the allocation
	if (s_current_memory_manager == nullptr)
		return nullptr;

	return s_current_memory_manager->handle_malloc(size);
}

/**
 * @brief Allocates a block of memory in the KERNEL space
 *
 * @param size The size of the block
 * @return The pointer to the block, or nullptr if no block is available
 */
void* MemoryManager::kmalloc(size_t size) {

	// Make sure there is a kernel memory manager
	if (s_kernel_memory_manager == nullptr)
		return nullptr;

	return s_kernel_memory_manager->handle_malloc(size);
}

/**
 * @brief Allocates a block of memory
 *
 * @param size The size of the block to allocate
 * @return A pointer to the block, or nullptr if no block is available
 */
void* MemoryManager::handle_malloc(size_t size) {

	MemoryChunk* result = nullptr;

	// Nothing to allocate
	if (size == 0)
		return nullptr;

	// Add room to store the chunk information
	size = align(size + sizeof(MemoryChunk));

	// Find the next free chunk that is big enough
	for (MemoryChunk* chunk = m_first_memory_chunk; chunk != nullptr && result == nullptr; chunk = chunk->next) {
		if (chunk->size > size && !chunk->allocated)
			result = chunk;
	}

	// If there is no free chunk then make more room
	if (result == nullptr)
		result = expand_heap(size);

	// If there is not left over space to store extra chunks there is no need to split the chunk
	if (result->size < size + sizeof(MemoryChunk) + 1) {
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
	if (extra->next != nullptr)
		extra->next->prev = extra;

	// Requested chunk is now allocated exactly to the size requested and points to the free (split) block of memory that
	// it did not use
	result->size = size;
	result->allocated = true;
	result->next = extra;

	// Update the last memory chunk if necessary
	if (result == m_last_memory_chunk)
		m_last_memory_chunk = extra;

	return (void*) (((size_t) result) + sizeof(MemoryChunk));
}

/**
 * @brief Frees a block of memory using the current memory manager
 *
 * @param pointer The pointer to the block
 */
void MemoryManager::free(void* pointer) {

	// Make sure there is a memory manager
	if (s_current_memory_manager == nullptr)
		return;

	s_current_memory_manager->handle_free(pointer);
}

/**
 * @brief Frees a block of memory using the kernel memory manager
 *
 * @param pointer The pointer to the block
 */
void MemoryManager::kfree(void* pointer) {

	// Make sure there is a kernel memory manager
	if (s_kernel_memory_manager == nullptr)
		return;

	s_kernel_memory_manager->handle_free(pointer);
}

/**
 * @brief Frees a block of memory
 *
 * @param pointer A pointer to the block
 */
void MemoryManager::handle_free(void* pointer) {

	// Cant free unallocated memory
	if (pointer == nullptr)
		return;

	// Check bounds
	if ((uint64_t) pointer < (uint64_t) m_first_memory_chunk || (uint64_t) pointer > (uint64_t) m_last_memory_chunk)
		return;

	// Get the chunk information from the pointer
	auto* chunk = (MemoryChunk*) ((size_t) pointer - sizeof(MemoryChunk));
	chunk->allocated = false;

	// If there is a free chunk before this chunk then merge them
	if (chunk->prev != nullptr && !chunk->prev->allocated) {

		// Grow the chunk behind this one so that it now contains the freed one
		chunk->prev->size += chunk->size + sizeof(MemoryChunk);
		chunk->prev->next = chunk->next;

		// The chunk in front of the freed one now needs to point to the merged chunk
		if (chunk->next != nullptr)
			chunk->next->prev = chunk->prev;

		// Freed chunk doesn't exist anymore so now working with the merged chunk
		chunk = chunk->prev;

	}

	// If there is a free chunk after this chunk then merge them
	if (chunk->next != nullptr && !chunk->next->allocated) {

		// Grow this chunk so that it now contains the free chunk in front of the old (now freed) one
		chunk->size += chunk->next->size + sizeof(MemoryChunk);

		// Now that this chunk contains the next one, it has to point to the one in front of what has just been merged
		// and that has to point to this
		chunk->next = chunk->next->next;
		if (chunk->next != nullptr)
			chunk->next->prev = chunk;

	}
}

/**
 * @brief Expands the heap by a given size
 *
 * @param size The size to expand the heap by
 * @return The new chunk of memory
 */
MemoryChunk* MemoryManager::expand_heap(size_t size) {

	// Create a new chunk of memory
	auto* chunk = (MemoryChunk*) m_virtual_memory_manager->allocate(size, Present | Write | NoExecute);
	ASSERT(chunk != nullptr, "Out of memory - kernel cannot allocate any more memory");

	// Handled by assert, but just in case
	if (chunk == nullptr)
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
	if (!chunk->prev->allocated)
		free((void*) ((size_t) chunk + sizeof(MemoryChunk)));

	return chunk;
}

/**
 * @brief Returns the amount of memory used
 *
 * @return The amount of memory used
 */
int MemoryManager::memory_used() {

	int result = 0;

	// Loop through all the chunks and add up the size of the allocated chunks
	for (MemoryChunk* chunk = m_first_memory_chunk; chunk != nullptr; chunk = chunk->next)
		if (chunk->allocated)
			result += chunk->size;

	return result;
}

/**
 * @brief Aligns the size to the chunk alignment
 *
 * @param size The size to align
 * @return The aligned size
 */
size_t MemoryManager::align(size_t size) {

	return (size / CHUNK_ALIGNMENT + 1) * CHUNK_ALIGNMENT;
}


/**
 * @brief Switches the active memory manager
 *
 * @param manager The new memory manager
 */
void MemoryManager::switch_active_memory_manager(MemoryManager* manager) {

	// Make sure there is a manager
	if (manager == nullptr)
		return;

	// Switch the address space
	asm volatile("mov %0, %%cr3"::"r"((uint64_t) manager->m_virtual_memory_manager->pml4_root_address_physical()) : "memory");

	// Set the active memory manager
	s_current_memory_manager = manager;
}

/**
 * Gets the active virtual memory manager
 *
 * @return The active virtual memory manager
 */
VirtualMemoryManager* MemoryManager::vmm() {

	return m_virtual_memory_manager;
}

//Redefine the default object functions with memory orientated ones (defaults disabled in makefile)


/**
 * @brief Overloaded new operator, allocates memory using the KERNEL memory manager
 *
 * @param size The size of the memory to allocate
 * @return The pointer to the allocated memory
 */
void* operator new(size_t size) throw() {

	// Handle the memory allocation
	return MaxOS::memory::MemoryManager::kmalloc(size);
}

/**
 * @brief Overloaded new operator, allocates memory using the KERNEL memory manager
 *
 * @param size The size of the memory to allocate
 * @return The pointer to the allocated memory
 */
void* operator new[](size_t size) throw() {

	// Handle the memory allocation
	return  MaxOS::memory::MemoryManager::kmalloc(size);
}

/**
 * @brief Overloaded new operator, allocates memory using the KERNEL memory manager
 *
 * @param pointer The pointer to the memory to allocate
 * @return The pointer to the memory
 */
void* operator new(size_t, void* pointer) {

	return pointer;
}

/**
 * @brief Overloaded new operator, allocates memory using the KERNEL memory manager
 *
 * @param pointer The pointer to the memory to allocate
 * @return The pointer to the memory
 */
void* operator new[](size_t, void* pointer) {


	return pointer;
}

/**
 * @brief Overloaded delete operator, frees memory using the KERNEL memory manager
 *
 * @param pointer The pointer to the memory to free
 */
void operator delete(void* pointer) {

	// Handle the memory freeing
	return MaxOS::memory::MemoryManager::kfree(pointer);
}

/**
 * @brief Overloaded delete operator, frees memory using the KERNEL memory manager
 *
 * @param pointer The pointer to the memory to free
 */
void operator delete[](void* pointer) {

	// Handle the memory freeing
	return MaxOS::memory::MemoryManager::kfree(pointer);
}

/**
 * @brief Overloaded delete operator, frees memory using the KERNEL memory manager
 *
 * @param pointer The pointer to the memory to free
 * @param size The size of the memory to free - ignored in this implementation
 */
void operator delete(void* pointer, size_t) {

	// Handle the memory freeing
	return MaxOS::memory::MemoryManager::kfree(pointer);
}

/**
 * @brief Overloaded delete operator, frees memory using the KERNEL memory manager
 *
 * @param pointer The pointer to the memory to free
 */
void operator delete[](void* pointer, size_t) {

	// Handle the memory freeing
	return MaxOS::memory::MemoryManager::kfree(pointer);
}