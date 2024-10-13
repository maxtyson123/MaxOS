//
// Created by 98max on 2/11/2024.
//

#include <memory/virtual.h>
#include <common/kprint.h>

using namespace MaxOS::memory;
using namespace MaxOS::common;

VirtualMemoryManager::VirtualMemoryManager(bool is_kernel)
: m_physical_memory_manager(PhysicalMemoryManager::s_current_manager),
  m_is_kernel(is_kernel)
{

    // If not the kernel, we need to allocate a new PML4 table
    if(!m_is_kernel){

      // Get a new pml4 table
      m_pml4_root_physical_address = (uint64_t*)m_physical_memory_manager->allocate_frame();
      m_pml4_root_address = (uint64_t*)MemoryManager::to_dm_region((uint64_t)m_pml4_root_physical_address);
      _kprintf("Allocated new PML4 table at: 0x%x\n", m_pml4_root_address);

      // Clear the table
      m_physical_memory_manager -> clean_page_table(m_pml4_root_address);

      // Map the higher half of the kernel (p4 256 - 511)
      for (size_t i = 256; i < 512; i++){

        // Recursive Map the pml4 table (so that we can access the new pml4 table later on)
        if(i == 510) {
          m_pml4_root_address[i] = (uint64_t)m_pml4_root_physical_address | Present | Write;
          continue;
        }

        // Set the new pml4 table to the old (kernel) pml4 table
        m_pml4_root_address[i] = m_physical_memory_manager->get_pml4_root_address()[i];

      }
      _kprintf("Mapped higher half of kernel\n");


    }else{
      m_pml4_root_address = m_physical_memory_manager->get_pml4_root_address();
    };

    // Space to store VMM chunks
    uint64_t vmm_space = PhysicalMemoryManager::align_to_page(MemoryManager::s_hh_direct_map_offset + m_physical_memory_manager->get_memory_size() + PhysicalMemoryManager::s_page_size);
    m_first_region = (virtual_memory_region_t*)vmm_space;
    m_current_region = m_first_region;

    // Allocate space for the vmm
    void* vmm_space_physical = m_physical_memory_manager->allocate_frame();
    ASSERT(vmm_space_physical != nullptr, "Failed to allocate VMM space");
    m_physical_memory_manager->map(vmm_space_physical, (virtual_address_t*)vmm_space, Present | Write, m_pml4_root_address);
    m_first_region->next = nullptr;
    _kprintf("Allocated VMM: physical: 0x%x, virtual: 0x%x\n", vmm_space_physical, vmm_space);

    // Calculate the next available address
    m_next_available_address = PhysicalMemoryManager::s_page_size;
    if(m_is_kernel){

      // Kernel needs to start at the higher half
      m_next_available_address += vmm_space + s_reserved_space;

    }
    _kprintf("Next available address: 0x%x\n", m_next_available_address);


}

VirtualMemoryManager::~VirtualMemoryManager() {

}


/**
 * @brief Allocate a new chunk of virtual memory
 * @param size The size of the memory to allocate
 * @param flags The flags to set on the memory
 * @return The address of the allocated memory
 */
void* VirtualMemoryManager::allocate(size_t size, size_t flags) {
  return allocate(0, size, flags);
}


/**
 * @brief Allocate a new chunk of virtual memory at a specific address (ie for mmap io devices)
 * @param address The address to allocate at
 * @param size The size of the memory to allocate
 * @param flags The flags to set on the memory
 * @return The address of the allocated memory or nullptr if failed
 */
void *VirtualMemoryManager::allocate(uint64_t address, size_t size, size_t flags) {

  // Make sure allocating something
  if(size == 0)
    return nullptr;

  // If specific address is given
  if(address != 0){

      // Make sure isnt already allocated
      if(address < m_next_available_address)
        return nullptr;

      // Make sure its aligned
      if(!PhysicalMemoryManager::check_aligned(address))
        return nullptr;

  }

  // Make sure the size is aligned
  size = PhysicalMemoryManager::align_up_to_page(size, PhysicalMemoryManager::s_page_size);

  // Is there space in the current region
  if(m_current_chunk >= s_chunks_per_page)
    new_region();

  // If we need to allocate at a specific address
  if(address != 0){
    m_next_available_address = address;    //TODO: Creates mem fragmentation - fix
  }

  // Allocate the memory
  virtual_memory_chunk_t* chunk = &m_current_region->chunks[m_current_chunk];
  chunk->size = size;
  chunk->flags = flags;
  chunk->start_address = m_next_available_address;

  // Update the next available address
  m_next_available_address += size;
  m_current_chunk++;

  // If just reserving the space don't map it
  if(flags & Reserve)
    return (void*)chunk->start_address;

  // Map the memory
  size_t pages = PhysicalMemoryManager::size_to_frames(size);
  for (size_t i = 0; i < pages; i++){

    // Allocate a new frame
    physical_address_t* frame = m_physical_memory_manager->allocate_frame();
    ASSERT(frame != nullptr, "Failed to allocate frame");

    // Map the frame
    m_physical_memory_manager->map(frame, (virtual_address_t*)chunk->start_address + (i * PhysicalMemoryManager::s_page_size), Present | Write, m_pml4_root_address);

  }

  // Return the address
  return (void*)chunk->start_address;
}

/**
 * @brief Create a mew region in the VMM to use for allocation of more chunks
 */
void VirtualMemoryManager::new_region() {

  // Space for the new region
  physical_address_t* new_region_physical = m_physical_memory_manager->allocate_frame();
  ASSERT(new_region_physical != nullptr, "Failed to allocate new VMM region");

  // Align the new region
  virtual_memory_region_t* new_region = (virtual_memory_region_t*)PhysicalMemoryManager::align_to_page((uint64_t)m_current_region + PhysicalMemoryManager::s_page_size);

  // Map the new region
  m_physical_memory_manager->map(new_region_physical, (virtual_address_t*)new_region, Present | Write, m_pml4_root_address);
  new_region->next = nullptr;

  // Set the current region
  m_current_region -> next = new_region;
  m_current_chunk = 0;
  m_current_region = new_region;

}

/**
 * @brief Free a chunk of virtual memory
 * @param address The address of the memory to free
 */
void VirtualMemoryManager::free(void *address) {

  // Make sure freeing something
  if(address == nullptr)
    return;

  // Find the chunk
  virtual_memory_region_t* region = m_first_region;
  virtual_memory_chunk_t* chunk = nullptr;
  while(region != nullptr){

      // Loop through the chunks
      for (size_t i = 0; i < s_chunks_per_page; i++){

        // Check if the address is in the chunk
        if(region->chunks[i].start_address == (uintptr_t)address){
              chunk = &region->chunks[i];
              break;
        }
      }

      // If the chunk was found
      if(chunk != nullptr)
        break;

      // Move to the next region
      region = region->next;
  }

  // Make sure the chunk was found
  if(chunk == nullptr)
    return;

  // Unmap the memory
  size_t pages = PhysicalMemoryManager::size_to_frames(chunk->size);
  for (size_t i = 0; i < pages; i++){

        // Unmap the frame
        m_physical_memory_manager->unmap((virtual_address_t*)chunk->start_address + (i * PhysicalMemoryManager::s_page_size), m_pml4_root_address);

  }

  // Clear the chunk
  chunk->size = 0;
  chunk->flags = 0;
  chunk->start_address = 0;

  // TODO: Some logic to use this space again
}

/**
 * @brief Returns the amount of memory used
 * @return The amount of memory used
 */
size_t VirtualMemoryManager::memory_used() {

  // Loop through all the regions and add up the size of the allocated chunks
  size_t result = 0;

  // Iterate through the regions
  virtual_memory_region_t *region = m_first_region;
  while (region != nullptr) {

    // Loop through the chunks
    for (size_t i = 0; i < s_chunks_per_page; i++) {

      // Check if the address is in the chunk
      if (region->chunks[i].size != 0)
        result += region->chunks[i].size;
    }

    // Move to the next region
    region = region->next;
  }

  return result;
}
