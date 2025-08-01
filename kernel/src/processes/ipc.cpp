//
// Created by 98max on 24/03/2025.
//
#include <processes/ipc.h>
using namespace MaxOS;
using namespace MaxOS::processes;
using namespace MaxOS::common;
using namespace MaxOS::memory;

#include <common/logger.h>
#include <processes/scheduler.h>      //TODO: Circular dependency, need to fix

/**
 * @brief Creates a new IPC handler
 */
InterProcessCommunicationManager::InterProcessCommunicationManager() = default;

InterProcessCommunicationManager::~InterProcessCommunicationManager() {

  // Free all the shared memory
  for(auto block : m_shared_memory_blocks)
        delete block;
}

/**
 * @brief Creates a new shared memory block
 *
 * @param size The size of the block
 * @param name The name of the block
 * @return The shared memory block
 */
SharedMemory* InterProcessCommunicationManager::alloc_shared_memory(size_t size, string name) {

  // Wait other processes to finish creating their blocks in order to ensure uniqueness
  m_lock.lock();

  // Make sure the name is unique
  for(auto endpoint : m_message_endpoints)
    if(endpoint -> name -> equals(name)){
      m_lock.unlock();
      return nullptr;
    }

  // Create the shared memory block
  auto* block = new SharedMemory(name, size);
  m_shared_memory_blocks.push_back(block);
  Logger::DEBUG() << "Created shared memory block " << name << " at 0x" << block -> physical_address() << "\n";

  // Return the block
  m_lock.unlock();
  return block;
}

/**
 * @brief Gets a shared memory block by name
 *
 * @param name The name of the block
 * @return The shared memory block or nullptr if not found
 */
SharedMemory* InterProcessCommunicationManager::get_shared_memory(const string& name) {

  // Wait for shared memory to be fully created before trying to search for it
  m_lock.lock();

  // Find the block
  for(auto block : m_shared_memory_blocks){

      // Block has wrong name
      if(!block -> name -> equals(name))
        continue;

      // Block is now in use
      block -> use_count++;
      m_lock.unlock();
      return block;
  }

  // Not found
  m_lock.unlock();
  return nullptr;

}

/**
 * @brief Deletes a shared memory block by physical address
 *
 * @param physical_address The physical address of the block
 */
void InterProcessCommunicationManager::free_shared_memory(uintptr_t physical_address) {

    // Find the block
    for(auto block : m_shared_memory_blocks)
      if(block -> physical_address() == physical_address){
          free_shared_memory(block);
          return;
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

  // One less process is using it
  block->use_count--;

  // If the block is in use let those processes handle it
  if (block->use_count > 0) {
    m_lock.unlock();
    return;
  }

  Logger::DEBUG() << "Deleting shared memory block " << block->name->c_str() << " at 0x" << block -> physical_address() << "\n";

  // Free the block
  delete block;
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
  if(get_message_endpoint(name) != nullptr){
      m_lock.unlock();
      return nullptr;
  }

  // Create the endpoint
  auto* endpoint = new SharedMessageEndpoint(name);
  m_message_endpoints.push_back(endpoint);

  m_lock.unlock();
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
  for(auto endpoint : m_message_endpoints)
      if(endpoint -> name -> equals(name))
        return endpoint;

  // Not found
  return nullptr;

}

/**
 * @brief Deletes a message endpoint by name - all messages will be lost
 *
 * @param name The name of the endpoint
 */
void InterProcessCommunicationManager::free_message_endpoint(const string& name) {

  free_message_endpoint(get_message_endpoint(name));
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

  // Store that this process owns it
  m_owner_pid = Scheduler::current_process() -> pid();


}

SharedMessageEndpoint::~SharedMessageEndpoint() {

  m_message_lock.lock();

  // Delete the messages
  ipc_message_t* message = m_queue -> messages;
  while(message != nullptr){
    auto* next = (ipc_message_t*)message -> next_message;
    MemoryManager::free(message -> message_buffer);
    message = next;
  }

  // Free the memory
  MemoryManager::free(m_queue);
  delete name;

  m_message_lock.unlock();
}

/**
 * @brief Gets the message queue for the endpoint
 *
 * @return The message queue
 */
ipc_message_queue_t *SharedMessageEndpoint::queue() const {

  return m_queue;

}

/**
 * @brief Checks if the current process can delete the endpoint
 *
 * @return True if the current process can delete the endpoint
 */
bool SharedMessageEndpoint::owned_by_current_process() const {

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

  // Copy the buffer into the kernel so that the endpoint (this code) can access it when the memory spaces are switched
  buffer_t kernel_copy(size);
  kernel_copy.copy_from(message,size);

  //Switch to endpoint's memory space
  MemoryManager::switch_active_memory_manager(Scheduler::get_process(m_owner_pid) -> memory_manager);

  // Create the message & copy it into the endpoint's memory space
  auto* new_message = (ipc_message_t*)MemoryManager::malloc(sizeof(ipc_message_t));
  void* new_buffer = MemoryManager::malloc(size);
  new_message -> message_buffer = memcpy(new_buffer, kernel_copy.raw(), size);
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

  // Clean up
  MemoryManager::switch_active_memory_manager(Scheduler::current_process() -> memory_manager);
  m_message_lock.unlock();
}