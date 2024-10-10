//
// Created by 98max on 1/30/2024.
//

#include <common/kprint.h>
#include <memory/physical.h>

using namespace MaxOS::memory;
using namespace MaxOS::system;
extern uint64_t p4_table[];

PhysicalMemoryManager* PhysicalMemoryManager::current_manager = nullptr;
extern uint64_t _kernel_end;
extern uint64_t _kernel_size;
extern uint64_t _kernel_physical_end;
extern uint64_t multiboot_tag_end;
extern uint64_t multiboot_tag_start;

MaxOS::memory::PhysicalMemoryManager::PhysicalMemoryManager(unsigned long reserved, Multiboot* multiboot, uint64_t pml4_root[512]) {

  // SEE boot.s FOR SETUP OF PAGING
  m_pml4_root = (pte_t *)pml4_root;
  m_pml4_root_address = pml4_root;

  // Set the current manager
  current_manager = this;

  // Store the information about the bitmap
  m_memory_size = multiboot->get_basic_meminfo()->mem_upper + 1024;
  m_bitmap_size = m_memory_size / PAGE_SIZE + 1;
  m_total_entries = m_bitmap_size / ROW_BITS + 1;
  _kprintf("Mem Info: size = %dmb, bitmap size = %d, total entries = %d, page size = %db\n", (m_memory_size * 1024) / 1024 / 1024, m_bitmap_size, m_total_entries, PAGE_SIZE);

  // Get the mmap that stores the memory to use
  m_mmap_tag = multiboot->get_mmap();
  for (multiboot_mmap_entry *entry = m_mmap_tag->entries; (multiboot_uint8_t *)entry < (multiboot_uint8_t *)m_mmap_tag + m_mmap_tag->size; entry = (multiboot_mmap_entry *)((unsigned long)entry + m_mmap_tag->entry_size)) {

    // Skip if the region is not free or there is not enough space
    if (entry->type != MULTIBOOT_MEMORY_AVAILABLE || (entry->addr + entry->len) < reserved)
      continue;

    // We want the last entry
    m_mmap = entry;
  }
  _kprintf("Mmap in use: 0x%x - 0x%x\n", m_mmap->addr, m_mmap->addr + m_mmap->len);

  // Kernel Memory (anonymous memory to the next page)
  _kprintf("Kernel Memory: kernel_end = 0x%x, kernel_size = 0x%x, kernel_physical_end = 0x%x\n", &_kernel_end, &_kernel_size, &_kernel_physical_end);
  m_anonymous_memory_physical_address = (uint64_t)align_up_to_page((size_t)(&_kernel_physical_end + PAGE_SIZE), PAGE_SIZE);
  m_anonymous_memory_virtual_address = (uint64_t)align_up_to_page((size_t)(&_kernel_end + PAGE_SIZE), PAGE_SIZE);
  _kprintf("Anonymous Memory: physical = 0x%x, virtual = 0x%x\n", m_anonymous_memory_physical_address, m_anonymous_memory_virtual_address);

  // Map the physical memory into the virtual memory
  uint64_t physical_address = 0;
  uint64_t virtual_address = MemoryManager::s_hh_direct_map_offset;
  uint64_t mem_end = m_mmap->addr + m_mmap->len;

  while (physical_address < mem_end) {
    map((physical_address_t *)physical_address, (virtual_address_t *)virtual_address, Present | Write);
    physical_address += PAGE_SIZE;
    virtual_address += PAGE_SIZE;
  }
  _kprintf("Mapped: physical = 0x%x-0x%x, virtual = 0x%x-0x%x\n", 0, physical_address, MemoryManager::s_hh_direct_map_offset, virtual_address); // TODO: FAILS WHEN TRYING WITH LIKE 2Gb Mem

  // Get the bitmap & clear it
  m_anonymous_memory_physical_address += PAGE_SIZE;
  m_bit_map = get_bitmap_address();

  for (uint32_t i = 0; i < m_total_entries; ++i)
    m_bit_map[i] = 0;
  _kprintf("Bitmap: location = 0x%x - 0x%x\n", m_bit_map, m_bit_map + m_bitmap_size / 8);

  // Calculate how much space the kernel takes up
  uint32_t kernel_entries = (m_anonymous_memory_physical_address / PAGE_SIZE) + 1;
  if ((((uint32_t)(m_anonymous_memory_physical_address)) % PAGE_SIZE) != 0) {
    // If the kernel takes up more then a whole page(s)
    kernel_entries += 1;
  }

  // Reserve the kernel in the bitmap
  uint32_t kernel_rows = kernel_entries / ROW_BITS;
  for (uint32_t i = 0; i < kernel_rows; ++i)
      m_bit_map[i] = 0xFFFFFFFF;

  // Change the final row to account for the remaining bits
  m_bit_map[kernel_rows] = ~(0ul) << (kernel_entries - (kernel_rows * 64));
  m_used_frames = kernel_entries;
  _kprintf("Kernel: entries = %d, rows = %d, used = %d\n", kernel_entries, kernel_rows, m_used_frames);

  // Reserve the area for the bitmap
  allocate_area((uint64_t)MemoryManager::to_lower_region((uint64_t)m_bit_map), m_bitmap_size / 8 + 1);

  // Reserve the area for the mmap
  for (multiboot_mmap_entry *entry = m_mmap_tag->entries; (multiboot_uint8_t *)entry < (multiboot_uint8_t *)m_mmap_tag + m_mmap_tag->size; entry = (multiboot_mmap_entry *)((unsigned long)entry + m_mmap_tag->entry_size)) {

    // Check if the entry is to be mapped
    if (entry->type <= 1)
      continue;

    // Where our free mem starts
    if(entry->addr >= mem_end)
      continue;

    // Reserve the area

    allocate_area(entry->addr, entry->len);
    _kprintf("Mmap Reserved: 0x%x - 0x%x\n", entry->addr, entry->addr + entry->len);
  }

  // Initialisation Done
  m_initialized = true;
}

PhysicalMemoryManager::~PhysicalMemoryManager() {

}

size_t PhysicalMemoryManager::size_to_frames(size_t size) {
    return align_to_page(size) / PAGE_SIZE;
}


size_t PhysicalMemoryManager::align_to_page(size_t size) {
  return (size + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);
}

size_t PhysicalMemoryManager::align_up_to_page(size_t size, size_t page_size) {
  return (size + page_size - 1) & ~(page_size - 1);
}


bool PhysicalMemoryManager::check_aligned(size_t size){
    return (size % PAGE_SIZE) == 0;
}

void* PhysicalMemoryManager::allocate_frame() {

  // Check if the pmm is initialized
  if(!m_initialized){

    // Find the first free frame
    while ((!is_anonymous_available(m_anonymous_memory_physical_address)) && (m_anonymous_memory_physical_address < m_memory_size)) {
      m_anonymous_memory_physical_address += PAGE_SIZE;
      m_anonymous_memory_virtual_address += PAGE_SIZE;
    }

    // Mark frame as used
    m_anonymous_memory_physical_address += PAGE_SIZE;
    m_anonymous_memory_virtual_address += PAGE_SIZE;

    // Return the address
    return (void*)(m_anonymous_memory_physical_address - PAGE_SIZE);

  }

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



virtual_address_t* PhysicalMemoryManager::map(physical_address_t *physical_address, virtual_address_t* address, size_t flags) {
#define SIGN_EXTENSION 0xFFFF000000000000
#define ENTRIES_TO_ADDRESS(pml4, pdpr, pd, pt)((pml4 << 39) | (pdpr << 30) | (pd << 21) |  (pt << 12))

  uint64_t* pml4_table = m_pml4_root_address;

  uint16_t pml4_e = PML4_GET_INDEX((uint64_t) address);
  uint16_t pdpr_e = PML3_GET_INDEX((uint64_t) address);
  uint16_t pd_e   = PML2_GET_INDEX((uint64_t) address);
  uint16_t pt_e   = PML1_GET_INDEX((uint64_t) address);

  uint64_t *pdpr_table =(uint64_t *) (SIGN_EXTENSION | ENTRIES_TO_ADDRESS(510l,510l,510l, (uint64_t) pml4_e));
  uint64_t *pd_table = (uint64_t *) (SIGN_EXTENSION | ENTRIES_TO_ADDRESS(510l,510l, (uint64_t) pml4_e, (uint64_t) pdpr_e));
  uint64_t *pt_table = (uint64_t *) (SIGN_EXTENSION | ENTRIES_TO_ADDRESS(510l, (uint64_t) pml4_e, (uint64_t) pdpr_e, (uint64_t) pd_e));


 if( !(pml4_table[pml4_e] & 0b1) ) {
    uint64_t *new_table = (uint64_t *)allocate_frame();
    pml4_table[pml4_e] = (uint64_t) new_table | Present | Write;
    clean_page_table(pdpr_table);
  }

  if( !(pdpr_table[pdpr_e] & 0b1) ) {
    uint64_t *new_table = (uint64_t *)allocate_frame();
    pdpr_table[pdpr_e] = (uint64_t) new_table | Present | Write;
    clean_page_table(pd_table);
  }

  if( !(pd_table[pd_e] & 0b01) ) {
    uint64_t *new_table = (uint64_t *)allocate_frame();
    pd_table[pd_e] = (uint64_t) new_table | Present | Write;
    clean_page_table(pt_table);
  }

   if( !(pt_table[pt_e] & 0b1)) {
    pt_table[pt_e] = (uint64_t) physical_address | flags;
  }

  // Flush the TLB
  asm volatile("invlpg (%0)" ::"r" (address) : "memory");

  return address;
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
  ASSERT(false, "Not implemented!")
  // TODO: Implement
}

/**
 * @brief Checks if a virtual address is mapped and whether it points to the correct physical address
 * @param physical_address The physical address to check (if 0 then wont check if correct entry)
 * @param virtual_address The address to check if it is mapped to
 * @return True if the physical address is mapped to the virtual address
 */
bool PhysicalMemoryManager::is_mapped(uintptr_t physical_address, uintptr_t virtual_address) {
  ASSERT(false, "Not implemented!")
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

bool PhysicalMemoryManager::is_anonymous_available(size_t address) {

  // Return false if the address range is entirely within or overlaps with the multiboot reserved region
  if ((address > multiboot_tag_start && address + PAGE_SIZE < multiboot_tag_end) || (address + PAGE_SIZE > multiboot_tag_start && address < multiboot_tag_end)) {
    return false;
  }

  // Loop through the mmmap entries
  for (multiboot_mmap_entry *entry = m_mmap_tag->entries; (multiboot_uint8_t *)entry < (multiboot_uint8_t *)m_mmap_tag + m_mmap_tag->size; entry = (multiboot_mmap_entry *)((unsigned long)entry + m_mmap_tag->entry_size)) {

    // If it doesn't overlap with the mmap entry
    if ((entry -> addr + entry -> len) < (address + PAGE_SIZE))
      continue;

    // If it is not available
    if(entry -> type != MULTIBOOT_MEMORY_AVAILABLE)
      continue;

    // Check if the address is reserved by the multiboot module
    if(is_multiboot_reserved(address))
        continue;

    // Memory is available
    return true;

  }

  // Memory is not available
  return false;
}
bool PhysicalMemoryManager::is_multiboot_reserved(uint64_t address) {
  //ASSERT(false, "Not implemented!")
  // TODO: Check if address is reserve by multiboot module

  return false;
}
uint64_t *PhysicalMemoryManager::get_bitmap_address() {


  // Earliest address to place the bitmap (after the kernel and hh direct map)
  uint64_t limit = m_anonymous_memory_physical_address;

  // Loop through the mmap entries
  for (multiboot_mmap_entry *entry = m_mmap_tag->entries; (multiboot_uint8_t *)entry < (multiboot_uint8_t *)m_mmap_tag + m_mmap_tag->size; entry = (multiboot_mmap_entry *)((unsigned long)entry + m_mmap_tag->entry_size)) {

        // If the entry is not available or the address is before the limit
        if (entry -> type != MULTIBOOT_MEMORY_AVAILABLE || entry -> len > limit)
          continue;

        size_t space = entry -> len;
        size_t offset = 0;

        // If the entry starts before the limit then adjust the space to start where the limit is
        if(entry -> addr < limit){
            offset = limit - entry -> addr;
            space -= offset;
        }


        // Make sure there is enough space
        ASSERT(space >= (m_bitmap_size / 8 + 1), "Not enough space for the bitmap");

        // Return the address
        return (uint64_t*)MemoryManager::to_dm_region(entry -> addr + offset);
    }

  // Error no space for the bitmap
  ASSERT(false, "No space for the bitmap");
}
