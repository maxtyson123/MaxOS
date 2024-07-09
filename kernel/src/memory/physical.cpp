//
// Created by 98max on 1/30/2024.
//

#include <common/kprint.h>
#include <memory/physical.h>

using namespace MaxOS::memory;
using namespace MaxOS::system;
extern uint64_t p4_table[];

PhysicalMemoryManager* PhysicalMemoryManager::current_manager = nullptr;

MaxOS::memory::PhysicalMemoryManager::PhysicalMemoryManager(unsigned long reserved, Multiboot* multiboot, uint64_t pml4_root[512]){

  // SEE boot.s FOR SETUP OF PAGING
  m_pml4_root = (pte_t*)pml4_root;
  m_pml4_root_address = pml4_root;

  // Set the current manager
  current_manager = this;

  // Store the information about the bitmap
  uint64_t memory_size = (multiboot->get_basic_meminfo() -> mem_upper + 1024) * 1024;
  m_bitmap_size = memory_size / PAGE_SIZE + 1;
  m_total_entries = m_bitmap_size / ROW_BITS + 1;

  // Loop through all the memory map entries
  multiboot_tag_mmap* mmap = multiboot->get_mmap();
  for (multiboot_mmap_entry *entry = mmap->entries; (multiboot_uint8_t *)entry < (multiboot_uint8_t *)mmap + mmap->size; entry = (multiboot_mmap_entry *)((unsigned long)entry + mmap->entry_size)) {

    // Skip if the region is not free or there is not enough space
    if (entry->type != MULTIBOOT_MEMORY_AVAILABLE || (entry->addr + entry->len) < reserved)
      continue;

    m_mmap = entry;
    break;
  }

  // Check if the reserved area is part of the mmap
  size_t offset = 0;
  if(reserved > m_mmap -> addr)
    offset = reserved - m_mmap -> addr;

  // Use the memory
  m_bit_map = (uint64_t *)(m_mmap->addr + offset);

  // Free all the entries
  for (uint32_t i = 0; i < m_total_entries; ++i)
    m_bit_map[i] = 0;

  // Calculate how much space the kernel takes up
  uint32_t kernel_entries = (reserved / PAGE_SIZE) + 1;
  if((((uint32_t)(reserved)) % PAGE_SIZE) != 0){
      // This means that there is kernel modules since there is spare space after one page
      kernel_entries += 1;
  }

  // Reserve the kernel in the bitmap
  m_bit_map[kernel_entries / 64] = ~(~(0ul) << (kernel_entries % 64));

  // Reserve the area for the bitmap
  allocate_area((uint64_t)m_bit_map, (m_bitmap_size / 8) + 1);

  // Reserve the area for the mmap
  for (multiboot_mmap_entry *entry = mmap->entries; (multiboot_uint8_t *)entry < (multiboot_uint8_t *)mmap + mmap->size; entry = (multiboot_mmap_entry *)((unsigned long)entry + mmap->entry_size)) {

    // Check if the entry is to be mapped
    if(entry->type <= 1)
      continue;

    allocate_area(entry->addr, entry->len);
  }

  // Log where the bitmap starts
  _kprintf("Bitmap starts at: 0x%x\n", m_bit_map);
  _kprintf("Bitmap size: %d\n", m_bitmap_size);
  _kprintf("Bitmap end: 0x%x\n", m_bit_map + m_bitmap_size / 8);
  _kprintf("Free memory: %dmb/%dmb (from 0x%x to 0x%x)\n", m_mmap->len / 1024 / 1024, memory_size / 1024 / 1024, m_mmap->addr, m_mmap->addr + m_mmap->len);

  return;

  // Allocate a new PML4 root
  m_pml4_root_address = (uint64_t*)allocate_frame();
  m_pml4_root = (pte_t*)m_pml4_root_address;

  // Map the first two pages for the kernel
  map_area((physical_address_t*)0, (virtual_address_t*)MemoryManager::s_higher_half_offset, PAGE_SIZE * 2, Present);

  // Map 4 GB of physical memory
  _kprintf("Mapping 4Gb Of io region...\n");
  map_area((physical_address_t*)0, (virtual_address_t*)MemoryManager::s_active_memory_manager -> to_io_region(0), 0xFFFFFFFF, Write);

  // Re-map the mmap
  for (multiboot_mmap_entry *entry = mmap->entries; (multiboot_uint8_t *)entry < (multiboot_uint8_t *)mmap + mmap->size; entry = (multiboot_mmap_entry *)((unsigned long)entry + mmap->entry_size)) {

      // Check this

      // 4GB Already mapped
      if(entry->addr >= 0xFFFFFFFF)
        continue;

      _kprintf("Mapping (io) mmap 0x%x - 0x%x\n", entry->addr, entry -> addr + entry->len);
      map_area((physical_address_t*)entry->addr, (virtual_address_t *)MemoryManager::s_active_memory_manager -> to_io_region(entry->addr), entry -> len, Write);
  }


  // Load the new PML4
  asm volatile("mov %0, %%cr3" :: "r"(m_pml4_root_address));

}


PhysicalMemoryManager::~PhysicalMemoryManager() {

}

size_t PhysicalMemoryManager::size_to_frames(size_t size) {
    return align_to_page(size) / PAGE_SIZE;
}


size_t PhysicalMemoryManager::align_to_page(size_t size) {
  return (size + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);
}

bool PhysicalMemoryManager::check_aligned(size_t size){
    return (size % PAGE_SIZE) == 0;
}

void* PhysicalMemoryManager::allocate_frame() {

  // Check if there are enough frames
  if(m_used_frames >= m_bitmap_size)
    return nullptr;

  // Loop through the bitmap
  for (uint16_t row = 0; row < m_total_entries; ++row) {
    for (uint16_t column = 0; column < ROW_BITS; ++column) {

        // Check if this frame is free
        if((m_bit_map[row] & (1 << column)))
          continue;

        // Mark the frame as used
        m_bit_map[row] |= (1 << column);
        m_used_frames++;

        // Return the address
        uint64_t frame_address = (row * ROW_BITS) + column;
        return (void*)(frame_address * PAGE_SIZE);
    }
  }

  // Error frame not found
  return nullptr;

}


void PhysicalMemoryManager::free_frame(void *address) {

    // Mark the frame as not used
    m_used_frames--;

    // Set the bit to 0
    uint64_t frame_address = (uint64_t)address / PAGE_SIZE;
    m_bit_map[frame_address / ROW_BITS] &= ~(1 << (frame_address % ROW_BITS));
}


void* PhysicalMemoryManager::allocate_area(uint64_t start_address, size_t size) {

  // Check how many frames are needed
  size_t frame_count = size_to_frames(size);

  // Store the information about the frames needed to be allocated for this size
  uint16_t start_row = 0;
  uint16_t start_column = 0;
  size_t adjacent_frames = 0;

  // Loop through the bitmap
  for (uint16_t row = 0; row < m_total_entries; ++row) {
    for (uint16_t column = 0; column < ROW_BITS; ++column) {

      // If this bit is not free reset the adjacent frames
      if((m_bit_map[row] & (1 << column))){
        adjacent_frames = 0;
        continue;
      }

      // Store the start of the area if it is not already stored
      if(adjacent_frames == 0){
        start_row = row;
        start_column = column;
      }

      // Increment the adjacent frames
      adjacent_frames++;

      // If enough frames are found we can allocate the area
      if(adjacent_frames == frame_count){

        // Mark the frames as used
        m_used_frames += frame_count;
        for (uint16_t i = 0; i < frame_count; ++i)
          m_bit_map[start_row + (start_column + i) / ROW_BITS] |= (1 << ((start_column + i) % ROW_BITS));

        // Return the address
        return (void*)(start_address + (start_row * ROW_BITS + start_column) * PAGE_SIZE);
      }
    }
  }

  // Error cant allocate that much
  return nullptr;
}

void PhysicalMemoryManager::free_area(uint64_t start_address, size_t size) {

    // Check how many frames are needed
    size_t frame_count = size_to_frames(size);
    uint64_t frame_address = start_address / PAGE_SIZE;

    // Check if the address is valid
    if(frame_address >= m_bitmap_size)
      return;

    // Mark the frames as not used
    m_used_frames -= frame_count;
    for (uint16_t i = 0; i < frame_count; ++i)
      m_bit_map[(frame_address + i) / ROW_BITS] &= ~(1 << ((frame_address + i) % ROW_BITS));

}

pml_t* PhysicalMemoryManager::get_or_create_table(pml_t* table, size_t index, size_t flags) {

    // Get the entry from the table
    pte_t entry = table->entries[index];

    // Check if the entry is present
    if(entry.present) {

      // Get the address of the table
      return (pml_t*)(entry.physical_address << 12);

    }

    _kprintf("Creating new table at index: %d\n", index);

    // Need to create a new table
    pml_t* new_table = (pml_t*)allocate_frame();

    // Clean the table
    clean_page_table((uint64_t*)new_table);

    // Set the entry
    table->entries[index] = create_page_table_entry((uintptr_t)new_table, flags);

    return new_table;


}


virtual_address_t* PhysicalMemoryManager::map(physical_address_t *physical, virtual_address_t *virtual_address, size_t flags) {

  // Get the page directory pointer m_pml4_root_address

  pml_t* pml3 = get_or_create_table((pml_t*)m_pml4_root_address, PML4_GET_INDEX(virtual_address), (flags | WriteBit));

  // Get the page directory
  pml_t* pml2 = get_or_create_table(pml3, PML3_GET_INDEX(virtual_address), (flags | WriteBit));

  // Get the page table
  pml_t* pml1 = get_or_create_table(pml2, PML2_GET_INDEX(virtual_address), (flags | WriteBit));

  // Get the entry
  pte_t* pte = &pml1->entries[PML1_GET_INDEX(virtual_address)];

  // Check if already mapped
  if(pte->present){
    _kprintf("ADDRESS ALREADY PRESENT");
    return virtual_address;
  }


  // Set the entry
  *pte = create_page_table_entry((uintptr_t)physical, flags);

  // Flush the TLB
  asm volatile("invlpg (%0)" ::"r" (virtual_address) : "memory");

//   _kprintf("Mapped: 0x%x to 0x%x\n", physical, virtual_address);



}

virtual_address_t* PhysicalMemoryManager::map(virtual_address_t *virtual_address, size_t flags) {

  // Create a new physical address for the frame
  physical_address_t* physical_address = (physical_address_t *)allocate_frame();

  // Map the physical address to the virtual address
  return map(physical_address, virtual_address, flags);

}

void PhysicalMemoryManager::map_area(virtual_address_t* virtual_address_start, size_t length, size_t flags) {

    // Get the size of the area
    size_t size = size_to_frames(length);

    // Map the required frames
    for (size_t i = 0; i < size; ++i)
        map(virtual_address_start + (i * PAGE_SIZE), flags);

}

void PhysicalMemoryManager::map_area(physical_address_t* physical_address_start, virtual_address_t* virtual_address_start, size_t length, size_t flags) {

  // Get the size of the area
  size_t size = size_to_frames(length);

  // Map the required frames
  for (size_t i = 0; i < size; ++i)
    map(physical_address_start + (i * PAGE_SIZE), virtual_address_start + (i * PAGE_SIZE), flags);

}

void PhysicalMemoryManager::identity_map(physical_address_t *physical_address, size_t flags) {

  // Map the physical address to its virtual address counter-part
  map(physical_address, physical_address, flags);

}

void PhysicalMemoryManager::unmap(virtual_address_t* virtual_address) {

  // TODO: Implement
}

/**
 * @brief Checks if a virtual address is mapped and whether it points to the correct physical address
 * @param physical_address The physical address to check (if 0 then wont check if correct entry)
 * @param virtual_address The address to check if it is mapped to
 * @return True if the physical address is mapped to the virtual address
 */
bool PhysicalMemoryManager::is_mapped(uintptr_t physical_address, uintptr_t virtual_address) {

  // TODO: Implement
}


void PhysicalMemoryManager::clean_page_table(uint64_t *table) {
  for(int i = 0; i < 512; i++){
        table[i] = 0x00l;
  }
}

void clearBits(uint64_t *num, int start, int end) {
  // Create a mask with 1s from start to end and 0s elsewhere
  uint64_t mask = (~0ULL << start) ^ (~0ULL << (end + 1));

  // Apply the mask to the number to clear the desired bits
  *num &= ~mask;
}

pte_t PhysicalMemoryManager::create_page_table_entry(uintptr_t address, size_t flags) {

  pte_t page =  (pte_t){
    .present = 1,
    .write = (flags & WriteBit) != 0,
    .user = (flags & UserBit) != 0,
    .write_through = (flags & (1 << 7)) != 0,
    .cache_disabled = 0,
    .accessed = 0,
    .dirty = 0,
    .huge_page = (flags & HugePageBit) != 0,
    .global = 0,
    .available = 0,
    .physical_address = (uint64_t)address >> 12
  };

  return page;
}
