/**
 * @file resource.h
 * @brief Implements the class ResourceServer to allow for a userspace process to provide resources to other processes
 *
 * @date 21 January 2026
 * @author Max Tyson
 *
 * @todo errors
 */

#include <resource.h>
#include <syscalls.h>
#include <ipc/sharedmemory.h>
#include <processes/thread.h>

using namespace MaxOS;
using namespace MaxOS::KPI;
using namespace MaxOS::KPI::ipc;
using namespace MaxOS::KPI::processes;


/**
 * @brief Constructs a new Resource object
 *
 * @param name The name of the resource
 * @param flags The flags for the resource (unused by default but resource type specific)
 * @param type The type of the resource
 */
Resource::Resource(const string& name, size_t flags, ResourceType type)
: m_name(name),
  m_type(type)
{

}

Resource::~Resource() = default;

/**
 * @brief Opens the resource
 *
 * @param flags Optional flags to pass (unused by default but resource type specific)
 */
void Resource::open(size_t flags) {

}


/**
 * @brief Closes the resource
 *
 * @param flags Optional flags to pass (unused by default but resource type specific)
 */
void Resource::close(size_t flags) {

}

/**
 * @brief read a certain amount of bytes from a resource
 *
 * @param buffer The buffer to read into
 * @param size How many bytes to read
 * @param flags Optional flags to pass (unused by default but resource type specific)
 * @return How many bytes were successfully read (negative can be used as errors)
 */
int Resource::read(void* buffer, size_t size, size_t flags) {
    return 0;
}

/**
 * @brief write a certain amount of bytes to a resource
 *
 * @param buffer The buffer to read from
 * @param size How many bytes to write
 * @param flags Optional flags to pass (unused by default but resource type specific)
 * @return How many bytes were successfully written
 */
int Resource::write(void const* buffer, size_t size, size_t flags) {

    return 0;
}

/**
 * @brief Gets the name of this resource
 *
 * @return The name
 */
string Resource::name() {
    return m_name;
}

/**
 * @brief Gets the type of this resource
 *
 * @return The type
 */
ResourceType Resource::type() {

    return m_type;
}

ResourceServer::ResourceServer(string server_name, size_t resource_id)
: m_server_name(server_name)
{

    // Cant use a kernel resource id
    if (resource_id <= (size_t)ResourceType::_END)
        return;

    // Create the shared memory
    string shared_name = (m_server_name + "_resource_server");
    m_shared_region = create_shared_memory(shared_name.c_str(), SERVICE_SHARED_MEM_SIZE);
    if (!m_shared_region)
        return;

    // Register the server
    resource_registry_create(shared_name.c_str(), resource_id);

    // Load the regions
    m_message_ring = (service_message_ring_t*)m_shared_region;
    m_data_region  = (void*)m_shared_region + MESSAGE_SIZE;

}

ResourceServer::~ResourceServer() {

    // If the region was created destroy it
    if (m_shared_region)
        delete m_shared_region;
}

/**
 * @brief Check if there are any messages waiting to be handled in the message ring buffer
 *
 * @return True if there are no messages, false otherwise
 */
bool ResourceServer::queue_empty() {

    // Get the queue indexes
    size_t head = __atomic_load_n(&m_message_ring->head, __ATOMIC_ACQUIRE);
    size_t tail = __atomic_load_n(&m_message_ring->tail, __ATOMIC_ACQUIRE);

    return head == tail;

}

/**
 * @brief Get the message at the front of the queue
 *
 * @return The first message on the queue or nullptr if the queue is empty
 */
service_resource_message_t* ResourceServer::peek_front() {

    // Get the queue indexes
    size_t tail = __atomic_load_n(&m_message_ring->tail, __ATOMIC_RELAXED);
    size_t head = __atomic_load_n(&m_message_ring->head, __ATOMIC_ACQUIRE);

    // Queue empty
    if (tail == head)
        return nullptr;

   return &m_message_ring->ring_buffer[tail];
}

/**
 * @brief Consume the message at the front of the queue
 *
 * @return The first message on the queue or nullptr if the queue is empty
 */
service_resource_message_t* ResourceServer::dequeue_front() {

    // Get the queue indexes
    size_t tail = __atomic_load_n(&m_message_ring->tail, __ATOMIC_RELAXED);
    size_t head = __atomic_load_n(&m_message_ring->head, __ATOMIC_ACQUIRE);

    // Queue empty
    if (tail == head)
        return nullptr;

    // Get current message
    service_resource_message_t* msg = &m_message_ring->ring_buffer[tail];

    // Advance tail
    size_t next = (tail + 1) % MESSAGE_SLOTS;
    __atomic_store_n(&m_message_ring->tail, next, __ATOMIC_RELEASE);

    return msg;
}

/**
 * @brief Move the start of the queue to the next item
 */
void ResourceServer::advance_queue() {

    // Get the queue indexes
    size_t tail = __atomic_load_n(&m_message_ring->tail, __ATOMIC_RELAXED);
    size_t head = __atomic_load_n(&m_message_ring->head, __ATOMIC_ACQUIRE);

    // Advance tail
    size_t next = (tail + 1) % MESSAGE_SLOTS;
    __atomic_store_n(&m_message_ring->tail, next, __ATOMIC_RELEASE);

}

void ResourceServer::process_message(service_resource_message_t *message) {

    // Parse the message
    auto data = m_data_region + message->data_offset;
    auto resource   = m_resource_map[message->resource_id];

    // Ensure the resource exists
    bool should_exist = !(message->command == ServiceResourceCommand::S_CREATE
                       || message->command == ServiceResourceCommand::S_GET);
    if (!resource && !should_exist) {
        message->state = ServiceMessageSlotState::RESPONSE;
        message->response = -1;
        return;
    }

    switch (message->command) {

        case ServiceResourceCommand::S_CREATE: {

            // Try to create the resource
            resource = create_resource((char*)data, message->flags);
            if (!resource)
                message->response = false;

            // Store the resource
            m_resource_map.insert(message->resource_id, resource);
            break;
        }

        case ServiceResourceCommand::S_GET: {

            // Try to create the resource
            resource = get_resource((char*)data);
            if (!resource)
                message->response = false;

            // Store the resource
            m_resource_map.insert(message->resource_id, resource);
            break;
        }

        case ServiceResourceCommand::R_OPEN: {

            // Delegate
            resource->open(message->flags);
            break;

        }

        case ServiceResourceCommand::R_CLOSE: {

            // Delegate
            resource->close(message->flags);
            break;

        }

        case ServiceResourceCommand::R_WRITE: {

            // Delegate
            message->response = resource->write(data, message->data_size, message->flags);
            break;

        }

        case ServiceResourceCommand::R_READ: {

            // Delegate
            message->response = resource->read(data, message->data_size, message->flags);
            break;

        }

        default:
            message->response = false;
    }

    // Done handling
    message->state = ServiceMessageSlotState::RESPONSE;
}

void ResourceServer::loop() {

    while (true) {

        // Nothing to process
        if (queue_empty()) {
            yield();
            continue;
        }

        // Proccess the message that has been waiting the longest
        auto message = peek_front();
        process_message(message);

        // Move to the next message
        advance_queue();

    }
}

Resource* ResourceServer::get_resource(const string &name) {

    return nullptr;
}

Resource * ResourceServer::create_resource(const string &name, size_t flags) {
    return nullptr;
}
