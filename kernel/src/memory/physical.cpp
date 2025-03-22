//
// Created by 98max on 1/30/2024.
//

#include <common/kprint.h>
#include <memory/physical.h>

using namespace MaxOS::memory;
using namespace MaxOS::system;
using namespace MaxOS::common;
extern uint64_t p4_table[];

PhysicalMemoryManager* PhysicalMemoryManager::s_current_manager = nullptr;
extern uint64_t _kernel_start;
extern uint64_t _kernel_end;
extern uint64_t _kernel_size;
extern uint64_t _kernel_physical_end;
extern uint64_t multiboot_tag_end;
extern uint64_t multiboot_tag_start;


MaxOS::memory::PhysicalMemoryManager::PhysicalMemoryManager(unsigned long reserved, Multiboot* multiboot, uint64_t pml4_root[512])
: m_kernel_end((uint64_t)&_kernel_physical_end),
  m_multiboot(multiboot),
  m_pml4_root_address(pml4_root),
  m_pml4_root((pte_t *)pml4_root)
{

  // Clear the spinlock
  m_lock = Spinlock();
  m_lock.unlock();

  // Set the current manager
  s_current_manager = this;

  // SEE boot.s FOR SETUP OF PAGING
  m_pml4_root = (pte_t *)pml4_root;
  m_pml4_root_address = pml4_root;
  _kprintf("PML4: 0x%x\n", m_pml4_root);

  // Store the information about the bitmap
  m_memory_size = (m_multiboot->get_basic_meminfo()->mem_upper + 1024) * 1024;
  m_bitmap_size = m_memory_size / s_page_size + 1;
  m_total_entries = m_bitmap_size / ROW_BITS + 1;
  _kprintf("Mem Info: size = %dmb, bitmap size = %d, total entries = %d, page size = %db\n", ((m_memory_size / 1000) * 1024) / 1024 / 1024, m_bitmap_size, m_total_entries, s_page_size);

  // Get the mmap that stores the memory to use
  m_mmap_tag = m_multiboot->get_mmap();
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
  m_anonymous_memory_physical_address = (uint64_t)align_up_to_page((size_t)&_kernel_physical_end + s_page_size, s_page_size);
  m_anonymous_memory_virtual_address  = (uint64_t)align_up_to_page((size_t)&_kernel_end + s_page_size, s_page_size);
  _kprintf("Anonymous Memory: physical = 0x%x, virtual = 0x%x\n", m_anonymous_memory_physical_address, m_anonymous_memory_virtual_address);

  // Map the physical memory into the virtual memory
  uint64_t physical_address = 0;
  uint64_t virtual_address = MemoryManager::s_hh_direct_map_offset;
  uint64_t mem_end = m_mmap->addr + m_mmap->len;

  while (physical_address < mem_end) {
    map((physical_address_t *)physical_address, (virtual_address_t *)virtual_address, Present | Write);
    physical_address += s_page_size;
    virtual_address += s_page_size;
  }
  _kprintf("Mapped: physical = 0x%x-0x%x, virtual = 0x%x-0x%x\n", 0, physical_address, MemoryManager::s_hh_direct_map_offset, virtual_address); // TODO: FAILS WHEN TRYING WITH LIKE 2Gb Mem

  // Calculate the bitmap address
  m_anonymous_memory_physical_address += s_page_size;
  m_bit_map = get_bitmap_address();

  // Clear the bitmap
  for (uint32_t i = 0; i < m_total_entries; ++i)
    m_bit_map[i] = 0;

  // Reserve the area for the bitmap
  _kprintf("Bitmap: location = 0x%x - 0x%x (range of 0x%x)\n", m_bit_map, m_bit_map + m_bitmap_size / 8, m_bitmap_size / 8);
  reserve((uint64_t)MemoryManager::from_dm_region((uint64_t)m_bit_map), m_bitmap_size / 8 );


  // Calculate how much space the kernel takes up
  uint32_t kernel_entries = (m_anonymous_memory_physical_address / s_page_size) + 1;
  if ((((uint32_t)(m_anonymous_memory_physical_address)) % s_page_size) != 0) {
    // If the kernel takes up more then a whole page(s)
    kernel_entries += 1;
  }

  // Reserve the kernel in the bitmap
  _kprintf("Kernel: location = 0x%x - 0x%x (range of 0x%x)\n", 0, m_anonymous_memory_physical_address, kernel_entries * s_page_size);
  reserve(0, kernel_entries * s_page_size);


  // Reserve the area for the mmap
  for (multiboot_mmap_entry *entry = m_mmap_tag->entries; (multiboot_uint8_t *)entry < (multiboot_uint8_t *)m_mmap_tag + m_mmap_tag->size; entry = (multiboot_mmap_entry *)((unsigned long)entry + m_mmap_tag->entry_size)) {

    // Check if the entry is to be mapped
    if (entry->type <= MULTIBOOT_MEMORY_AVAILABLE)
      continue;

    // Where the free mem starts
    if(entry->addr >= mem_end)
      continue;

    // Reserve the area
    reserve(entry->addr, entry->len);
  }

  // Reserve the area for each multiboot module
  for(multiboot_tag* tag = multiboot -> get_start_tag(); tag->type != MULTIBOOT_TAG_TYPE_END; tag = (struct multiboot_tag *) ((multiboot_uint8_t *) tag + ((tag->size + 7) & ~7))) {

    // Check if the tag is a module
    if(tag -> type != MULTIBOOT_TAG_TYPE_MODULE)
      continue;

    // Get the module tag
    struct multiboot_tag_module* module = (struct multiboot_tag_module*)tag;

    // Reserve the address
    reserve(module->mod_start, module->mod_end - module->mod_start);
  }

  // Initialisation Done
  m_initialized = true;
}

PhysicalMemoryManager::~PhysicalMemoryManager() {

}

/**
 * @brief Converts a size to the number of frames
 * @param size The size to convert
 * @return The number of frames
 */
size_t PhysicalMemoryManager::size_to_frames(size_t size) {
    return align_to_page(size) / s_page_size;
}

/**
 * @brief Aligns a size to the page size
 * @param size The size to align
 * @return  The aligned size
 */
size_t PhysicalMemoryManager::align_to_page(size_t size) {
  return ((size + s_page_size - 1) /s_page_size) * s_page_size;
}

/**
 * @brief Aligns a size up to the page size
 * @param size  The size to align
 * @param page_size The page size to align to
 * @return The aligned size
 */
size_t PhysicalMemoryManager::align_up_to_page(size_t size, size_t page_size) {
  return (size + page_size - 1) & ~(page_size - 1);
}

/**
 * @brief Checks if an address is aligned
 * @param size The address to check
 * @return True if the address is aligned
 */
bool PhysicalMemoryManager::check_aligned(size_t size){
    return (size % s_page_size) == 0;
}

/**
 * @brief Allocates a physical page of memory, if the PMM is not initalise it will use the anon memory instead of the bitmap
 * @return The physical address of the page
 */
void* PhysicalMemoryManager::allocate_frame() {

  // Wait for the lock
  m_lock.lock();

  // Check if the pmm is initialized
  if(!m_initialized){

    // Find the first free frame
    while ((!is_anonymous_available(m_anonymous_memory_physical_address)) && (m_anonymous_memory_physical_address < m_memory_size)) {
      m_anonymous_memory_physical_address += s_page_size;
      m_anonymous_memory_virtual_address += s_page_size;
    }

    // Mark frame as used
    m_anonymous_memory_physical_address += s_page_size;
    m_anonymous_memory_virtual_address += s_page_size;

    // Clear the lock
    m_lock.unlock();

    // Return the address
    return (void*)(m_anonymous_memory_physical_address - s_page_size);

  }

  // Check if there are enough frames
  ASSERT(m_used_frames < m_bitmap_size, "No more frames available\n")

  // Loop through the bitmap
  for (uint16_t row = 0; row < m_total_entries; ++row) {

    // If the row is full continue
    if(m_bit_map[row] == 0xFFFFFFFFFFFFFFF)
      continue;

    for (uint16_t column = 0; column < ROW_BITS; ++column) {

      // Prevent out-of-bounds shifts if column exceeds the bit-width of uint64_t
      if (column >= ROW_BITS)
        break;

      // Check if the bitmap is free
      if (m_bit_map[row] & (1ULL << column))
        continue;


      // Mark the frame as used
      m_bit_map[row] |= (1ULL << column);
      m_used_frames++;

      // Return the address
      uint64_t frame_address = (row * ROW_BITS) + column;
      frame_address *= s_page_size;


      // Clear the lock
      m_lock.unlock();

      // Return the address
      return (void*)(frame_address);
    }
  }

  // Error frame not found
  ASSERT(false, "Frame not found\n")
  m_lock.unlock();
  return nullptr;

}

/**
 * @brief Frees a frame in the bit map
 * @param address The address to free
 */
void PhysicalMemoryManager::free_frame(void *address) {

    // Wait for the lock
    m_lock.lock();

    // Mark the frame as not used
    m_used_frames--;

    // Set the bit to 0
    uint64_t frame_address = (uint64_t)address / s_page_size;
    m_bit_map[frame_address / ROW_BITS] &= ~(1 << (frame_address % ROW_BITS));

    // Clear the lock
    m_lock.unlock();
}

/**
 * @brief Allocate an area of physical memory (ie reserve it)
 * @param start_address The start of the block
 * @param size The size to allocate
 * @return A pointer to the start of the block (physical address)
 */
void* PhysicalMemoryManager::allocate_area(uint64_t start_address, size_t size) {

  // Wait to be able to allocate
  m_lock.lock();

  // Check how many frames are needed
  size_t frame_count = size_to_frames(size);

  // Store the information about the frames needed to be allocated for this size
  uint16_t start_row = 0;
  uint16_t start_column = 0;
  size_t adjacent_frames = 0;

  // Loop through the bitmap
  for (uint16_t row = 0; row < m_total_entries; ++row) {

    // If the row is full continue
    if(m_bit_map[row] == 0xFFFFFFFFFFFFFFF)
      continue;

    for (uint16_t column = 0; column < ROW_BITS; ++column) {

      // Prevent out-of-bounds shifts if column exceeds the bit-width of uint64_t
      if (column >= ROW_BITS)
        break;

      // If this bit is not free, reset the adjacent frames
      if (m_bit_map[row] & (1ULL << column)) {
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
        for (uint16_t i = 0; i < frame_count; ++i) {

          // Get the location of the bit
          uint16_t index = start_row + (start_column + i) / ROW_BITS;
          uint16_t bit = (start_column + i) % ROW_BITS;

          // Skip if index exceeds bounds
          if (index >= m_total_entries || bit >= ROW_BITS) {
            ASSERT(false, "Index out of bounds\n")
          }

          m_bit_map[index] |= (1ULL << bit); // Mark the bit as used
        }

        // Clear the lock
        m_lock.unlock();

        // Return the address
        return (void*)(start_address + (start_row * ROW_BITS + start_column) * s_page_size);
      }
    }
  }

  // Error cant allocate that much
  m_lock.unlock();
  ASSERT(false, "Cannot allocate that much memory\n")
  return nullptr;
}

/**
 * @brief Frees an area of physical memory
 * @param start_address The start of the block
 * @param size The size to free
 */
void PhysicalMemoryManager::free_area(uint64_t start_address, size_t size) {

    // Check how many frames are needed
    size_t frame_count = size_to_frames(size);
    uint64_t frame_address = start_address / s_page_size;

    // Check if the address is valid
    if(frame_address >= m_bitmap_size)
      return;

    // Wait to be able to free
    m_lock.lock();

    // Mark the frames as not used
    m_used_frames -= frame_count;
    for (uint16_t i = 0; i < frame_count; ++i)
      m_bit_map[(frame_address + i) / ROW_BITS] &= ~(1 << ((frame_address + i) % ROW_BITS));


    // Clear the lock
    m_lock.unlock();
}

/**
 * @brief Checks if a sub table is available a table, if not it is created
 * @param table The table to check
 * @param next_table The table to create
 * @param index The index of the table to create
 */
void PhysicalMemoryManager::create_table(pml_t* table, pml_t* next_table, size_t index) {

  // If the table is already created return
  if(table_has_entry(table, index))
    return;

  // Create the table
  uint64_t *new_table = (uint64_t *)allocate_frame();

  // Set the table to the next table
  table -> entries[index] = create_page_table_entry((uint64_t)new_table, Present | Write);

  // Invalidate the TLB entry for the recursive mapping.
  asm volatile("invlpg (%0)" ::"r" (next_table) : "memory");

  // Clear the table
  clean_page_table((uint64_t*)next_table);

}

/**
 * @brief Gets or creates a table in a table
 * @param table The table to check
 * @param index The index of the table to get or create
 * @param flags The flags to set the table to
 * @return The created table
 */
uint64_t* PhysicalMemoryManager::get_or_create_table(uint64_t *table, size_t index, size_t flags) {

  // Address mask
  uint64_t mask = 0xFFFFFFF000;

  // If the table is already created return it
  if(table[index] & 0b1)
      return (uint64_t *) MemoryManager::to_dm_region((uintptr_t) table[index] & mask);

  // Need to create the table
  uint64_t *new_table = (uint64_t*)allocate_frame();
  table[index] = (uint64_t) new_table | flags;

  // Move the table to the higher half
  new_table = (uint64_t*)MemoryManager::to_dm_region((uintptr_t) new_table);

  // Clear the table
  clean_page_table(new_table);

  // All done
  return new_table;
}

/**
 * @brief Checks if a table has an entry
 * @param table The table to check
 * @param index The index of the table to check
 * @return True if the table has an entry
 */
bool PhysicalMemoryManager::table_has_entry(pml_t *table, size_t index) {
  // Get the entry
  pte_t* entry = &table -> entries[index];

  // If the table is already created return it
  return (entry -> present);

}

/**
 * @brief Gets a table if it exists
 * @param table  The table to check
 * @param index The index of the table to get
 * @return  The table if it exists, nullptr otherwise
 */
uint64_t *PhysicalMemoryManager::get_table_if_exists(uint64_t *table, size_t index) {

  // Address mask
  uint64_t mask = 0xFFFFFFF000;

  // If the table is null return null
  if(table == nullptr)
    return nullptr;

  // Check if the table is present
  if(table[index] & 0b1)
      return (uint64_t *) MemoryManager::to_dm_region((uintptr_t) table[index] & mask);

  // Not found
  return nullptr;
}

#define ENTRIES_TO_ADDRESS(pml4, pdpr, pd, pt)((pml4 << 39) | (pdpr << 30) | (pd << 21) |  (pt << 12))
/**
 * @brief Maps a physical address to a virtual address, using the kernel's pml4 table
 * @param physical_address The physical address to map
 * @param address The virtual address to map to
 * @param flags The flags to set the mapping to
 * @return The virtual address
 */
virtual_address_t* PhysicalMemoryManager::map(physical_address_t *physical_address, virtual_address_t* address, size_t flags) {

  // Base information
  pml_t* pml4_table = (pml_t *)m_pml4_root_address;
  size_t base_addr = 0xFFFF000000000000;

  // Get the indexes
  uint16_t pml4_index = PML4_GET_INDEX((uint64_t) address);
  uint16_t pdpr_index = PML3_GET_INDEX((uint64_t) address);
  uint16_t pd_index   = PML2_GET_INDEX((uint64_t) address);
  uint16_t pt_index   = PML1_GET_INDEX((uint64_t) address);

  // Get the tables
  pml_t *pdpr_table =(pml_t *) (base_addr | ENTRIES_TO_ADDRESS(510l,510l,510l, (uint64_t) pml4_index));
  pml_t *pd_table = (pml_t *) (base_addr | ENTRIES_TO_ADDRESS(510l,510l, (uint64_t) pml4_index, (uint64_t) pdpr_index));
  pml_t *pt_table = (pml_t *) (base_addr | ENTRIES_TO_ADDRESS(510l, (uint64_t) pml4_index, (uint64_t) pdpr_index, (uint64_t) pd_index));

  // Create the tables
  create_table(pml4_table, pdpr_table, pml4_index);
  create_table(pdpr_table, pd_table, pdpr_index);
  create_table(pd_table, pt_table, pd_index);

  // Get the entry
  pte_t* pte = &pt_table -> entries[pt_index];

  // If it already exists return the address
  if(pte -> present)
    return address;

  // Map the physical address to the virtual address
 *pte = create_page_table_entry((uint64_t)physical_address, flags);


  // Flush the TLB
  asm volatile("invlpg (%0)" ::"r" (address) : "memory");

  return address;
}

/**
 * @brief Maps a physical address to a virtual address
 * @param physical The physical address
 * @param virtual_address The virtual address
 * @param flags The flags to set the mapping to
 * @param pml4_table The pml4 table to use
 * @return The virtual address
 */
virtual_address_t* PhysicalMemoryManager::map(physical_address_t *physical, virtual_address_t *virtual_address, size_t flags, uint64_t *pml4_table) {

    // Get the indexes
    uint16_t pml4_index = PML4_GET_INDEX((uint64_t) virtual_address);
    uint16_t pdpr_index = PML3_GET_INDEX((uint64_t) virtual_address);
    uint16_t pd_index   = PML2_GET_INDEX((uint64_t) virtual_address);
    uint16_t pt_index   = PML1_GET_INDEX((uint64_t) virtual_address);

    // If it is in a lower region then assume it is the user space
    uint8_t is_user = !(MemoryManager::in_higher_region((uint64_t)virtual_address));
    if(is_user) {
      // Change the flags to user
      flags |= User;
      is_user = User;

    }

    // Store the tables
    uint64_t* pdpr_table = get_or_create_table(pml4_table, pml4_index, Present | Write | is_user);
    uint64_t* pd_table = get_or_create_table(pdpr_table, pdpr_index, Present | Write | is_user);
    uint64_t* pt_table = get_or_create_table(pd_table, pd_index, Present | Write | is_user);

    // If the page is already mapped return the address
    if(pt_table[pt_index] & 0b1)
      return virtual_address;

    // Map the physical address to the virtual address
    pt_table[pt_index] = (uint64_t) physical | flags;

    // Flush the TLB
    asm volatile("invlpg (%0)" ::"r" (virtual_address) : "memory");

    return virtual_address;
}

/**
 * @brief Allocates a physical address to a virtual address
 * @param virtual_address The virtual address
 * @param flags The flags to set the mapping to
 * @return The virtual address
 */
virtual_address_t* PhysicalMemoryManager::map(virtual_address_t *virtual_address, size_t flags) {

  // Create a new physical address for the frame
  physical_address_t* physical_address = (physical_address_t *)allocate_frame();

  // Map the physical address to the virtual address
  return map(physical_address, virtual_address, flags);

}

/**
 * @brief Allocates a new area physical memory to a area virtual address
 * @param virtual_address_start The start of the virtual address
 * @param length The length of the area
 * @param flags The flags to set the mapping to
 */
void PhysicalMemoryManager::map_area(virtual_address_t* virtual_address_start, size_t length, size_t flags) {

    // Get the size of the area
    size_t size = size_to_frames(length);

    // Map the required frames
    for (size_t i = 0; i < size; ++i)
        map(virtual_address_start + (i * s_page_size), flags);

}

/**
 * @brief Maps an area of physical memory to a virtual address
 * @param physical_address_start The start of the physical address
 * @param virtual_address_start The start of the virtual address
 * @param length  The length of the area
 * @param flags The flags to set the mapping to
 */
void PhysicalMemoryManager::map_area(physical_address_t* physical_address_start, virtual_address_t* virtual_address_start, size_t length, size_t flags) {

  // Get the size of the area
  size_t size = size_to_frames(length);

  // Map the required frames
  for (size_t i = 0; i < size; ++i)
    map(physical_address_start + (i * s_page_size), virtual_address_start + (i * s_page_size), flags);

}

/**
 * @brief Maps a physical address to its virtual address counter-part
 * @param physical_address The physical address to map
 * @param flags The flags to set the mapping to
 */
void PhysicalMemoryManager::identity_map(physical_address_t *physical_address, size_t flags) {

  // Map the physical address to its virtual address counter-part
  map(physical_address, physical_address, flags);

}

/**
 * @brief Unmaps a virtual address using the kernel's pml4 table
 * @param virtual_address The virtual address to unmap
 */
void PhysicalMemoryManager::unmap(virtual_address_t* virtual_address) {

  // Base information
  pml_t* pml4_table = (pml_t *)m_pml4_root_address;
  size_t base_addr = 0xFFFF000000000000;

  // Get the indexes
  uint16_t pml4_index = PML4_GET_INDEX((uint64_t) virtual_address);
  uint16_t pdpr_index = PML3_GET_INDEX((uint64_t) virtual_address);
  uint16_t pd_index   = PML2_GET_INDEX((uint64_t) virtual_address);
  uint16_t pt_index   = PML1_GET_INDEX((uint64_t) virtual_address);

  // Get the tables
  pml_t *pdpr_table =(pml_t *) (base_addr | ENTRIES_TO_ADDRESS(510l,510l,510l, (uint64_t) pml4_index));
  pml_t *pd_table = (pml_t *) (base_addr | ENTRIES_TO_ADDRESS(510l,510l, (uint64_t) pml4_index, (uint64_t) pdpr_index));
  uint64_t* pt_table = (uint64_t *) (base_addr | ENTRIES_TO_ADDRESS(510l, (uint64_t) pml4_index, (uint64_t) pdpr_index, (uint64_t) pd_index));

  // Check if the entry is present
  if(table_has_entry(pml4_table, pml4_index) && table_has_entry(pdpr_table, pdpr_index) && table_has_entry(pd_table, pd_index))
    return;

  // Check if the entry isn't present
  if(!(pt_table[pt_index] & 0b1))
    return;

  // Unmap the entry
  pt_table[pt_index] = 0x00l;

  // Flush the TLB
  asm volatile("invlpg (%0)" ::"r" (virtual_address) : "memory");
}

/**
 * @brief Unmaps a virtual address
 * @param virtual_address The virtual address to unmap
 * @param pml4_root The pml4 table to use
 */
void PhysicalMemoryManager::unmap(virtual_address_t *virtual_address, uint64_t *pml4_root) {

    // Get the indexes
    uint16_t pml4_index = PML4_GET_INDEX((uint64_t) virtual_address);
    uint16_t pdpr_index = PML3_GET_INDEX((uint64_t) virtual_address);
    uint16_t pd_index   = PML2_GET_INDEX((uint64_t) virtual_address);
    uint16_t pt_index   = PML1_GET_INDEX((uint64_t) virtual_address);

    // Get the tables
    uint64_t* pdpr_table = get_table_if_exists(pml4_root, pml4_index);
    uint64_t* pd_table = get_table_if_exists(pdpr_table, pdpr_index);
    uint64_t* pt_table = get_table_if_exists(pd_table, pd_index);

    // Check if the tables are present (if any are not then a pt entry will not be present)
    if(pt_table == nullptr)
      return;


    // Check if the entry is present
    if(!(pt_table[pt_index] & 0b1))
      return;

    // Unmap the entry
    pt_table[pt_index] = 0x00l;

    // Flush the TLB
    asm volatile("invlpg (%0)" ::"r" (virtual_address) : "memory");

}

/**
 * @brief Unmaps an area of virtual memory
 * @param virtual_address_start The start of the area
 * @param length The length of the area
 */
void PhysicalMemoryManager::unmap_area(virtual_address_t *virtual_address_start, size_t length) {

    // Get the size of the area
    size_t size = size_to_frames(length);

    // Unmap the required frames
    for (size_t i = 0; i < size; ++i)
      unmap(virtual_address_start + (i * s_page_size));
}

/**
 * @brief Checks if a virtual address is mapped and whether it points to the correct physical address
 * @param physical_address The physical address to check (if 0 then wont check if correct entry)
 * @param virtual_address The address to check if it is mapped to
 * @return True if the physical address is mapped to the virtual address
 */
bool PhysicalMemoryManager::is_mapped(uintptr_t physical_address, uintptr_t virtual_address) {
  ASSERT(false, "Not implemented! Check if physical address is mapped to virtual address 0x%x -> 0x%x\n", physical_address, virtual_address)
  // TODO: Implement
  return false;
}

/**
 * @brief Cleans a page table (fills it with 0 or null entries)
 * @param table The table to clean
 */
void PhysicalMemoryManager::clean_page_table(uint64_t *table) {
  for(int i = 0; i < 512; i++){
        table[i] = 0x00l;
  }
}

/**
 * @brief Creates a page table entry
 * @param address The address to create the entry for
 * @param flags The flags to set the entry to
 * @return The created page table entry
 */
pte_t PhysicalMemoryManager::create_page_table_entry(uintptr_t address, size_t flags) {

  pte_t page =  (pte_t){
    .present = 1,
    .write = (flags & Write) != 0,
    .user = (flags & User) != 0,
    .write_through = (flags & (1 << 7)) != 0,
    .cache_disabled = 0,
    .accessed = 0,
    .dirty = 0,
    .huge_page = (flags & HugePage) != 0,
    .global = 0,
    .available = 0,
    .physical_address = (uint64_t)address >> 12
  };

  return page;
}

/**
 * @brief Checks if a physical address is reserved by multiboot mmap
 * @param address The address to check
 * @return True if the address is reserved
 */
bool PhysicalMemoryManager::is_anonymous_available(size_t address) {

  // Return false if the address range is entirely within or overlaps with the multiboot reserved region
  if ((address > multiboot_tag_start && address + s_page_size < multiboot_tag_end) || (address + s_page_size > multiboot_tag_start && address < multiboot_tag_end)) {
    return false;
  }

  // Loop through the mmmap entries
  for (multiboot_mmap_entry *entry = m_mmap_tag->entries; (multiboot_uint8_t *)entry < (multiboot_uint8_t *)m_mmap_tag + m_mmap_tag->size; entry = (multiboot_mmap_entry *)((unsigned long)entry + m_mmap_tag->entry_size)) {

    // If it doesn't overlap with the mmap entry
    if ((entry -> addr + entry -> len) < (address + s_page_size))
      continue;

    // If it is not available
    if(entry -> type != MULTIBOOT_MEMORY_AVAILABLE)
      continue;

    // Check if the address is overwriting with some reserved memory
    if(m_multiboot -> is_reserved(address))
       return false;

    // Memory is available
    return true;

  }

  // Memory is not available
  return false;
}


/**
 * @brief Gets the address of the bitmap
 * @return The address of the bitmap
 */
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
        ASSERT(space >= (m_bitmap_size / 8 + 1), "Not enough space for the bitmap\n");

        // Return the address
        return (uint64_t*)MemoryManager::to_dm_region(entry -> addr + offset);
    }

  // Error no space for the bitmap
  ASSERT(false, "No space for the bitmap\n");
  return nullptr;
}

/**
 * @brief Gets the pml4 root address for the kernel
 * @return The pml4 root address
 */
uint64_t *PhysicalMemoryManager::get_pml4_root_address() {
    return m_pml4_root_address;
}

/**
 * @brief Gets total the memory size available for use (allocated or not)
 * @return The memory size in bytes
 */
uint64_t PhysicalMemoryManager::get_memory_size() {
  return m_memory_size;
}

/**
 * @brief Gets the memory currently used
 * @return The memory size in bytes
 */
uint64_t PhysicalMemoryManager::get_memory_used() {
    return m_used_frames * s_page_size;
}

/**
 * @brief Aligns a address to the page size
 * @param size The address to align
 * @return The aligned address
 */
size_t PhysicalMemoryManager::align_direct_to_page(size_t size) {
  return (size & (~(s_page_size - 1)));
}

/**
 * @brief Reserves a physical address
 * @param address The address to reserve
 */
void PhysicalMemoryManager::reserve(uint64_t address) {


  // If the address is not part of physical memory then return
  if(address >= m_memory_size)
    return;

  // Get the address to a page
  address = align_direct_to_page(address);

  // Set the bit to 1 in the bitmap
  m_bit_map[address / ROW_BITS] |= (1 << (address % ROW_BITS));


  _kprintf("Reserved Address: 0x%x\n", address);

}

/**
 * @brief Reserves an area of physical memory
 * @param address The start of the area
 * @param size The size of the area
 */
void PhysicalMemoryManager::reserve(uint64_t address, size_t size) {

  if(address >= m_memory_size)
    return;

  // Wait to be able to reserve
  m_lock.lock();

  // Align address and size to page boundaries
  address = align_direct_to_page(address);
  size = align_up_to_page(size, s_page_size);

  // Calculate how many pages need to be reserved
  size_t page_count = size / s_page_size;
  // Convert the starting address to a frame index
  uint64_t frame_index = address / s_page_size;

  for (size_t i = 0; i < page_count; ++i) {
    m_bit_map[(frame_index + i) / ROW_BITS] |= (1ULL << ((frame_index + i) % ROW_BITS));
  }

  // Update the used frames
  m_used_frames += page_count;

  // Clear the lock
  m_lock.unlock();
  _kprintf("Reserved Address: 0x%x - 0x%x (length of 0x%x)\n", address, address + size, size);
}
