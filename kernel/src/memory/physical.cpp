//
// Created by 98max on 1/30/2024.
//

#include <common/kprint.h>
#include <memory/physical.h>

using namespace MaxOS::memory;
using namespace MaxOS::system;

MaxOS::memory::PhysicalMemoryManager::PhysicalMemoryManager(unsigned long reserved, Multiboot* multiboot){

  // Store the information about the bitmap
  uint64_t memory_size = (multiboot->get_basic_meminfo() -> mem_upper + 1024) * 1024;
  m_bitmap_size = memory_size / PAGE_SIZE;
  m_total_entries = m_bitmap_size / ROW_BITS;

  _kprintf("Found Memory with size of %d bytes\n", memory_size);
  _kprintf("Bitmap Size: %d\n", m_bitmap_size);
  _kprintf("Total Entries: %d \n", m_total_entries);

  // Loop through all the memory map entries
  multiboot_tag_mmap* mmap = multiboot->get_mmap();
  for (multiboot_mmap_entry *entry = mmap->entries; (multiboot_uint8_t *)entry < (multiboot_uint8_t *)mmap + mmap->size; entry = (multiboot_mmap_entry *)((unsigned long)entry + mmap->entry_size)) {

    // Skip if the region is not free or there is not enough space
    if (entry->type != MULTIBOOT_MEMORY_AVAILABLE ||
        (entry->addr + entry->len) < reserved)
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
}

PhysicalMemoryManager::~PhysicalMemoryManager() {

}

size_t PhysicalMemoryManager::size_to_frames(size_t size) const {
    return align_to_page(size) / PAGE_SIZE;
}


size_t PhysicalMemoryManager::align_to_page(size_t size) const {
  return (size + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);
}

bool PhysicalMemoryManager::check_aligned(size_t size) const {
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
