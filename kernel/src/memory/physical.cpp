//
// Created by 98max on 1/30/2024.
//

#include <common/kprint.h>
#include <memory/physical.h>

using namespace MaxOS::memory;
using namespace MaxOS::system;
extern uint64_t p4_table[];

MaxOS::memory::PhysicalMemoryManager::PhysicalMemoryManager(unsigned long reserved, Multiboot* multiboot){

  // SEE boot.s FOR SETUP OF PAGING

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

  // Set up the PML4
  m_pml4_root_address = (uint64_t *)(PAGE_TABLE_OFFSET | get_table_address(510,510,510,510));

  // Mapping information
  uintptr_t base_map_address = (uint64_t)MemoryManager::s_higher_half_offset + PAGE_SIZE; //TODO: Maybe PAGE_SIZE should be multiplied by kernel_entries to get the correct padding?
  uint64_t physical_address = 0;
  uint64_t virtual_address = base_map_address;

  // Check if the paging is working
  size_t base_page_entry = get_pml4_index(base_map_address);
  if((p4_table[base_page_entry] & 1) == 0)
    _kprintf("ERROR: Page Table not set up");

  // Map all the physical memory into the virtual address space
  while(physical_address < memory_size){
    _kprintf("Mapping: 0x%x to 0x%x\n", physical_address, virtual_address);
    map((physical_address_t *)physical_address, (virtual_address_t *)virtual_address, Present | Write);

    // Move to the next page
    physical_address += PAGE_SIZE;
    virtual_address += PAGE_SIZE;
  }

  //TODO: Note when the kernel VMM is set up paging should start at base_map_address + memory_size
}

PhysicalMemoryManager::~PhysicalMemoryManager() {

}

uint16_t PhysicalMemoryManager::get_pml4_index(uintptr_t virtual_address) {
  // The PML4 index is the last byte
  return (uint16_t)((uint64_t)virtual_address >> 39 & 0x1FF);
}

uint16_t PhysicalMemoryManager::get_page_directory_index(uintptr_t virtual_address) {
  // The PDP index is the 3rd byte
  return (uint16_t)((uint64_t)virtual_address >> 30 & 0x1FF);
}
uint16_t PhysicalMemoryManager::get_page_table_index(uintptr_t virtual_address) {
  // The PD index is the 2nd byte
  return (uint16_t)((uint64_t)virtual_address >> 21 & 0x1FF);
}
uint16_t PhysicalMemoryManager::get_page_index(uintptr_t virtual_address) {
  // The PT index is the 1st byte (starting from 12)
  return (uint16_t)((uint64_t)virtual_address >> 12 & 0x1FF);
}

uint64_t PhysicalMemoryManager::get_table_address(uint16_t pml4_index, uint16_t pdp_index, uint16_t page_table_index, uint16_t page_index) {
    return (pml4_index << 39) | (pdp_index << 30) | (page_table_index << 21) | (page_index << 12);
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

void clean_new_table( uint64_t *table_to_clean ) {
  for(int i = 0; i < 512; i++){
    table_to_clean[i] = 0x00l;
  }
}

virtual_address_t* PhysicalMemoryManager::map(physical_address_t *physical, virtual_address_t *virtual_address, size_t flags) {

  // Check if the address is already mapped
  if(is_mapped((uintptr_t)physical, (uintptr_t)virtual_address))
      return virtual_address;

  // Get the indexes of the address
  uint16_t pml4_index = get_pml4_index((uintptr_t)virtual_address);
  uint16_t page_directory_index = get_page_directory_index((uintptr_t)virtual_address);
  uint16_t page_table_index = get_page_table_index((uintptr_t)virtual_address);

  // Get the table address
  uint64_t* page_directory_pointer = (uint64_t *)(PAGE_TABLE_OFFSET | get_table_address(510, 510, 510 , pml4_index));
  uint64_t* page_directory = (uint64_t *)(PAGE_TABLE_OFFSET | get_table_address(510, 510, (uint16_t)pml4_index, (uint16_t)page_directory_index));

  // Check if the page directory for this address is present
  if(!(m_pml4_root_address[pml4_index] & 1)){
    // Allocate a new page directory
    uint64_t* new_table = (uint64_t*)allocate_frame();
    m_pml4_root_address[pml4_index] = (uint64_t) new_table | WriteBit | PresentBit;

    // Clear this new address
    clean_new_table(page_directory_pointer);

    _kprintf("Allocated new page directory pointer (%d) at: 0x%x, ", pml4_index, &m_pml4_root_address[pml4_index]);
  }

  // Check if the page directory for this address is present
  if(!(page_directory_pointer[page_directory_index] & 1)){

    // Allocate a new page table
    uint64_t* new_table = (uint64_t*)allocate_frame();
    page_directory_pointer[page_directory_index] = (uint64_t) new_table | WriteBit | PresentBit;

    // The page directory is not present, so we need to clear it
    clean_new_table(page_directory);

    _kprintf("Allocated new page directory (%s)  at: 0x%x, ", page_directory_index, &page_directory_pointer[page_directory_index]);
  }

  // Set the page entry in the page table
  page_directory[page_table_index] = (uint64_t) (physical) | HugePageBit | flags;
  _kprintf("Mapped: 0x%x to 0x%x\n", (uint64_t)physical, (uint64_t)virtual_address);
  return virtual_address;

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

void PhysicalMemoryManager::identity_map(physical_address_t *physical_address, size_t flags) {

  // Map the physical address to its virtual address counter-part
  map(physical_address, physical_address, flags);

}

void PhysicalMemoryManager::unmap(virtual_address_t* virtual_address) {

  //TODO: TEST WORKS

  // Check if the address is mapped
  if(!is_mapped(0, (uintptr_t)virtual_address))
    return;

  // Get the indexes of the address
  uint16_t page_directory_pointer = get_pml4_index((uintptr_t)virtual_address);
  uint16_t page_directory_index = get_page_directory_index((uintptr_t)virtual_address);
  uint16_t page_table_index = get_page_table_index((uintptr_t)virtual_address);

  // Set the page entry in the page directory to 0
  uint64_t* page_directory = (uint64_t*)(PAGE_TABLE_OFFSET | get_table_address(510l, 510, (uint64_t) page_directory_pointer, (uint64_t) page_directory_index));
  page_directory[page_table_index] = 0;

  // Invalidate the TLB
  asm volatile("invlpg (%0)" ::"r" ((uint64_t)virtual_address) : "memory");
}

/**
 * @brief Checks if a virtual address is mapped and whether it points to the correct physical address
 * @param physical_address The physical address to check (if 0 then wont check if correct entry)
 * @param virtual_address The address to check if it is mapped to
 * @return True if the physical address is mapped to the virtual address
 */
bool PhysicalMemoryManager::is_mapped(uintptr_t physical_address, uintptr_t virtual_address) {

  //TODO: Test works

  // Get the indexes of the address
  uint16_t pml4_index = get_pml4_index(virtual_address);
  uint16_t pdp_index = get_page_directory_index(virtual_address);
  uint16_t page_table_index = get_page_table_index(virtual_address);

  // Check if there is a correct entry in the PML4
  if((m_pml4_root_address[pml4_index] & 1) == 0)
      return false;

  // Get the address of the pointer to the page directory and check if it is valid
  uint64_t* pdp_address = (uint64_t *)(PAGE_TABLE_OFFSET | get_table_address(510, 510, 510 , pdp_index));
  if((pdp_address[pdp_index] & 1) == 0)
      return false;

  // Get the address to the page table and check if it is valid
  uint64_t* pd_address = (uint64_t *)(PAGE_TABLE_OFFSET | get_table_address(510, 510, pdp_index, page_table_index));
  if((pd_address[page_table_index] & 1) == 0)
     return false;

  // If the physical address is a nullpointer then don't bother checking if it is correct
  if(physical_address == 0)
    return true;

  // Check if the physical address is the same as the one in the page table
  return align_to_page((size_t)physical_address) == align_to_page(pd_address[page_table_index]);
}