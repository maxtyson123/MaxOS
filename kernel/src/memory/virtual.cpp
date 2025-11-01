/**
 * @file virtual.cpp
 * @brief Implementation of a Virtual Memory Manager for managing virtual memory
 *
 * @date 11th February 2024
 * @author Max Tyson
 */

#include <memory/virtual.h>
#include <common/logger.h>
#include <processes/scheduler.h>

using namespace MaxOS::memory;
using namespace MaxOS::common;
using namespace MaxOS::processes;

VirtualMemoryManager::VirtualMemoryManager() {

	// Set the kernel flag
	bool is_kernel = MemoryManager::s_kernel_memory_manager == nullptr;
	if (!is_kernel) {

		// Get a new pml4 table
		m_pml4_root_physical_address = (uint64_t*) PhysicalMemoryManager::s_current_manager->allocate_frame();
		m_pml4_root_address = (uint64_t*) PhysicalMemoryManager::to_dm_region((uint64_t) m_pml4_root_physical_address);

		// Clear the table
		PhysicalMemoryManager::clean_page_table(m_pml4_root_address);

		// Map the higher half of the kernel (p4 256 - 511)
		for (size_t i = 256; i < 512; i++) {

			// Recursive Map the pml4 table (so that we can access the new pml4 table later on)
			if (i == 510) {
				m_pml4_root_address[i] = (uint64_t) m_pml4_root_physical_address | PRESENT | WRITE;
				continue;
			}

			// Set the new pml4 table to the old (kernel) pml4 table
			m_pml4_root_address[i] = PhysicalMemoryManager::s_current_manager->pml4_root_address()[i];

		}
		Logger::DEBUG() << "Mapped higher half of kernel\n";


	} else {
		m_pml4_root_address = PhysicalMemoryManager::s_current_manager->pml4_root_address();
		m_pml4_root_physical_address = (uint64_t*) PhysicalMemoryManager::to_lower_region((uint64_t) m_pml4_root_address);
	}

	// Allocate space for the vmm
	uint64_t vmm_space = PhysicalMemoryManager::align_to_page(HIGHER_HALF_DIRECT_MAP + PhysicalMemoryManager::s_current_manager->memory_size() + PAGE_SIZE);
	void* vmm_space_physical = PhysicalMemoryManager::s_current_manager->allocate_frame();
	PhysicalMemoryManager::s_current_manager->map(vmm_space_physical, (virtual_address_t*) vmm_space, PRESENT | WRITE, m_pml4_root_address);
	Logger::DEBUG() << "VMM space: physical - 0x" << (uint64_t) vmm_space_physical << ", virtual - 0x" << (uint64_t) vmm_space << "\n";

	// Make sure everything is mapped correctly
	if (!is_kernel)
		ASSERT(vmm_space_physical != PhysicalMemoryManager::s_current_manager->get_physical_address((virtual_address_t*) vmm_space, m_pml4_root_address), "Physical address does not match mapped address: 0x%x != 0x%x\n", vmm_space_physical,
			   PhysicalMemoryManager::s_current_manager->get_physical_address((virtual_address_t*) vmm_space, m_pml4_root_address));

	// Set the first region
	m_first_region = (virtual_memory_region_t*) vmm_space;
	m_current_region = m_first_region;
	m_first_region->next = nullptr;

	// Calculate the next available address (kernel needs to reserve space for the higher half)
	m_next_available_address = is_kernel ? vmm_space + VMM_RESERVED : PAGE_SIZE;
	Logger::DEBUG() << "Next available address: 0x" << m_next_available_address << "\n";

}

VirtualMemoryManager::~VirtualMemoryManager() {

	// Free all the frames used by the VMM
	virtual_memory_region_t* region = m_first_region;

	// Loop through the regions
	while (region != nullptr) {

		// Loop through the chunks
		for (size_t i = 0; i < CHUNKS_PER_PAGE; i++) {

			// Have reached the end?
			if (i == m_current_chunk && region == m_current_region)
				break;

			// Loop through the pages
			size_t pages = PhysicalMemoryManager::size_to_frames(region->chunks[i].size);
			for (size_t j = 0; j < pages; j++) {

				// Get the frame
				physical_address_t* frame = PhysicalMemoryManager::s_current_manager->get_physical_address((virtual_address_t*) region->chunks[i].start_address + (j * PAGE_SIZE), m_pml4_root_address);

				// Free the frame
				PhysicalMemoryManager::s_current_manager->free_frame(frame);

			}
		}

		// Move to the next region
		region = region->next;

	}
}


/**
 * @brief Allocate a new chunk of virtual memory
 *
 * @param size The size of the memory to allocate
 * @param flags The flags to set on the memory
 * @return The address of the allocated memory
 */
void* VirtualMemoryManager::allocate(size_t size, size_t flags) {

	return allocate(0, size, flags);
}


/**
 * @brief Allocate a new chunk of virtual memory at a specific address (ie for mmap io devices)
 *
 * @param address The address to allocate at
 * @param size The size of the memory to allocate
 * @param flags The flags to set on the memory
 * @return The address of the allocated memory or nullptr if failed
 */
void* VirtualMemoryManager::allocate(uint64_t address, size_t size, size_t flags) {

	// Make sure allocating something
	if (size == 0)
		return nullptr;

	// If specific address is given
	if (address != 0) {

		// Make sure isn't already allocated
		if (address < m_next_available_address)
			return nullptr;

		// Make sure its aligned
		if (!PhysicalMemoryManager::check_aligned(address))
			return nullptr;

	}

	// Make sure the size is aligned
	size = PhysicalMemoryManager::align_up_to_page(size, PAGE_SIZE);

	// Check the free list for a chunk (if not asking for a specific address)
	free_chunk_t* reusable_chunk = address == 0 ? find_and_remove_free_chunk(size) : nullptr;
	if (reusable_chunk != nullptr) {

		// If the chunk is not being reserved then the old memory needs to be unmapped
		if (flags & VirtualFlags::RESERVE) {

			// Unmap the memory
			size_t pages = PhysicalMemoryManager::size_to_frames(size);
			for (size_t i = 0; i < pages; i++) {

				// Get the frame
				physical_address_t* frame = PhysicalMemoryManager::s_current_manager->get_physical_address((virtual_address_t*) reusable_chunk->start_address + (i * PAGE_SIZE), m_pml4_root_address);

				// Free the frame
				PhysicalMemoryManager::s_current_manager->free_frame(frame);

			}
		}

		// Return the address
		return (void*) reusable_chunk->start_address;
	}


	// Is there space in the current region
	if (m_current_chunk >= CHUNKS_PER_PAGE)
		new_region();

	// If needed to allocate at a specific address, fill with free memory up to that address to prevent fragmentation
	if (address != 0)
		fill_up_to_address(address, flags, false);

	// Allocate the memory
	virtual_memory_chunk_t* chunk = &m_current_region->chunks[m_current_chunk];
	chunk->size = size;
	chunk->flags = flags;
	chunk->start_address = m_next_available_address;

	// Update the next available address
	m_next_available_address += size;
	m_current_chunk++;

	// If just reserving the space don't map it
	if (flags & RESERVE)
		return (void*) chunk->start_address;

	// Map the memory
	size_t pages = PhysicalMemoryManager::size_to_frames(size);
	for (size_t i = 0; i < pages; i++) {

		// Allocate a new frame
		physical_address_t* frame = PhysicalMemoryManager::s_current_manager->allocate_frame();
		ASSERT(frame != nullptr, "Failed to allocate frame (from current region)\n");

		// Map the frame
		PhysicalMemoryManager::s_current_manager->map(frame, (virtual_address_t*) chunk->start_address + (i * PAGE_SIZE), PRESENT | WRITE, m_pml4_root_address);

	}

	// Return the address
	return (void*) chunk->start_address;
}

/**
 * @brief Create a mew region in the VMM to use for allocation of more chunks
 */
void VirtualMemoryManager::new_region() {

	// Space for the new region
	physical_address_t* new_region_physical = PhysicalMemoryManager::s_current_manager->allocate_frame();
	ASSERT(new_region_physical != nullptr, "Failed to allocate new VMM region\n");

	// Align the new region
	auto* new_region = (virtual_memory_region_t*) PhysicalMemoryManager::align_to_page((uint64_t) m_current_region + PAGE_SIZE);

	// Map the new region
	PhysicalMemoryManager::s_current_manager->map(new_region_physical, (virtual_address_t*) new_region, PRESENT | WRITE, m_pml4_root_address);
	new_region->next = nullptr;

	// Clear the new region
	for (size_t i = 0; i < CHUNKS_PER_PAGE; i++) {
		new_region->chunks[i].size = 0;
		new_region->chunks[i].flags = 0;
		new_region->chunks[i].start_address = 0;
	}

	// Set the current region
	m_current_region->next = new_region;
	m_current_chunk = 0;
	m_current_region = new_region;
}

/**
 * @brief Free a chunk of virtual memory
 *
 * @param address The address of the memory to free
 */
void VirtualMemoryManager::free(void* address) {

	// Make sure freeing something
	if (address == nullptr)
		return;

	// Find the chunk
	virtual_memory_region_t* region = m_first_region;
	virtual_memory_chunk_t* chunk = nullptr;
	while (region != nullptr) {

		// Loop through the chunks
		for (size_t i = 0; i < CHUNKS_PER_PAGE; i++) {

			// Check if the address is in the chunk
			if (region->chunks[i].start_address == (uintptr_t) address) {
				chunk = &region->chunks[i];
				break;
			}
		}

		// If the chunk was found
		if (chunk != nullptr)
			break;

		// Move to the next region
		region = region->next;
	}

	// Make sure the chunk was found
	if (chunk == nullptr)
		return;

	// If the chunk is shared, don't unmap it incase other processes are using it
	if (chunk->flags & VirtualFlags::SHARED) {

		// Find the resource
		for(const auto& resource : GlobalScheduler::current_process()->resource_manager.resources()){

			// Skip non-shared memory resources
			if(resource.second->type() != resource_type_t::SHARED_MEMORY)
				continue;

			// Skip shared memory that points elsewhere
			auto shared = (SharedMemory*)resource.second;
			if((void*)shared->physical_address() != address)
				continue;

			// Close the resource
			GlobalScheduler::current_process()->resource_manager.close_resource(resource.first, 0);
		}
	}

	// Add the chunk to the free list
	add_free_chunk(chunk->start_address, chunk->size);

	// Clear the chunk
	chunk->size = 0;
	chunk->flags = 0;
	chunk->start_address = 0;
}

/**
 * @brief Returns the amount of memory used
 *
 * @return The amount of memory used
 */
size_t VirtualMemoryManager::memory_used() {

	// Loop through all the regions and add up the size of the allocated chunks
	size_t result = 0;

	// Iterate through the regions
	virtual_memory_region_t* region = m_first_region;
	while (region != nullptr) {

		// Loop through the chunks
		for (size_t i = 0; i < CHUNKS_PER_PAGE; i++) {

			// Check if the address is in the chunk
			if (region->chunks[i].size != 0)
				result += region->chunks[i].size;
		}

		// Move to the next region
		region = region->next;
	}

	return result;
}

/**
 * @brief Add a free chunk to the free list
 *
 * @param start_address The start address of the chunk
 * @param size The size of the chunk
 */
void VirtualMemoryManager::add_free_chunk(uintptr_t start_address, size_t size) {

	// Create the new chunk
	auto* new_chunk = (free_chunk_t*) start_address;
	new_chunk->start_address = start_address;
	new_chunk->size = size;
	new_chunk->next = m_free_chunks;

	// Set the new chunk
	m_free_chunks = new_chunk;
}

/**
 * @brief Find and remove a free chunk from the free list to use
 *
 * @param size The size of the chunk to find
 * @return The free chunk or nullptr if not found
 *
 * @todo Split the chunk if it is too big
 */
free_chunk_t* VirtualMemoryManager::find_and_remove_free_chunk(size_t size) {

	// Find the chunk
	free_chunk_t* current = m_free_chunks;
	free_chunk_t* previous = nullptr;
	while (current != nullptr) {

		// Check if the chunk is big enough
		if (current->size >= size) {

			// Remove the chunk
			if (previous != nullptr) {
				previous->next = current->next;
			} else {
				m_free_chunks = current->next;
			}

			// Return the chunk
			return current;

		}

		// Move to the next chunk
		previous = current;
		current = current->next;
	}

	// No chunk found
	return nullptr;

}

/**
 * @brief Get the physical address of the PML4 root
 *
 * @return The physical address of the PML4 root
 */
uint64_t* VirtualMemoryManager::pml4_root_address_physical() {

	return m_pml4_root_physical_address;
}

/**
 * @brief Load shared memory into the VMM's address space
 *
 * @param name The name of the shared memory
 * @return The address of the shared memory in the VMM's address space
 */
void* VirtualMemoryManager::load_shared_memory(const string& name) {

	// Get the shared memory block
	auto block = (SharedMemory*)GlobalScheduler::current_process()->resource_manager.get_resource(name);

	// Load the shared memory
	if (block != nullptr)
		return load_shared_memory(block->physical_address(), block->size());

	return nullptr;
}

/**
 * @brief Load shared memory into the VMM's address space
 *
 * @param physical_address The physical address of the shared memory
 * @param size The size of the shared memory
 * @return The address of the shared memory in the VMM's address space
 */
void* VirtualMemoryManager::load_shared_memory(uintptr_t physical_address, size_t size) {

	// Make sure there is somthing to map
	if (size == 0 || physical_address == 0)
		return nullptr;

	// Load it into physical memory
	return load_physical_into_address_space(physical_address, size, SHARED);
}

/**
 * @brief Load physical memory into the VMM's address space
 *
 * @param physical_address The physical address of the memory
 * @param size The size of the memory
 * @param flags The flags to set on the memory
 * @return The address of the memory in the VMM's address space
 */
void* VirtualMemoryManager::load_physical_into_address_space(uintptr_t physical_address, size_t size, size_t flags) {

	// Reserve some space
	void* address = allocate(size, flags | RESERVE);

	// Map the shared memory
	size_t pages = PhysicalMemoryManager::size_to_frames(size);
	for (size_t i = 0; i < pages; i++)
		PhysicalMemoryManager::s_current_manager->map((physical_address_t*) (physical_address + (i * PAGE_SIZE)), (virtual_address_t*) ((uintptr_t) address + (i * PAGE_SIZE)), PRESENT | WRITE, m_pml4_root_address);


	// All done
	return address;
}

/**
 * @brief Fill from the current address to the given address
 *
 * @param address The address to fill up to
 * @param flags The flags to set on the memory
 * @param mark_used If true, mark the memory as used
 */
void VirtualMemoryManager::fill_up_to_address(uintptr_t address, size_t flags, bool mark_used) {

	// Make sure the address is aligned
	address = PhysicalMemoryManager::align_to_page(address);

	// Make sure the address is not before the next available address
	ASSERT(address >= m_next_available_address, "FAILED TO FILL: Address is before the next available address - 0x%x < 0x%x\n", address, m_next_available_address);

	// Calculate the size
	size_t size = address - m_next_available_address;

	// Allocate the memory
	virtual_memory_chunk_t* chunk = &m_current_region->chunks[m_current_chunk];
	chunk->size = size;
	chunk->flags = flags;
	chunk->start_address = m_next_available_address;

	// Update the next available address
	m_next_available_address += size;
	m_current_chunk++;

	// Map the memory
	size_t pages = PhysicalMemoryManager::size_to_frames(size);
	for (size_t i = 0; i < pages; i++) {

		// Allocate a new frame
		physical_address_t* frame = PhysicalMemoryManager::s_current_manager->allocate_frame();
		ASSERT(frame != nullptr, "Failed to allocate frame (from fill up)\n");

		// Map the frame
		PhysicalMemoryManager::s_current_manager->map(frame, (virtual_address_t*) chunk->start_address + (i * PAGE_SIZE), PRESENT | WRITE, m_pml4_root_address);

	}

	// Mark as free if needed
	if (!mark_used)
		free((void*) chunk->start_address);
}

/**
 * @brief Get the virtual address of the PML4 root
 *
 * @return The virtual address or nullptr if not found
 */
uint64_t* VirtualMemoryManager::pml4_root_address() {

	// Make sure the address is valid
	if (m_pml4_root_address == nullptr)
		return nullptr;

	// Return the address
	return m_pml4_root_address;
}