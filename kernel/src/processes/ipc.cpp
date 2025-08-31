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
 */
SharedMemory::SharedMemory(const string& name, size_t size, ResourceType type)
: Resource(name, size, type),
  m_size(size),
  name(name)
{

	m_physical_address = (uintptr_t) PhysicalMemoryManager::s_current_manager->allocate_area(0, size);
	m_owner_pid = Scheduler::current_process()->pid();
}

SharedMemory::~SharedMemory() {

	PhysicalMemoryManager::s_current_manager->free_area(m_physical_address, m_size);
}

size_t SharedMemory::read(void* buffer, size_t size, size_t flags) {

	// Reading gets the address
	if(size < sizeof(m_physical_address))
		return 0;

	memcpy(buffer, &m_physical_address, sizeof(m_physical_address));
	return sizeof(m_physical_address);
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

SharedMessageEndpoint::SharedMessageEndpoint(const string& name, size_t size, ResourceType type)
: Resource(name, size, type)
{

}

SharedMessageEndpoint::~SharedMessageEndpoint() {

	// Free the messages
	for(auto& message : m_queue)
		delete message;
}

/**
 * @brief Queues a message to be processed by the endpoint (buffer can be freed as it is copied in to the receiving process's memory)
 *
 * @param message The message to queue
 * @param size The size of the message
 */
void SharedMessageEndpoint::queue_message(void const* message, size_t size) {

	m_message_lock.lock();

	// Create the message
	auto* new_message = new buffer_t(size);
	new_message->copy_from(message, size);
	m_queue.push_back(new_message);

	m_message_lock.unlock();
}

// TODO: Add a min() max() to common somewhere

/**
 * @brief Reads the first message from the endpoint or will yield until a message has been written
 *
 * @param buffer Where to write the message to
 * @param size Max size of the message to be read
 * @return The amount of bytes read
 */
size_t SharedMessageEndpoint::read(void* buffer, size_t size, size_t flags) {

	// Wait for a message
	while(m_queue.empty())
		Scheduler::system_scheduler()->yield();

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
 * @return The amount of bytes written
 */
size_t SharedMessageEndpoint::write(void const* buffer, size_t size, size_t flags) {

	queue_message(buffer, size);
	return size;
}