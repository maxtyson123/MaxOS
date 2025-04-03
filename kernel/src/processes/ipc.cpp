//
// Created by 98max on 24/03/2025.
//
#include <processes/ipc.h>
using namespace MaxOS;
using namespace MaxOS::processes;
using namespace MaxOS::common;
using namespace MaxOS::memory;

#include <common/kprint.h>
#include <processes/scheduler.h>      //TODO: Circular dependency, need to fix

/**
 * @brief Creates a new IPC handler
 */
InterProcessCommunicationManager::InterProcessCommunicationManager() {

  // Clear the spinlock
  m_lock = Spinlock();

}


InterProcessCommunicationManager::~InterProcessCommunicationManager() {

  // Free all the shared memory
  for(auto block : m_shared_memory_blocks){
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
SharedMemory* InterProcessCommunicationManager::alloc_shared_memory(size_t size, string name) {

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
  auto* block = new SharedMemory(name, size);

  // Add the block to the list
  m_shared_memory_blocks.push_back(block);

  // Clear the lock
  m_lock.unlock();

  _kprintf("Created shared memory block %s at 0x%x\n", name.c_str(), block -> physical_address());

  // Return the block
  return block;
}

/**
 * @brief Gets a shared memory block by name
 *
 * @param name The name of the block
 * @return The shared memory block or nullptr if not found
 */
SharedMemory* InterProcessCommunicationManager::get_shared_memory(const string& name) {

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
void InterProcessCommunicationManager::free_shared_memory(uintptr_t physical_address) {

    // Find the block
    for(auto block : m_shared_memory_blocks){

        if(block -> physical_address() == physical_address){
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
void InterProcessCommunicationManager::free_shared_memory(const string& name) {


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
void InterProcessCommunicationManager::free_shared_memory(SharedMemory* block) {

  // Wait for the lock
  m_lock.lock();

  // Decrement the use count
  block->use_count--;

  // If the block is still in use
  if (block->use_count > 0) {
    m_lock.unlock();
    return;
  }

  _kprintf("Deleting shared memory block %s at 0x%x\n", block->name->c_str(), block -> physical_address());

  // Free the block
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
SharedMessageEndpoint* InterProcessCommunicationManager::create_message_endpoint(const string& name) {

  // Wait for the lock
  m_lock.lock();

  // Make sure the name is unique
  SharedMessageEndpoint* existing = get_message_endpoint(name);
  if(existing != nullptr){
      m_lock.unlock();
      return nullptr;
  }

  // Create the endpoint
  auto* endpoint = new SharedMessageEndpoint(name);

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
SharedMessageEndpoint* InterProcessCommunicationManager::get_message_endpoint(const string& name) {

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
void InterProcessCommunicationManager::free_message_endpoint(const string& name) {

  // Find the endpoint
  SharedMessageEndpoint* endpoint = get_message_endpoint(name);

  // Free the endpoint
  free_message_endpoint(endpoint);

}

/**
 * @brief Deletes a message endpoint - all messages will be lost
 *
 * @param endpoint
 */
void InterProcessCommunicationManager::free_message_endpoint(SharedMessageEndpoint* endpoint) {

  // Make sure the endpoint exists
  if(endpoint == nullptr)
     return;

  // Make sure the endpoint is owned by the current process
  if(endpoint -> owned_by_current_process())
      return;

  // Delete the endpoint
  delete endpoint;

}

/**
 * @brief Creates a new shared memory block
 *
 * @param name The name of the block
 */
SharedMemory::SharedMemory(string name, size_t size)
: m_size(size),
  name(new string(name))
{

  m_physical_address  = (uintptr_t)PhysicalMemoryManager::s_current_manager -> allocate_area(0, size);
  m_owner_pid         = Scheduler::current_process()->pid();

}

SharedMemory::~SharedMemory() {

  // Free the memory
  PhysicalMemoryManager::s_current_manager->free_area(m_physical_address, m_size);
}

/**
 * @brief Gets the physical address of the shared memory block
 *
 * @return The physical address
 */
uintptr_t SharedMemory::physical_address() const {

  return m_physical_address;

}

/**
 * @brief Gets the size of the shared memory block
 *
 * @return The size
 */
size_t SharedMemory::size() const {

  return m_size;

}

SharedMessageEndpoint::SharedMessageEndpoint(string name)
: name(new string(name))
{

  // Make the queue in the user's memory space
  m_queue = (ipc_message_queue_t*)MemoryManager::malloc(sizeof(ipc_message_queue_t));

  // Get the owner
  m_owner_pid = Scheduler::current_process() -> pid();


}

SharedMessageEndpoint::~SharedMessageEndpoint() {

  // Wait for the lock
  m_message_lock.lock();

  // Delete the messages
  ipc_message_t* message = m_queue -> messages;
  while(message != nullptr){
    auto* next = (ipc_message_t*)message -> next_message;
    MemoryManager::free(message -> message_buffer);
    message = next;
  }

  // Free the queue
  MemoryManager::free(m_queue);

  // Free the name
  delete name;

  m_message_lock.unlock();
}

/**
 * @brief Gets the message queue for the endpoint
 *
 * @return The message queue
 */
ipc_message_queue_t *SharedMessageEndpoint::queue() const {

  // Return the queue
  return m_queue;

}

/**
 * @brief Checks if the current process can delete the endpoint
 *
 * @return True if the current process can delete the endpoint
 */
bool SharedMessageEndpoint::owned_by_current_process() const {

    // Check if the owner is the current process
    return m_owner_pid == Scheduler::current_process() -> pid();

}

/**
 * @brief Queues a message to be processed by the endpoint (buffer can be freed as it is copied in to the receiving process's memory)
 *
 * @param message The message to queue
 * @param size The size of the message
 */
void SharedMessageEndpoint::queue_message(void *message, size_t size) {

  // Wait for the  lock
  m_message_lock.lock();

  // Copy the buffer into the kernel so that the endpoint can access it
  auto* kernel_copy = (uintptr_t*)new char[size];
  memcpy(kernel_copy, message, size);

  //Switch to endpoint's memory space
  MemoryManager::switch_active_memory_manager(Scheduler::get_process(m_owner_pid) -> memory_manager);

  // Create the message & copy it into the endpoint's memory space
  auto* new_message = (ipc_message_t*)MemoryManager::malloc(sizeof(ipc_message_t));
  void* new_buffer = MemoryManager::malloc(size);
  new_message -> message_buffer = memcpy(new_buffer, kernel_copy, size);
  new_message -> message_size = size;
  new_message -> next_message = 0;

  // Add the message to the end of the queue
  ipc_message_t* current = m_queue -> messages;
  while(current != nullptr){
    if(current -> next_message == 0){
      current -> next_message = (uintptr_t)new_message;
      break;
    }
    current = (ipc_message_t*)current -> next_message;
  }

  // If it was the first message
  if (current == nullptr)
    m_queue->messages = new_message;

  //Switch back from endpoint's memory space
  MemoryManager::switch_active_memory_manager(Scheduler::current_process() -> memory_manager);

  // Free the lock & kernel copy
  delete[] kernel_copy;
  m_message_lock.unlock();
}
