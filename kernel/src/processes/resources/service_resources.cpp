/**
 * @file service_resources.cpp
 * @brief Implements scheduler resource classes for wrapping userspace service Resources in the Global Scheduler, allowing operations to be performed through the Resource interface.
 *
 * @date 18th January 2026
 * @author Max Tyson
 */

#include <processes/resources/service_resources.h>

using namespace MaxOS;
using namespace MaxOS::processes;
using namespace MaxOS::processes::resources;
using namespace MaxOS::common;

/**
 * @brief Constructs a handler to manage resource commands over shared memory
 */
ServiceHandler::ServiceHandler() {


    // Setup the shared memory
    size_t size = sizeof(service_message_ring_t) + DATA_SIZE;
    uintptr_t address = 0;
    m_data_region = address + sizeof(service_message_ring_t);

    setup_region(m_data_region, DATA_SIZE);
}

ServiceHandler::~ServiceHandler() = default;

/**
 * @brief Allocates space in the shared memory region for a resource data buffer and copies it into that space
 *
 * @param buffer The buffer that stores the data
 * @param size The amount of data to store
 * @param reserve_only If true, dont copy the data in the buffer just reserve the space for it
 * @return The offset from the start of shared data region
 */
size_t ServiceHandler::store_data(const void *buffer, size_t size, bool reserve_only) {

    // Setup the storage
    auto data = handle_malloc(size);
    if (!reserve_only)
        memcpy(data, buffer, size);

    return (size_t)data - m_data_region;

}

/**
 * @brief Wait until a free spot for a message is available and then claim it
 *
 * @return
 */
service_resource_message_t* ServiceHandler::aquire_slot() {

    // Find a spot
    while (true) {

        // Get the data
        size_t head = m_message_ring->head;
        size_t tail = m_message_ring->tail;

        // No spot available
        if (((head + 1) % MESSAGE_SLOTS) == tail) {
            block();
            continue;
        }

        // Claim slot
        size_t slot = head;
        m_message_ring->head = (head + 1) % MESSAGE_SLOTS;

        return &m_message_ring->ring_buffer[slot];
    }
}

/**
 * @brief Allocates an ID to identify a resource on the service
 *
 * @return The ID that the resource can use
 */
size_t ServiceHandler::allocate_id() {
    return m_next_id++;
}


/**
 * @brief Send a command and its associated data to the endpoint associated with this resource
 *
 * @param id The id of the resource
 * @param command The command to execute
 * @param buffer The data to pass along with the command
 * @param size The size of the data buffer
 * @param flags Flags associated with the command
 *
 * @todo If data is too big should do page mapping
 */
int64_t ServiceHandler::send_to_service(size_t id, ServiceResourceCommand command, const void *buffer, size_t size, size_t flags) {

    // Copy data into shared region
    bool is_read = command == ServiceResourceCommand::R_READ; // || command == ServiceResourceCommand::R_READ_ATTR;
    size_t offset = store_data(buffer, size, is_read);

    // Construct the message
    auto slot = aquire_slot();
    slot -> resource_id = id;
    slot -> flags = flags;
    slot -> command = (size_t)command;
    slot -> data_size = size;
    slot -> data_offset = offset;
    slot -> response = 0;
    slot -> state = ServiceMessageSlotState::REQUEST;

    // Wait for a response
    while (slot -> state == ServiceMessageSlotState::REQUEST)
        block();

    // Read request
    if (is_read)
        memcpy((void*)buffer, (void*)m_data_region + offset, size);

    return slot->response;
}


ServiceResource::ServiceResource(const string &name, size_t flags, resource_type_t type)
: Resource(name, flags, type)
{
}

ServiceResource::~ServiceResource() = default;

/**
 * @copydoc Resource::open
 */
void ServiceResource::open(size_t flags) {
     m_handler -> send_to_service(m_id, ServiceResourceCommand::R_OPEN, 0,0, flags);
}

/**
 * @copydoc Resource::close
 */
void ServiceResource::close(size_t flags) {
     m_handler -> send_to_service(m_id, ServiceResourceCommand::R_CLOSE, 0,0, flags);
}

/**
 * @copydoc Resource::read
 */
int ServiceResource::read(void *buffer, size_t size, size_t flags) {
    return m_handler -> send_to_service(m_id, ServiceResourceCommand::R_READ, buffer, size, flags);
}

/**
 * @copydoc Resource::write
 */
int ServiceResource::write(const void *buffer, size_t size, size_t flags) {
    return m_handler -> send_to_service(m_id, ServiceResourceCommand::R_WRITE, buffer, size, flags);
}

void ServiceResource::attach_wrapper(ServiceHandler *wrapper) {

    m_handler = wrapper;
    m_id = wrapper->allocate_id();

}

/**
 * @brief Attempt to get a resource on a service endpoint that may not be registered yet
 *
 * @return True if the resource can be fetched, False otherwise
 */
bool ServiceResource::get_on_service() {

   // Let the service get the object on its end
   return m_handler -> send_to_service(m_id, ServiceResourceCommand::S_GET, name().c_str(),name().length() + 1, 0);


}

/**
 * @brief Passes the creation of a resource to the service
 */
void ServiceResource::construct_on_service() {

    // Let the service construct the object on its end
    m_handler -> send_to_service(m_id, ServiceResourceCommand::S_CREATE, name().c_str(),name().length() + 1, 0);

}


ServiceResourceRegistry::ServiceResourceRegistry(size_t resource_id)
: ResourceRegistry<ServiceResource>((resource_type_t)resource_id)
{

}

ServiceResourceRegistry::~ServiceResourceRegistry() = default;

Resource* ServiceResourceRegistry::get_resource(const string &name) {

    // Resource already opened
    auto resource = BaseResourceRegistry::get_resource(name);
    if(resource != nullptr)
        return resource;

    // Setup a temporary resource
    auto attempt_resource = new ServiceResource(name, 0, m_type);
    attempt_resource->attach_wrapper(&m_handler);

    // Try open on the server
    if (attempt_resource->get_on_service()) {
        register_resource(attempt_resource);
        return attempt_resource;
    }

    // Must have failed to get on server
    delete attempt_resource;
    return nullptr;
}

/**
 * @brief Create a new resource on the service
 *
 * @param name The name of the resource
 * @param flags The flags to pass
 * @return The new resource
 */
Resource* ServiceResourceRegistry::create_resource(const string &name, size_t flags) {

    // Create the resource
    auto resource = new ServiceResource(name, flags, m_type);

    // Creation failed
    if(!register_resource(resource)) {
        delete resource;
        return nullptr;
    }

    // Handoff setup to service
    resource->attach_wrapper(&m_handler);
    resource->construct_on_service();
    return resource;
}