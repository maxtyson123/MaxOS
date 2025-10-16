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
 * @brief Creates a new shared memory block
 *
 * @param name The name of the block
 * @param size The size of the shared memory region
 */
SharedMemory::SharedMemory(const string& name, size_t size)
: Resource(name, size, syscore::ResourceType::SHARED_MEMORY),
  m_size(size),
  name(name)
{

	m_physical_address = (uintptr_t) PhysicalMemoryManager::s_current_manager->allocate_area(0, size);
}

SharedMemory::~SharedMemory() = default;

/**
 * @brief Get the address that the current process has this shared memory mapped to
 *
 * @param buffer Not used
 * @param size Not used
 * @param flags Not used
 * @return The virtual address in the current process's address space or 0 if not found
 */
int SharedMemory::read(void* buffer, size_t size, size_t flags) {

	// Process hasn't opened the resource
	auto it = m_mappings.find(GlobalScheduler::current_process()->pid());
	if(it == m_mappings.end())
		return 0;

	return it->second;
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

/**
 * @brief Map the shared memory into the address space of the owning process
 *
 * @param flags Not used for this resource
 */
void SharedMemory::open(size_t flags) {

	// Process has already opened this memory
	auto it = m_mappings.find(GlobalScheduler::current_process()->pid());
	if(it != m_mappings.end())
		return;

	auto virtual_address = (uintptr_t)GlobalScheduler::current_process()->memory_manager->vmm()->load_shared_memory(m_physical_address, m_size);
	m_mappings.insert(GlobalScheduler::current_process()->pid(), virtual_address);

}

/**
 * @brief Frees the page containing the shared memory
 *
 * @param flags Not used for this resource
 */
void SharedMemory::close(size_t flags) {

	PhysicalMemoryManager::s_current_manager->free_area(m_physical_address, m_size);

}

SharedMessageEndpoint::SharedMessageEndpoint(const string& name, size_t size, resource_type_t type)
: Resource(name, size, type)
{

}

SharedMessageEndpoint::~SharedMessageEndpoint() {

	// Free the messages
	for(auto& message : m_queue)
		delete message;
}

// TODO: Add a min() max() to common somewhere

/**
 * @brief Reads the first message from the endpoint or will yield until a message has been written
 *
 * @param buffer Where to write the message to
 * @param size Max size of the message to be read
 * @param flags Unused
 * @return The amount of bytes read
 */
int SharedMessageEndpoint::read(void* buffer, size_t size, size_t flags) {

	// Wait for a message
	if(m_queue.empty())
		return -1 * (int)resource_error_base_t::SHOULD_BLOCK;

	// Read the message into the buffer
	buffer_t* message = m_queue.pop_front();
	size_t readable = size > message->capacity() ? message->capacity() : size;
	memcpy(buffer, message -> raw(), readable);

	return readable;
}

/**
 * @brief Handles writing to the message endpoint resource by writing the buffer as a message
 *
 * @param buffer The message to write to the endpoint
 * @param size The size of the message
 * @param flags Unused
 * @return The amount of bytes written
 */
int SharedMessageEndpoint::write(void const* buffer, size_t size, size_t flags) {

	m_message_lock.lock();

	// Create the message
	auto* new_message = new buffer_t(size);
	new_message->copy_from(buffer, size);
	m_queue.push_back(new_message);

	m_message_lock.unlock();
	return size;
}