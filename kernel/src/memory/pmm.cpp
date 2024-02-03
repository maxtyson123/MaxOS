//
// Created by 98max on 1/30/2024.
//

#include <memory/pmm.h>
#include <common/kprint.h>

MaxOS::memory::PhysicalMemoryManager::PhysicalMemoryManager(unsigned long reserved, multiboot_tag_basic_meminfo* meminfo){

  // Store the information about the bitmap
  uint64_t memory_size = (meminfo -> mem_upper + 1024) * 1024;
  m_total_entries = (memory_size / PAGE_SIZE);

  _kprintf("Found Memory with size of %d bytes\n", memory_size);
  _kprintf("Total Pages: %d \n", m_total_entries);

  // Map Kernel
  // Reserve the area
}
