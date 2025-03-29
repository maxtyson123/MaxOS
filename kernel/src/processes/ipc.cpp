//
// Created by 98max on 24/03/2025.
//
#include <processes/ipc.h>
using namespace MaxOS;
using namespace MaxOS::processes;
using namespace MaxOS::common;
using namespace MaxOS::memory;

#include <common/kprint.h>
#include <processes/scheduler.h>      //TODO: Cicrular dependency, need to fix

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
 *
 * @param size The size of the block
 * @param name The name of the block
 * @return The shared memory block
 */
ipc_shared_memory_t *IPC::alloc_shared_memory(size_t size, string name) {

  // Wait for the lock
  m_lock.lock();

  // Make sure the name is unique
  for(auto endpoint : m_message_endpoints){
    if(endpoint -> name -> equals(name)){
      m_lock.unlock();
      return nullptr;
    }
  }

  // Create the shared memory block
  ipc_shared_memory_t* block = new ipc_shared_memory_t;
  block -> physical_address  = (uintptr_t)PhysicalMemoryManager::s_current_manager -> allocate_area(0, size);
  block -> size              = size;
  block -> use_count         = 1;
  block -> owner_pid         = Scheduler::get_current_process()->get_pid();
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
 *
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
 *
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
 *
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
 *
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

/**
 * @brief Creates a endpoint for message passing
 *
 * @param name The name of the endpoint
 * @return The endpoint
 */
ipc_message_endpoint_t* IPC::create_message_endpoint(string name) {

  // Wait for the lock
  m_lock.lock();

  // Make sure the name is unique
  ipc_message_endpoint_t* existing = get_message_endpoint(name);
  if(existing != nullptr){
      m_lock.unlock();
      return nullptr;
  }

  // Create the endpoint (With the queue on in the user's memory space)
  ipc_message_endpoint_t* endpoint = new ipc_message_endpoint_t;
  endpoint -> queue = (ipc_message_queue_t*)MemoryManager::malloc(sizeof(ipc_message_queue_t));
  endpoint -> queue -> messages = nullptr;
  endpoint -> owner_pid = Scheduler::get_current_process() -> get_pid();
  endpoint -> name = new String(name);

  // Add the endpoint to the list
  m_message_endpoints.push_back(endpoint);

  // Free the lock
  m_lock.unlock();

  // Return the endpoint
  return endpoint;
}

/**
 * @brief Gets a message endpoint by name
 *
 * @param name The name of the endpoint
 * @return The endpoint or nullptr if not found
 */
ipc_message_endpoint_t *IPC::get_message_endpoint(string name) {

  // Try to find the endpoint
  for(auto endpoint : m_message_endpoints){
      if(endpoint -> name -> equals(name))
        return endpoint;
  }

  // Not found
  return nullptr;

}

/**
 * @brief Deletes a message endpoint by name - all messages will be lost
 *
 * @param name The name of the endpoint
 */
void IPC::free_message_endpoint(string name) {

  // Find the endpoint
  ipc_message_endpoint_t* endpoint = get_message_endpoint(name);

  // Free the endpoint
  free_message_endpoint(endpoint);

}

/**
 * @brief Deletes a message endpoint - all messages will be lost
 *
 * @param endpoint
 */
void IPC::free_message_endpoint(ipc_message_endpoint_t *endpoint) {

  // Make sure the endpoint exists
  if(endpoint == nullptr)
     return;

  // Make sure the endpoint is owned by the current process
  if(endpoint -> owner_pid != Scheduler::get_current_process() -> get_pid())
      return;

  // Wait for the lock
  endpoint -> message_lock.lock();

  // Delete the messages
  ipc_message_t* message = endpoint -> queue -> messages;
  while(message != nullptr){
      ipc_message_t* next = (ipc_message_t*)message -> next_message;
      delete message;
      message = next;
  }
  delete endpoint -> name;

  // Free the lock (nothing can be done to it now that the name isn't there)
  endpoint -> message_lock.unlock();

  // Delete the endpoint
  delete endpoint;

}

/**
 * @brief Finds the endpoint by name and adds a message to it's queue
 *
 * @param name The name of the endpoint
 * @param message The message to send
 * @param size The size of the message
 */
void IPC::send_message(string name, void* message, size_t size) {


  // Find the endpoint
  ipc_message_endpoint_t* endpoint = get_message_endpoint(name);
  if(endpoint == nullptr)
      return;


  // Send the message
  send_message(endpoint, message, size);


}

/**
 * @brief Sends the message to an endpoint (buffer can be freed as it is copied in to the receiving process's memory)
 *
 * @param endpoint The endpoint to send the message to
 * @param message The message to send
 * @param size The size of the message
 */
void IPC::send_message(ipc_message_endpoint_t *endpoint, void *message, size_t size) {

  // Wait for the endpoint lock
  endpoint -> message_lock.lock();

  // Copy the buffer into the kernel so that the endpoint can access it
  void* kernel_copy = new char[size];
  memcpy(kernel_copy, message, size);

  //Switch to endpoint's memory space
  MemoryManager::switch_active_memory_manager(Scheduler::get_process(endpoint -> owner_pid) -> memory_manager);

  // Create the message & copy it into the endpoint's memory space
  ipc_message_t* new_message = (ipc_message_t*)MemoryManager::malloc(sizeof(ipc_message_t));
  void* new_buffer = MemoryManager::malloc(size);
  new_message -> message_buffer = memcpy(new_buffer, kernel_copy, size);
  new_message -> message_size = size;
  new_message -> next_message = 0;

  // Add the message to the end of the queue
  ipc_message_t* current = endpoint -> queue -> messages;
  while(current != nullptr){
    if(current -> next_message == 0){
      current -> next_message = (uintptr_t)new_message;
      break;
    }
    current = (ipc_message_t*)current -> next_message;
  }

  // If it was the first message
  if (current == nullptr)
    endpoint->queue->messages = new_message;

  //Switch back from endpoint's memory space
  MemoryManager::switch_active_memory_manager(Scheduler::get_current_process() -> memory_manager);

  // Free the lock & kernel copy
  delete kernel_copy;
  endpoint -> message_lock.unlock();

}
