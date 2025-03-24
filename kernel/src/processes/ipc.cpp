//
// Created by 98max on 24/03/2025.
//
#include <processes/ipc.h>
using namespace MaxOS;
using namespace MaxOS::processes;
using namespace MaxOS::common;
using namespace MaxOS::memory;

#include <common/kprint.h>

/**
 * @brief Creates a new IPC handler
 */
IPC::IPC() {

  // Clear the spinlock
  m_lock = Spinlock();

}


IPC::~IPC() {

  // Free all the shared memory
  for(auto block : m_shared_memory_blocks){
        delete block -> name;
        delete block;
  }


}

/**
 * @brief Creates a new shared memory block
 * @param size The size of the block
 * @param name The name of the block
 * @return The shared memory block
 */
ipc_shared_memory_t *IPC::alloc_shared_memory(size_t size, string name) {

  // Wait for the lock
  m_lock.lock();

  // Create the shared memory block
  ipc_shared_memory_t* block = new ipc_shared_memory_t;
  block -> physical_address  = (uintptr_t)PhysicalMemoryManager::s_current_manager -> allocate_area(0, size);
  block -> size              = size;
  block -> use_count         = 1;
  block -> name              = new String(name);

  // Add the block to the list
  m_shared_memory_blocks.push_back(block);

  // Clear the lock
  m_lock.unlock();

  _kprintf("Created shared memory block %s at 0x%x\n", name.c_str(), block -> physical_address);

  // Return the block
  return block;
}

/**
 * @brief Gets a shared memory block by name
 * @param name The name of the block
 * @return The shared memory block or nullptr if not found
 */
ipc_shared_memory_t *IPC::get_shared_memory(string name) {

  // Wait for the lock
  m_lock.lock();

  // Find the block
  for(auto block : m_shared_memory_blocks){
      if(block -> name -> equals(name)){
        block -> use_count++;
        m_lock.unlock();
        return block;
      }
  }

  // Clear the lock
  m_lock.unlock();

  // Not found
  return nullptr;

}

/**
 * @brief Deletes a shared memory block by physical address
 * @param physical_address The physical address of the block
 */
void IPC::free_shared_memory(uintptr_t physical_address) {

    // Find the block
    for(auto block : m_shared_memory_blocks){

        if(block -> physical_address == physical_address){
            free_shared_memory(block);
            return;
        }

    }
}

/**
 * @brief Deletes a shared memory block by name
 * @param name The name of the block
 */
void IPC::free_shared_memory(string name) {


  // Find the block
  for (auto block : m_shared_memory_blocks) {

    // Check if the block is the one we are looking for
    if (!block->name->equals(name))
      continue;

    free_shared_memory(block);

  }


}

/**
 * @brief Deletes a shared memory block
 * @param block The block to delete (will only free memory if no one is using it)
 */
void IPC::free_shared_memory(ipc_shared_memory_t *block) {

  // Wait for the lock
  m_lock.lock();

  // Decrement the use count
  block->use_count--;

  // If the block is still in use
  if (block->use_count > 0) {
    m_lock.unlock();
    return;
  }

  _kprintf("Deleting shared memory block %s at 0x%x\n", block->name->c_str(), block->physical_address);

  // Free the block
  PhysicalMemoryManager::s_current_manager->free_area(block->physical_address, block->size);
  delete block->name;
  delete block;

  // Clear the lock
  m_lock.unlock();

}
