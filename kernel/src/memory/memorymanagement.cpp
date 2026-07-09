/**
 * @file memorymanagement.cpp
 * @brief Implementation of a Memory Manager for dynamic memory allocation
 *
 * @date 20th October 2022
 * @author Max Tyson
 */

#include <memory/memorymanagement.h>
#include <common/logger.h>
#include <processes/scheduler.h>

using namespace MaxOS;
using namespace MaxOS::memory;
using namespace MaxOS::memory::allocator;
using namespace MaxOS::common;
using namespace MaxOS::processes;
using namespace MaxOS::system;

BumpMemoryManager::BumpMemoryManager(VirtualMemoryManager* virtual_memory_manager)
	: m_virtual_memory_manager(virtual_memory_manager)
{

	// Create the VMM if not provided
	if(m_virtual_memory_manager == nullptr)
		m_virtual_memory_manager = new VirtualMemoryManager();

	// Set up the first chunk of memory
	auto address = (uintptr_t)m_virtual_memory_manager->allocate(PAGE_SIZE + sizeof(MemoryChunk), 0);
	setup_region(address, PAGE_SIZE - sizeof(MemoryChunk));

}

BumpMemoryManager::~BumpMemoryManager() {

	// Free the VMM (if this is not the kernel memory manager)
	if(m_virtual_memory_manager != nullptr)
		delete m_virtual_memory_manager;

}

/**
 * @brief Expands the memory region by a given size
 *
 * @param size The size to expand the region by
 * @return The new space of memory
 */
void* BumpMemoryManager::allocate_extra_space(size_t size) {

	// Create a new chunk of memory
	auto* chunk = m_virtual_memory_manager->allocate(size, PRESENT | WRITE | NO_EXECUTE);
	ASSERT(chunk != nullptr, "Out of memory - kernel cannot allocate any more memory");

	return chunk;

}


/**
 * Gets the active virtual memory manager
 *
 * @return The active virtual memory manager
 */
VirtualMemoryManager* BumpMemoryManager::vmm() {

	return m_virtual_memory_manager;
}


/**
 * @brief Switches the active memory manager
 *
 * @param manager The new memory manager
 */
void BumpMemoryManager::switch_active_memory_manager(BumpMemoryManager* manager) {

	// Make sure there is a manager
	if(manager == nullptr)
		return;

	// Switch the address space
	asm volatile("mov %0, %%cr3"::"r"((uint64_t) manager->m_virtual_memory_manager->pml4_root_address_physical()) : "memory");

}

/**
 * @brief Construct a new Memory Manager object. Will switch the pml4 to use the calling process's page tables.
 *
 * @param vmm The virtual memory manager to use, if nullptr a new one will be created
 */
MemoryManager::MemoryManager(VirtualMemoryManager* vmm)
: BumpMemoryManager(vmm)
{

	// First memory manager is the kernel memory manager
	if(s_kernel_memory_manager == nullptr)
		s_kernel_memory_manager = this;

}

/**
 * @brief Destroy the Memory Manager object, frees the VMM if not the kernel memory manager
 */
MemoryManager::~MemoryManager() {

	// Remove the kernel reference to this
	if(s_kernel_memory_manager == this)
		s_kernel_memory_manager = nullptr;
}

/**
 * @brief Allocates a block of memory in the current USERSPACE heap
 *
 * @param size size of the block
 * @return a pointer to the block, 0 if no block is available or no memory manager is set
 */
void* MemoryManager::malloc(size_t size) {

	return GlobalScheduler::current_process()->memory_manager->allocate(size);
}

/**
 * @brief Allocates a block of memory in the KERNEL space
 *
 * @param size The size of the block
 * @return The pointer to the block, or nullptr if no block is available
 */
void* MemoryManager::kmalloc(size_t size) {

	// Make sure there is a kernel memory manager
	if(s_kernel_memory_manager == nullptr)
		return nullptr;

	return s_kernel_memory_manager->allocate(size);
}

/**
 * @brief Frees a block of memory using the current memory manager
 *
 * @param pointer The pointer to the block
 */
void MemoryManager::free(void* pointer) {

	return  GlobalScheduler::current_process()->memory_manager->unallocate(pointer);
}

/**
 * @brief Frees a block of memory using the kernel memory manager
 *
 * @param pointer The pointer to the block
 */
void MemoryManager::kfree(void* pointer) {

	// Make sure there is a kernel memory manager
	if(s_kernel_memory_manager == nullptr)
		return;

	s_kernel_memory_manager->unallocate(pointer);
}

//Redefine the default object functions with memory orientated ones (defaults disabled in makefile)