//
// Created by 98max on 1/30/2024.
//

#include <common/logger.h>
#include <memory/physical.h>
#include <system/cpu.h>

using namespace MaxOS::memory;
using namespace MaxOS::system;
using namespace MaxOS::common;

volatile extern __attribute__((aligned(4096))) unsigned char p4_table[];
volatile extern __attribute__((aligned(4096))) unsigned char p3_table[];
volatile extern __attribute__((aligned(4096))) unsigned char p3_table_hh[];
volatile extern __attribute__((aligned(4096))) unsigned char p2_table[];
volatile extern __attribute__((aligned(4096))) unsigned char p1_table[];

extern uint64_t _kernel_end;
extern uint64_t _kernel_size;
extern uint64_t _kernel_physical_end;

PhysicalMemoryManager::PhysicalMemoryManager(Multiboot* multiboot)
: m_kernel_end((uint64_t) &_kernel_physical_end),
  m_multiboot(multiboot),
  m_pml4_root_address((uint64_t*) p4_table),
  m_pml4_root((pte_t*) p4_table)
{

	Logger::INFO() << "Setting up Physical Memory Manager\n";
	Logger::DEBUG() << "Kernel Memory: kernel_end = 0x" << (uint64_t) &_kernel_end << ", kernel_size = 0x" << (uint64_t) &_kernel_size << ", kernel_physical_end = 0x" << (uint64_t) &_kernel_physical_end << "\n";
	m_kernel_start_page = align_up_to_page((size_t) &_kernel_physical_end + PAGE_SIZE, PAGE_SIZE);

	// Set up the current manager
	unmap_lower_kernel();
	m_lock.unlock();
	s_current_manager = this;
	m_nx_allowed = CPU::check_nx();

  	// Find a region of memory available to be used
  	m_mmap_tag = m_multiboot->mmap();
  	for (multiboot_mmap_entry *entry = m_mmap_tag->entries; (multiboot_uint8_t *)entry < (multiboot_uint8_t *)m_mmap_tag + m_mmap_tag->size; entry = (multiboot_mmap_entry *)((unsigned long)entry + m_mmap_tag->entry_size)) {

		// Skip if the region is not free or there is not enough space
		if (entry->type != MULTIBOOT_MEMORY_AVAILABLE)
			continue;

		// Store the entry. (note: don't break here as it is desired to find the last usable entry as that is normally biggest)
		m_mmap = entry;
	}

	// Store the information about the bitmap
	m_memory_size = (m_mmap->addr + m_mmap->len);
	m_bitmap_size = m_memory_size / PAGE_SIZE + 1;
	m_total_entries = m_bitmap_size / ROW_BITS + 1;
	Logger::DEBUG() << "Memory Info: size = " << (int) (m_memory_size / 1024 / 1024) << "mb, bitmap size = 0x" << (uint64_t) m_bitmap_size << ", total entries = " << (int) m_total_entries << ", page size = 0x" << (uint64_t) PAGE_SIZE << "\n";

	// Map the physical memory into the virtual memory
	Logger::DEBUG() << "Mapping from 0x0 to 0x" << (uint64_t) (m_mmap->addr + m_mmap->len) << " to higher half direct map at offset 0x" << HIGHER_HALF_DIRECT_MAP << "\n";
	for (uint64_t physical_address = 0; physical_address < (m_mmap->addr + m_mmap->len); physical_address += PAGE_SIZE)
		map((physical_address_t*) physical_address, (virtual_address_t*) (HIGHER_HALF_DIRECT_MAP + physical_address), Present | Write);

	// Kernel Setup
	initialise_bit_map();
	reserve_kernel_regions(multiboot);

	// Initialisation Done
	m_initialized = true;
	Logger::DEBUG() << "Memory used at start: " << (int) (memory_used() / 1024 / 1024) << "mb \n";
}

PhysicalMemoryManager::~PhysicalMemoryManager() = default;


void PhysicalMemoryManager::reserve_kernel_regions(Multiboot* multiboot) {

	// Reserve the pages used by the higher half mapping
	reserve((uint64_t)m_mmap->addr, m_setup_frames * PAGE_SIZE, "HHDM");

	// Reserve the area for the bitmap
	reserve((uint64_t) from_dm_region((uint64_t) m_bit_map), m_bitmap_size / 8, "Bitmap");

	// Calculate how much space the kernel takes up
	uint32_t kernel_entries = (m_kernel_start_page / PAGE_SIZE) + 1;
	if ((((uint32_t) (m_kernel_start_page)) % PAGE_SIZE) != 0)
		kernel_entries += 1;

	// Reserve the kernel entries
	reserve(0, kernel_entries * PAGE_SIZE, "Kernel");

	// Reserve the area for the mmap
	uint64_t mem_end = m_mmap->addr + m_mmap->len;
	for (multiboot_mmap_entry* entry = m_mmap_tag->entries; (multiboot_uint8_t*) entry < (multiboot_uint8_t*) m_mmap_tag + m_mmap_tag->size; entry = (multiboot_mmap_entry*) ((unsigned long) entry + m_mmap_tag->entry_size)) {

		// Dont reserve free regions
		if (entry->type <= MULTIBOOT_MEMORY_AVAILABLE)
			continue;

		// Don't reserve the memory being managed by pmm
		if (entry->addr >= mem_end)
			continue;

		reserve(entry->addr, entry->len, "MMap");
	}

	// Reserve the area for each multiboot module
	for (multiboot_tag* tag = multiboot->start_tag(); tag->type != MULTIBOOT_TAG_TYPE_END; tag = (struct multiboot_tag*) ((multiboot_uint8_t*) tag + ((tag->size + 7) & ~7))) {

		if (tag->type != MULTIBOOT_TAG_TYPE_MODULE)
			continue;

		// Reserve the module's address
		auto* module = (struct multiboot_tag_module*) tag;
		reserve(module->mod_start, module->mod_end - module->mod_start, "Module");
	}

	// Reserve all the tags
	auto end_tag = (multiboot_tag*)to_higher_region(multiboot->end_address);
	auto start_tag = (uint64_t)to_lower_region((uintptr_t)multiboot->start_tag());
	size_t tags_size = multiboot->end_address - start_tag + ((end_tag->size + 7) & ~7);
	reserve(start_tag, tags_size, "Tags");
}

/**
 * @brief Converts a size to the number of frames
 *
 * @param size The size to convert
 * @return The number of frames
 */
size_t PhysicalMemoryManager::size_to_frames(size_t size) {

	return align_to_page(size) / PAGE_SIZE;
}

/**
 * @brief Aligns a size to the page size
 *
 * @param size The size to align
 * @return  The aligned size
 */
size_t PhysicalMemoryManager::align_to_page(size_t size) {

	return ((size + PAGE_SIZE - 1) / PAGE_SIZE) * PAGE_SIZE;
}

/**
 * @brief Aligns a size up to the page size
 *
 * @param size  The size to align
 * @param page_size The page size to align to
 * @return The aligned size
 */
size_t PhysicalMemoryManager::align_up_to_page(size_t size, size_t page_size) {

	return (size + page_size - 1) & ~(page_size - 1);
}

/**
 * @brief Checks if an address is aligned
 *
 * @param size The address to check
 * @return True if the address is aligned
 */
bool PhysicalMemoryManager::check_aligned(size_t size) {

	return (size % PAGE_SIZE) == 0;
}

/**
 * @brief Allocates a physical page of memory, if the PMM is not initalise it will use the anon memory instead of the bitmap
 *
 * @return The physical address of the page
 */
void* PhysicalMemoryManager::allocate_frame() {

	// Wait for the lock
	m_lock.lock();

	// If not initialised, cant use the bitmap or higher half mapped physical memory so use leftover kernel memory already
	// mapped in loader.s
	if (!m_initialized) {

		// Use frames at the start of the mmap free
		void* address = (void*)m_mmap->addr + (m_setup_frames * PAGE_SIZE);
		m_setup_frames++;

		m_lock.unlock();
		return address;
	}

	// Check if there are enough frames
	ASSERT(m_used_frames < m_bitmap_size, "No more frames available\n");

	for (uint32_t row = 0; row < m_total_entries; ++row) {

		// If the row is full continue
		if (m_bit_map[row] == 0xFFFFFFFFFFFFFFF)
			continue;

		for (uint32_t column = 0; column < ROW_BITS; ++column) {

			// Entry isn't free
			if (m_bit_map[row] & (1ULL << column))
				continue;

			// Mark the frame as used
			m_bit_map[row] |= (1ULL << column);
			m_used_frames++;

			// Thread safe
			m_lock.unlock();

			// Return the address
			uint64_t frame_address = (row * ROW_BITS) + column;
			return (void*) (frame_address * PAGE_SIZE);
		}
	}

	// Error frame not found
	ASSERT(false, "Frame not found\n");
	m_lock.unlock();
	return nullptr;
}

/**
 * @brief Frees a frame in the bit map
 *
 * @param address The address to free
 */
void PhysicalMemoryManager::free_frame(void* address) {

	m_lock.lock();

	// Mark the frame as not used
	m_used_frames--;
	uint64_t frame_address = (uint64_t) address / PAGE_SIZE;
	m_bit_map[frame_address / ROW_BITS] &= ~(1 << (frame_address % ROW_BITS));

	m_lock.unlock();
}

/**
 * @brief Allocate an area of physical memory (ie reserve it)
 *
 * @param start_address The start of the block
 * @param size The size to allocate
 * @return A pointer to the start of the block (physical address)
 */
void* PhysicalMemoryManager::allocate_area(uint64_t start_address, size_t size) {

	m_lock.lock();

	// Store the information about the frames needed to be allocated for this size
	size_t frame_count = size_to_frames(size);
	uint32_t start_row = 0;
	uint32_t start_column = 0;
	size_t adjacent_frames = 0;

	for (uint32_t row = 0; row < m_total_entries; ++row) {

		// Skip full rows
		if (m_bit_map[row] == 0xFFFFFFFFFFFFFFF)
			continue;

		for (uint32_t column = 0; column < ROW_BITS; ++column) {

			// Not enough adjacent frames
			if (m_bit_map[row] & (1ULL << column)) {
				adjacent_frames = 0;
				continue;
			}

			// Store the address of the first frame in set of adjacent ones
			if (adjacent_frames == 0) {
				start_row = row;
				start_column = column;
			}

			// Make sure there are enough frames in a row found
			adjacent_frames++;
			if (adjacent_frames != frame_count)
				continue;

			// Mark the frames as used
			m_used_frames += frame_count;
			for (uint32_t i = 0; i < frame_count; ++i) {

				// Get the location of the bit
				uint32_t index = start_row + (start_column + i) / ROW_BITS;
				uint32_t bit = (start_column + i) % ROW_BITS;

				// Check bounds
				ASSERT(index >= m_total_entries || bit >= ROW_BITS, "Index out of bounds\n");

				// Mark the bit as used
				m_bit_map[index] |= (1ULL << bit);
			}

			// Return start of the block of adjacent frames
			m_lock.unlock();
			return (void*) (start_address + (start_row * ROW_BITS + start_column) * PAGE_SIZE);

		}
	}

	// Not enough free frames adjacent to each other
	m_lock.unlock();
	ASSERT(false, "Cannot allocate that much memory\n");
	return nullptr;
}

/**
 * @brief Frees an area of physical memory
 *
 * @param start_address The start of the block
 * @param size The size to free
 */
void PhysicalMemoryManager::free_area(uint64_t start_address, size_t size) {

	// Convert address into frames
	size_t frame_count = size_to_frames(size);
	uint64_t frame_address = start_address / PAGE_SIZE;

	// Check bounds
	if (frame_address >= m_bitmap_size)
		return;

	// Wait until other threads have finished other memory operations
	m_lock.lock();

	// Mark the frames as not used
	m_used_frames -= frame_count;
	for (uint32_t i = 0; i < frame_count; ++i)
		m_bit_map[(frame_address + i) / ROW_BITS] &= ~(1 << ((frame_address + i) % ROW_BITS));

	m_lock.unlock();
}

/**
 * @brief Gets the higher half of a tabled address in the kernel pml4
 *
 * @param index The index of the table to get
 * @param index2 The index of the parent table
 * @param index3 The index of the parent's parent table
 * @return
 */
pml_t* PhysicalMemoryManager::get_higher_half_table(uint64_t index, uint64_t index2, uint64_t index3) {

	return (pml_t*) (0xFFFF000000000000 | ((510UL << 39) | (index3 << 30) | (index2 << 21) | (index << 12)));
}


/**
 * @brief Gets or creates a table in a table
 *
 * @param table The table to check
 * @param index The index of the table to get or create
 * @param flags The flags to set the table to
 * @return The created table
 */
pml_t* PhysicalMemoryManager::get_or_create_table(pml_t* table, size_t index, size_t flags) {

	// Table is already created so just find the entry
	if (table->entries[index].present)
		return (pml_t*) to_dm_region(physical_address_of_entry(&table->entries[index]));

	// Create the table
	auto* new_table = (uint64_t*) allocate_frame();
	table->entries[index] = create_page_table_entry((uint64_t) new_table, flags);

	// Move the table to the higher half
	new_table = (uint64_t*) to_dm_region((uintptr_t) new_table);

	// Reset the memory contents at the address
	clean_page_table(new_table);

	return (pml_t*) new_table;
}

/**
 * @brief Creates a page table entry when the higher half is not initialized
 *
 * @param parent_table The parent table to create the entry in
 * @param table_index The index of the table to create
 * @param table The table to create
 *
 * @return The created page table entry
 */
pml_t* PhysicalMemoryManager::get_and_create_table(pml_t* parent_table, uint64_t table_index, pml_t* table) {

	// Table already created so dont need to do anything
	/// Note: this is where it differs when not having pmm init done as cant find the entry (direct map not setup) thus
	///       requiring get_higher_half_table() to be passed in as the *table arg by the caller
	if (parent_table->entries[table_index].present)
		return table;

	// Create the table
	auto* new_table = (uint64_t*) allocate_frame();
	parent_table->entries[table_index] = create_page_table_entry((uint64_t) new_table, Present | Write);

	// Move the table to higher half
	// Except this doesn't need to be done because using anom memory

	// Reset the memory contents at the address
	clean_page_table((uint64_t*) table);

	return table;
}


/**
 * @brief Get the page table entry for a virtual address
 *
 * @param virtual_address The virtual address to get the entry for
 * @return The page table entry for the virtual address or nullptr if not found
 */
pte_t* PhysicalMemoryManager::get_entry(virtual_address_t* virtual_address, pml_t* pml4_table) {

	// Kernel memory must be in the higher half
	size_t flags = Present | Write;
	if (!in_higher_region((uint64_t) virtual_address))
		flags |= User;

	uint16_t pml4_index = PML4_GET_INDEX((uint64_t) virtual_address);
	uint16_t pdpr_index = PML3_GET_INDEX((uint64_t) virtual_address);
	uint16_t pd_index = PML2_GET_INDEX((uint64_t) virtual_address);
	uint16_t pt_index = PML1_GET_INDEX((uint64_t) virtual_address);

	pml_t* pdpr_table = nullptr;
	pml_t* pd_table = nullptr;
	pml_t* pt_table = nullptr;

	// If it is before initialization then cant rely on the direct map
	if (!m_initialized) {
		pdpr_table = get_and_create_table(pml4_table, pml4_index, get_higher_half_table(pml4_index));
		pd_table = get_and_create_table(pdpr_table, pdpr_index, get_higher_half_table(pdpr_index, pml4_index));
		pt_table = get_and_create_table(pd_table, pd_index, get_higher_half_table(pd_index, pdpr_index, pml4_index));

	} else {
		pdpr_table = get_or_create_table(pml4_table, pml4_index, flags);
		pd_table = get_or_create_table(pdpr_table, pdpr_index, flags);
		pt_table = get_or_create_table(pd_table, pd_index, flags);
	}

	// Get the entry
	return &pt_table->entries[pt_index];
}


/**
 * @brief Gets the physical address of an entry
 *
 * @param entry The entry to get the physical address of
 * @return The physical address of the entry
 */
uint64_t PhysicalMemoryManager::physical_address_of_entry(pte_t* entry) {

	return entry->physical_address << 12;
}


/**
 * @brief Maps a physical address to a virtual address, using the kernel's pml4 table
 *
 * @param physical_address The physical address to map
 * @param virtual_address The virtual address to map to
 * @param flags The flags to set the mapping to
 * @return The virtual address
 */
virtual_address_t* PhysicalMemoryManager::map(physical_address_t* physical_address, virtual_address_t* virtual_address, size_t flags) {

	// Map using the kernel's pml4 table
	return map(physical_address, virtual_address, flags, m_pml4_root_address);
}

/**
 * @brief Maps a physical address to a virtual address
 *
 * @param physical_address The physical address
 * @param virtual_address The virtual address
 * @param flags The flags to set the mapping to
 * @param pml4_table The pml4 table to use
 * @return The virtual address
 */
virtual_address_t* PhysicalMemoryManager::map(physical_address_t* physical_address, virtual_address_t* virtual_address, size_t flags, uint64_t* pml4_table) {

	// If it is in a lower region then assume it is the user space
	if (!in_higher_region((uint64_t) virtual_address))
		flags |= User;

	// If the entry already exists then the mapping is already done
	pte_t* pte = get_entry(virtual_address, (pml_t*) pml4_table);
	if (pte->present)
		return virtual_address;

	// Map the physical address to the virtual address
	*pte = create_page_table_entry((uint64_t) physical_address, flags);

	// Flush the TLB (cache)
	asm volatile("invlpg (%0)"::"r" (virtual_address) : "memory");

	return virtual_address;
}

/**
 * @brief Allocates a physical address to a virtual address
 *
 * @param virtual_address The virtual address
 * @param flags The flags to set the mapping to
 * @return The virtual address
 */
virtual_address_t* PhysicalMemoryManager::map(virtual_address_t* virtual_address, size_t flags) {

	// Map a new physical address to the requested virtual address
	return map(allocate_frame(), virtual_address, flags);

}

/**
 * @brief Allocates a new area physical memory to a area virtual address
 *
 * @param virtual_address_start The start of the virtual address
 * @param length The length of the area
 * @param flags The flags to set the mapping to
 */
void PhysicalMemoryManager::map_area(virtual_address_t* virtual_address_start, size_t length, size_t flags) {

	// Map the required frames
	for (size_t i = 0; i < size_to_frames(length); ++i)
		map(virtual_address_start + (i * PAGE_SIZE), flags);

}

/**
 * @brief Maps an area of physical memory to a virtual address
 *
 * @param physical_address_start The start of the physical address
 * @param virtual_address_start The start of the virtual address
 * @param length  The length of the area
 * @param flags The flags to set the mapping to
 */
void PhysicalMemoryManager::map_area(physical_address_t* physical_address_start, virtual_address_t* virtual_address_start, size_t length, size_t flags) {

	// Map the required frames
	for (size_t i = 0; i < size_to_frames(length); ++i)
		map(physical_address_start + (i * PAGE_SIZE), virtual_address_start + (i * PAGE_SIZE), flags);
}

/**
 * @brief Maps a physical address to its virtual address counter-part
 *
 * @param physical_address The physical address to map
 * @param flags The flags to set the mapping to
 */
void PhysicalMemoryManager::identity_map(physical_address_t* physical_address, size_t flags) {

	// Map the physical address to its virtual address counter-part
	map(physical_address, physical_address, flags);
}

/**
 * @brief Unmaps a virtual address using the kernel's pml4 table
 *
 * @param virtual_address The virtual address to unmap
 */
void PhysicalMemoryManager::unmap(virtual_address_t* virtual_address) {

	// Pass the kernel's pml4 table
	unmap(virtual_address, m_pml4_root_address);
}

/**
 * @brief Unmaps a virtual address
 *
 * @param virtual_address The virtual address to unmap
 * @param pml4_root The pml4 table to use
 */
void PhysicalMemoryManager::unmap(virtual_address_t* virtual_address, uint64_t* pml4_root) {

	// Get the entry
	pte_t* pte = get_entry(virtual_address, (pml_t*) pml4_root);

	// Make sure the address is actually mapped
	if (!pte->present)
		return;

	// Unmap the entry
	pte->present = false;

	// Flush the TLB (cache)
	asm volatile("invlpg (%0)"::"r" (virtual_address) : "memory");
}

/**
 * @brief Unmaps an area of virtual memory
 *
 * @param virtual_address_start The start of the area
 * @param length The length of the area
 */
void PhysicalMemoryManager::unmap_area(virtual_address_t* virtual_address_start, size_t length) {

	// Unmap the required frames
	for (size_t i = 0; i < size_to_frames(length); ++i)
		unmap(virtual_address_start + (i * PAGE_SIZE));
}

/**
 * @brief Cleans a page table (fills it with 0 or null entries)
 *
 * @param table The table to clean
 */
void PhysicalMemoryManager::clean_page_table(uint64_t* table) {

	// Null the table (prevents false mappings when re-using frames)
	for (int i = 0; i < 512; i++)
		table[i] = 0x00l;
}

/**
 * @brief Creates a page table entry
 *
 * @param address The address to create the entry for
 * @param flags The flags to set the entry to
 * @return The created page table entry
 */
pte_t PhysicalMemoryManager::create_page_table_entry(uintptr_t address, size_t flags) const {

	pte_t page = (pte_t) {
			.present            = (flags & Present) != 0,
			.write              = (flags & Write) != 0,
			.user               = (flags & User) != 0,
			.write_through      = (flags & WriteThrough) != 0,
			.cache_disabled     = (flags & CacheDisabled) != 0,
			.accessed           = (flags & Accessed) != 0,
			.dirty              = (flags & Dirty) != 0,
			.huge_page          = (flags & HugePage) != 0,
			.global             = (flags & Global) != 0,
			.available          = 0,
			.physical_address   = address >> 12,
	};

	// Set the NX bit if it is allowed
	if (m_nx_allowed && (flags & NoExecute)) {
		auto page_raw = (uint64_t) &page;
		page_raw |= NoExecute;
		page = *(pte_t*) page_raw;
	}

	return page;
}

/**
 * @brief Gets the address of the bitmap
 *
 * @return The address of the bitmap
 */
void PhysicalMemoryManager::initialise_bit_map() {

	// Earliest address to place the bitmap (after the kernel and hh direct map)
	uint64_t limit = m_kernel_start_page;

	// Find a region for the bitmap to handle
	for (multiboot_mmap_entry* entry = m_mmap_tag->entries; (multiboot_uint8_t*) entry < (multiboot_uint8_t*) m_mmap_tag + m_mmap_tag->size; entry = (multiboot_mmap_entry*) ((unsigned long) entry + m_mmap_tag->entry_size)) {

		// Cant use a non-free entry or an entry past limit (ie don't user higher addresses that will be overridden later)
		if (entry->type != MULTIBOOT_MEMORY_AVAILABLE || entry->len > limit)
			continue;

		size_t space = entry->len;
		size_t offset = 0;

		// Determine how much of the region is below the limit and adjust the starting point to there
		if (entry->addr < limit) {
			offset = limit - entry->addr;
			space -= offset;
		}

		// Make sure there is enough space
		ASSERT(space >= (m_bitmap_size / 8 + 1), "Not enough space for the bitmap (too big)\n");

		// Return the address (ensuring that it is in the safe region)
		m_bit_map = (uint64_t*) to_dm_region(entry->addr + offset);
		break;
	}

	// Error no suitable space for the bitmap found
	ASSERT(m_bit_map != nullptr, "No space for the bitmap (no region)\n");

	// Clear the bitmap (mark all as free)
	for (uint32_t i = 0; i < m_total_entries; ++i)
		m_bit_map[i] = 0;

	Logger::DEBUG() << "Bitmap: location: 0x" << (uint64_t) m_bit_map << " - 0x" << (uint64_t) (m_bit_map + m_bitmap_size / 8) << " (range of 0x" << (uint64_t) m_bitmap_size / 8 << ")\n";

}

/**
 * @brief Gets the pml4 root address for the kernel
 *
 * @return The pml4 root address
 */
uint64_t* PhysicalMemoryManager::pml4_root_address() {

	return m_pml4_root_address;
}

/**
 * @brief Gets total the memory size available for use (allocated or not)
 *
 * @return The memory size in bytes
 */
uint64_t PhysicalMemoryManager::memory_size() const {

	return m_memory_size;
}

/**
 * @brief Gets the memory currently used
 *
 * @return The memory size in bytes
 */
uint64_t PhysicalMemoryManager::memory_used() const {

	return m_used_frames * PAGE_SIZE;
}

/**
 * @brief Aligns a address to the page size
 *
 * @param size The address to align
 * @return The aligned address
 */
size_t PhysicalMemoryManager::align_direct_to_page(size_t size) {

	return (size & (~(PAGE_SIZE - 1)));
}

/**
 * @brief Reserves a physical address
 *
 * @param address The address to reserve
 */
void PhysicalMemoryManager::reserve(uint64_t address) {

	reserve(address, PAGE_SIZE);
}

/**
 * @brief Reserves an area of physical memory
 *
 * @param address The start of the area
 * @param size The size of the area
 * @param type The name for the region being reserved (not required, logging purposes only, defaults to unkown)
 */
void PhysicalMemoryManager::reserve(uint64_t address, size_t size, const char* type) {

	// Cant reserve virtual addresses (ensure the address is physical)
	ASSERT(address < m_memory_size, "Attempt to reserve address bigger then the memory can contain: 0x%x\n", address);
	ASSERT(address + size < m_memory_size, "Attempt to reserve region bigger then the memory can contain: 0x%x-0x%x\n", address, address+size);

	// Wait to be able to reserve
	m_lock.lock();

	// Align to a page, if rounding down need to correct the range
	size_t aligned_address = align_direct_to_page(address);
	if(aligned_address < address)
		size += address - aligned_address;

	// Convert in to amount of pages
	size = align_up_to_page(size, PAGE_SIZE);
	size_t page_count = size / PAGE_SIZE;
	uint64_t frame_index = aligned_address / PAGE_SIZE;

	// Mark all as used
	for (size_t i = 0; i < page_count; ++i)
		m_bit_map[(frame_index + i) / ROW_BITS] |= (1ULL << ((frame_index + i) % ROW_BITS));

	// Update the used frames
	m_used_frames += page_count;

	// Clear the lock
	m_lock.unlock();
	Logger::DEBUG() << "Reserved Address for "<< type << ": 0x" << aligned_address << " - 0x" << aligned_address + size << " (length of 0x" << size << ")\n";
}

/**
 * @brief Gets the physical address from a virtual address (if it exists)
 *
 * @param virtual_address The virtual address to get the physical address from
 * @param pml4_root The address of the root pml to use
 * @return  The physical address or nullptr if it does not exist
 */
physical_address_t* PhysicalMemoryManager::get_physical_address(virtual_address_t* virtual_address, uint64_t* pml4_root) {

	pte_t* entry = get_entry(virtual_address, (pml_t*) pml4_root);

	// Cant get a physical address if its inst free
	if (!entry->present)
		return nullptr;

	return (physical_address_t*) physical_address_of_entry(entry);
}

/**
 * @brief Changes the flags of a page
 *
 * @param virtual_address The virtual address of the page
 * @param flags The flags to set the page to
 * @param pml4_root The address of the root pml to use
 */
void PhysicalMemoryManager::change_page_flags(virtual_address_t* virtual_address, size_t flags, uint64_t* pml4_root) {

	pte_t* entry = get_entry(virtual_address, (pml_t*) pml4_root);

	// Cant edit a non-present entry (will page fault)
	if (!entry->present)
		return;

	*entry = create_page_table_entry(physical_address_of_entry(entry), flags);

	// Flush the TLB (cache)
	asm volatile("invlpg (%0)"::"r" (virtual_address) : "memory");

}

/**
 * @brief Checks if a physical address is mapped to a virtual address
 *
 * @param physical_address The physical address to check
 * @param virtual_address The virtual address to check
 * @param pml4_root The pml4 table to use
 * @return True if the physical address is mapped to the virtual address
 */
bool PhysicalMemoryManager::is_mapped(uintptr_t physical_address, uintptr_t virtual_address, uint64_t* pml4_root) {

	return get_physical_address((virtual_address_t*) virtual_address, pml4_root) == (physical_address_t*) physical_address;

}

/**
 * @brief Converts a physical address to a higher region address if it is in the lower region using the higher half kernel offset
 *
 * @param physical_address The physical address
 * @return The higher region address
 */
void* PhysicalMemoryManager::to_higher_region(uintptr_t physical_address) {

	// If it's in the lower half then add the offset
	if (physical_address < HIGHER_HALF_KERNEL_OFFSET)
		return (void*) (physical_address + HIGHER_HALF_KERNEL_OFFSET);

	// Must be in the higher half
	return (void*) physical_address;
}

/**
 * @brief Converts a virtual address to a lower region address if it is in the higher region using the higher half kernel offset
 *
 * @param virtual_address The virtual address
 * @return The lower region address
 */
void* PhysicalMemoryManager::to_lower_region(uintptr_t virtual_address) {

	// If it's in the lower half then add the offset
	if (virtual_address > HIGHER_HALF_KERNEL_OFFSET)
		return (void*) (virtual_address - HIGHER_HALF_KERNEL_OFFSET);

	// Must be in the lower half
	return (void*) virtual_address;
}

/**
 * @brief Converts a physical address to an IO region address if it is in the lower region using the higher half memory offset
 *
 * @param physical_address The physical address
 * @return The IO region address
 */
void* PhysicalMemoryManager::to_io_region(uintptr_t physical_address) {

	if (physical_address < HIGHER_HALF_MEM_OFFSET)
		return (void*) (physical_address + HIGHER_HALF_MEM_OFFSET);

	// Must be in the higher half
	return (void*) physical_address;
}

/**
 * @brief Converts a physical address to a direct map region address if it is in the lower region using the higher half direct map offset
 *
 * @param physical_address The physical address
 * @return The direct map region address
 */
void* PhysicalMemoryManager::to_dm_region(uintptr_t physical_address) {

	if (physical_address < HIGHER_HALF_OFFSET)
		return (void*) (physical_address + HIGHER_HALF_DIRECT_MAP);

	// Must be in the higher half
	return (void*) physical_address;
}

/**
 * @brief Converts a direct map region address to a physical address if it is in the higher region using the higher half direct map offset
 *
 * @param virtual_address The physical address in the direct map region
 * @return The physical address
 */
void* PhysicalMemoryManager::from_dm_region(uintptr_t virtual_address) {

	if (virtual_address > HIGHER_HALF_DIRECT_MAP)
		return (void*) (virtual_address - HIGHER_HALF_DIRECT_MAP);

	// Must be in the lower half
	return (void*) virtual_address;
}


/**
 * @brief Checks if a virtual address is in the higher region
 *
 * @param virtual_address The virtual address
 * @return True if the address is in the higher region, false otherwise
 */
bool PhysicalMemoryManager::in_higher_region(uintptr_t virtual_address) {

	return virtual_address & (1l << 62);
}


/**
 * @brief Unmaps the kernel physical memory from the lower half that was set up during the kernel boot
 */
void PhysicalMemoryManager::unmap_lower_kernel() {

	p4_table[0] = 0;
}