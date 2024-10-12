//
// Created by 98max on 10/20/2022.
//

#include <memory/memorymanagement.h>

using namespace MaxOS;
using namespace MaxOS::memory;

MemoryManager* MemoryManager::s_active_memory_manager = 0;

MemoryManager::MemoryManager(multiboot_tag_mmap* memory_map)
{

     size_t heap = 0;
     size_t size = 0;

    // Find the available memory
    for (multiboot_memory_map_t* mmap = memory_map->entries; (unsigned long)mmap < (unsigned long)memory_map + memory_map->size;
         mmap = (multiboot_memory_map_t*)((unsigned long)mmap + memory_map->entry_size)) {

        // If the memory is available then use it
        if(mmap -> type == MULTIBOOT_MEMORY_AVAILABLE){
            heap = mmap -> addr;
            size = mmap -> len;
        }
    }

    // Add a 10MB offset to the heap
        heap += 0x1000000;

    s_active_memory_manager = this;

    //Prevent wiring outside the area that is allowed to write
    if(size < sizeof(MemoryChunk)){

        this ->m_first_memory_chunk = 0;

    }else{

        this ->m_first_memory_chunk = (MemoryChunk*)heap;
        m_first_memory_chunk-> allocated = false;
        m_first_memory_chunk-> prev = 0;
        m_first_memory_chunk-> next = 0;
        m_first_memory_chunk-> size = size - sizeof(MemoryChunk);
    }
}

MemoryManager::~MemoryManager() {
    if(s_active_memory_manager == this)
      s_active_memory_manager = 0;
}

/**
 * @brief Allocates a block of memory
 *
 * @param size size of the block
 * @return a pointer to the block, 0 if no block is available
 */
void* MemoryManager::malloc(size_t size) {

    MemoryChunk* result = 0;

    // Find the next free chunk that is big enough
    for (MemoryChunk* chunk = m_first_memory_chunk; chunk != 0 && result == 0; chunk = chunk->next) {
        if(chunk -> size > size && !chunk -> allocated)
            result = chunk;
    }

    // If there is no free chunk then return 0
    if(result == 0)
        return 0;

    // If there is space to split the chunk
    if(result -> size < size + sizeof(MemoryChunk) + 1) {
        result->allocated = true;
        return (void *)(((size_t)result) + sizeof(MemoryChunk));
    }


    // Create a new chunk after the current one
    MemoryChunk* temp = (MemoryChunk*)((size_t)result + sizeof(MemoryChunk) + size);

    // Set the new chunk's properties and insert it into the linked list
    temp -> allocated = false;
    temp -> size =  result->size - size - sizeof(MemoryChunk);
    temp -> prev = result;
    temp -> next = result -> next;

    // If there is a chunk after the current one then set its previous to the new chunk
    if(temp -> next != 0)
       temp -> next -> prev = temp;

    // Current chunk is now allocated and is pointing to the new chunk
    result->size = size;
    result -> allocated = true;
    result->next = temp;

    return (void*)(((size_t)result) + sizeof(MemoryChunk));
}


/**
 * @brief Frees a block of memory
 *
 * @param pointer A pointer to the block
 */
void MemoryManager::free(void *pointer) {


    // Create a new free chunk
    MemoryChunk* chunk = (MemoryChunk*)((size_t)pointer - sizeof(MemoryChunk));
    chunk -> allocated = false;

    // If there is a free chunk before this chunk then merge them
    if(chunk -> prev != 0 && !chunk -> prev -> allocated){

        // Increase the previous chunk's size and remove the current chunk from the linked list
        chunk->prev->size += chunk->size + sizeof(MemoryChunk);
        chunk -> prev -> next = chunk -> next;

        // If there is a next chunk then ensure this chunk is removed from its linked list
        if(chunk -> next != 0)
            chunk -> next -> prev = chunk->prev;

        // Chunk is now the previous chunk
        chunk = chunk -> prev;

    }

    // If there is a free chunk after this chunk then merge them
    if(chunk -> next != 0 && !chunk -> next -> allocated){

        // Increase the current chunk's size and remove the next chunk from the linked list
        chunk -> size += chunk -> next -> size + sizeof(MemoryChunk);
        chunk -> next = chunk -> next -> next;

        // Remove the just merged chunk from the linked list
        if(chunk -> next != 0)
            chunk -> next -> prev = chunk;

    }
}

/**
 * @brief Returns the amount of memory used
 * @return The amount of memory used
 */
int MemoryManager::memory_used() {

        int result = 0;

        // Loop through all the chunks and add up the size of the allocated chunks
        for (MemoryChunk* chunk = m_first_memory_chunk; chunk != 0; chunk = chunk->next)
            if(chunk -> allocated)
                result += chunk -> size;

        return result;
}
void* MemoryManager::to_higher_region(uintptr_t physical_address) {

  // If it's in the lower half then add the offset
  if(physical_address < s_higher_half_kernel_offset)
    return (void*)(physical_address + s_higher_half_kernel_offset);

  // Must be in the higher half
  return (void*)physical_address;

}

void *MemoryManager::to_lower_region(uintptr_t virtual_address) {
  // If it's in the lower half then add the offset
  if(virtual_address > s_higher_half_kernel_offset)
    return (void*)(virtual_address - s_higher_half_kernel_offset);

  // Must be in the lower half
  return (void*)virtual_address;
}

void *MemoryManager::to_io_region(uintptr_t physical_address) {

  if(physical_address < s_higher_half_mem_offset)
    return (void*)(physical_address + s_higher_half_mem_offset);

  // Must be in the higher half
  return (void*)physical_address;

}
void *MemoryManager::to_dm_region(uintptr_t physical_address) {

  if(physical_address < s_higher_half_offset)
    return (void*)(physical_address + s_hh_direct_map_offset);

  // Must be in the higher half
  return (void*)physical_address;

}

bool MemoryManager::in_higher_region(uintptr_t virtual_address) {
  return virtual_address & (1l << 62);
}

//Redefine the default object functions with memory orientated ones (defaults disabled in makefile)

void* operator new(size_t size) throw(){

    // Use the memory manager to allocate the memory
    if(MaxOS::memory::MemoryManager::s_active_memory_manager != 0)
        return MaxOS::memory::MemoryManager::s_active_memory_manager-> malloc(size);

    return 0;

}

void* operator new[](size_t size) throw(){

    // Use the memory manager to allocate the memory
    if(MaxOS::memory::MemoryManager::s_active_memory_manager != 0)
        return MaxOS::memory::MemoryManager::s_active_memory_manager-> malloc(size);

    return 0;

}

void* operator new(size_t, void* pointer){

    return pointer;

}
void* operator new[](size_t, void* pointer){

    return pointer;

}

void operator delete(void* pointer){

    // Use the memory manager to free the memory
    if(MaxOS::memory::MemoryManager::s_active_memory_manager != 0)
        return MaxOS::memory::MemoryManager::s_active_memory_manager-> free(pointer);

}

void operator delete[](void* pointer){

    // Use the memory manager to free the memory
    if(MaxOS::memory::MemoryManager::s_active_memory_manager != 0)
        return MaxOS::memory::MemoryManager::s_active_memory_manager-> free(pointer);

}

// NOTE: The size_t parameter is ignored, compiler was just complaining
void operator delete(void* pointer, size_t){

    // Use the memory manager to free the memory
    if(MaxOS::memory::MemoryManager::s_active_memory_manager != 0)
        return MaxOS::memory::MemoryManager::s_active_memory_manager-> free(pointer);

}

void operator delete[](void* pointer, size_t){

    // Use the memory manager to free the memory
    if(MaxOS::memory::MemoryManager::s_active_memory_manager != 0)
        return MaxOS::memory::MemoryManager::s_active_memory_manager-> free(pointer);

}