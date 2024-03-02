//
// Created by 98max on 2/11/2024.
//

#include <memory/virtual.h>

using namespace MaxOS::memory;

VirtualMemoryManager::VirtualMemoryManager(PhysicalMemoryManager* physical_memory_manager)
: m_physical_memory_manager(physical_memory_manager)
{

  //NOTE: See boot.s for the set up of paging

  // Map the physical memory to the higher half virtual memory

  // TODO: Virtual Memory Management (not just virtual addresses & physical addresses)

}

VirtualMemoryManager::~VirtualMemoryManager() {

}


