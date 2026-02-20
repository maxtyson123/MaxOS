/**
 * @file resource_bridge.cpp
 * @brief Implements scheduler resource classes for wrapping userspace bridge Resources in the Global Scheduler, allowing operations to be performed through the Resource interface.
 *
 * @date 18th January 2026
 * @author Max Tyson
 */

#include <processes/resources/resource_bridge.h>

using namespace MaxOS;
using namespace MaxOS::processes;
using namespace MaxOS::processes::resources;
using namespace MaxOS::common;
using namespace MaxOS::memory;
using namespace MaxOS::KPI;

/**
 * @brief Constructs a handler to manage resource commands over shared memory
 */
BridgeHandler::BridgeHandler(string const& shared_name, Process* owner_process)
: m_owner_process(owner_process)
{

    // Get the shared memory region
    auto shared_memory = (SharedMemory*)GlobalResourceRegistry::get_registry(resource_type_t::SHARED_MEMORY)->get_resource(shared_name);
    ASSERT(shared_memory != nullptr, "Shared memory resource not found\n");

    // Copy into the kernel
    ASSERT(shared_memory->size() == SERVICE_SHARED_MEM_SIZE, "Shared memory resource size mismatch\n");
    auto address = MemoryManager::s_kernel_memory_manager->vmm()->load_shared_memory(shared_memory->physical_address(), SERVICE_SHARED_MEM_SIZE);

    // Setup the message region
    m_message_ring = (service_message_ring_t*)address;

    // Setup the data region
    m_data_region = (uintptr_t)address + MESSAGE_SIZE;
    setup_region(m_data_region, DATA_SIZE);
}


BridgeHandler::~BridgeHandler() = default;

/**
 * @brief Allocates space in the shared memory region for a resource data buffer and copies it into that space, or maps
 * the pages into the resource server if the buffer is bigger than MAX_COPY_SIZE
 *
 * @param buffer The buffer that stores the data
 * @param size The amount of data to store
 * @param reserve_only If true, dont copy the data in the buffer just reserve the space for it
 * @return The offset from the start of shared data region, or the virtual address of the mapped range
 */
size_t BridgeHandler::store_data(const void *buffer, size_t size, bool reserve_only) {

    // Nothing to store
    if (!size)
        return 0;

    bool is_mapped = size >= MAX_COPY_SIZE;

    // Set up the storage
    auto data = is_mapped  ? m_owner_process->memory_manager->vmm()->load_range_from_process(GlobalScheduler::current_process()->memory_manager->vmm(), buffer, size)
                                : handle_malloc(size);
    ASSERT(data != nullptr, "Could not allocate memory for bridge\n");

    // Copy the data
    if (!reserve_only && !is_mapped)
        common::memcpy(data, buffer, size);

    // If not mapped then its an offset into the process's own virtual address for the shared mem
    return (size_t)data - (is_mapped ? 0 : m_data_region);

}
void block(){}

/**
 * @brief Wait until a free spot for a message is available and then claim it
 *
 * @return
 */
service_resource_message_t* BridgeHandler::aquire_slot() {

    // Find a spot
    while (true) {

        // Get the data
        size_t head = __atomic_load_n(&m_message_ring->head, __ATOMIC_RELAXED);
        size_t tail = __atomic_load_n(&m_message_ring->tail, __ATOMIC_ACQUIRE);

        // Try to claim a spot
        size_t next = (head + 1) % MESSAGE_SLOTS;
        if(!__atomic_compare_exchange_n(&m_message_ring->head, &head, next,true,__ATOMIC_ACQ_REL,__ATOMIC_RELAXED))
            continue;

        // Claim slot
        size_t slot = head;
        return &m_message_ring->ring_buffer[slot];
    }
}

/**
 * @brief Allocates an ID to identify a resource on the bridge
 *
 * @return The ID that the resource can use
 */
size_t BridgeHandler::allocate_id() {
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
int64_t BridgeHandler::send_to_bridge(size_t id, ServiceResourceCommand command, const void *buffer, size_t size, size_t flags) {

    // Copy data into shared region
    bool is_read = command == ServiceResourceCommand::R_READ; // || command == BridgeResourceCommand::R_READ_ATTR;
    bool is_page_mapped = size >= MAX_COPY_SIZE;
    size_t offset = store_data(buffer, size, is_read);

    // Construct the message
    auto slot = aquire_slot();
    __atomic_store_n(&slot->sending_pid,    GlobalScheduler::current_process()->pid(),  __ATOMIC_RELAXED);
    __atomic_store_n(&slot->resource_id,        id,                                         __ATOMIC_RELAXED);
    __atomic_store_n(&slot->flags,              flags,                                      __ATOMIC_RELAXED);
    __atomic_store_n(&slot->command,            (uint8_t)command,                           __ATOMIC_RELAXED);
    __atomic_store_n(&slot->data_size,          size,                                       __ATOMIC_RELAXED);
    __atomic_store_n(&slot->data_offset,        offset,                                     __ATOMIC_RELAXED);
    __atomic_store_n(&slot->data_page_mapped,   is_page_mapped,                             __ATOMIC_RELAXED);
    __atomic_store_n(&slot->response,       0,                                          __ATOMIC_RELAXED);
    __atomic_store_n(&slot->state,              (uint8_t)ServiceMessageSlotState::REQUEST,  __ATOMIC_RELEASE);

    // Wait for a response
    while (__atomic_load_n(&slot->state, __ATOMIC_ACQUIRE) == (uint8_t)ServiceMessageSlotState::REQUEST)
        GlobalScheduler::current_thread()->yield();

    // Read request
    if (is_read)
        common::memcpy((void*)buffer, is_page_mapped ? (void*)offset : (void*)m_data_region + offset, size);

    // Free resources
    is_page_mapped  ? m_owner_process->memory_manager->vmm()->unload_range_from_process((void*)offset, size)
                    : handle_free((void*)m_data_region + offset);
    __atomic_store_n(&slot->state, (uint8_t)ServiceMessageSlotState::FREE,__ATOMIC_RELEASE);

    return slot->response;
}


BridgeResource::BridgeResource(const string &name, size_t flags, resource_type_t type)
: Resource(name, flags, type)
{
}

BridgeResource::~BridgeResource() = default;

/**
 * @copydoc Resource::open
 */
void BridgeResource::open(size_t flags) {
     m_handler -> send_to_bridge(m_id, ServiceResourceCommand::R_OPEN, 0,0, flags);
}

/**
 * @copydoc Resource::close
 */
void BridgeResource::close(size_t flags) {
     m_handler -> send_to_bridge(m_id, ServiceResourceCommand::R_CLOSE, 0,0, flags);
}

/**
 * @copydoc Resource::read
 */
int BridgeResource::read(void *buffer, size_t size, size_t flags) {
    return m_handler -> send_to_bridge(m_id, ServiceResourceCommand::R_READ, buffer, size, flags);
}

/**
 * @copydoc Resource::write
 */
int BridgeResource::write(const void *buffer, size_t size, size_t flags) {
    return m_handler -> send_to_bridge(m_id, ServiceResourceCommand::R_WRITE, buffer, size, flags);
}

void BridgeResource::attach_wrapper(BridgeHandler *wrapper) {

    m_handler = wrapper;
    m_id = wrapper->allocate_id();

}

/**
 * @brief Attempt to get a resource on a bridge endpoint that may not be registered yet
 *
 * @return True if the resource can be fetched, False otherwise
 */
bool BridgeResource::get_on_bridge() {

   // Let the bridge get the object on its end
   return m_handler -> send_to_bridge(m_id, ServiceResourceCommand::S_GET, name().c_str(),name().length() + 1, 0);


}

/**
 * @brief Passes the creation of a resource to the bridge
 */
void BridgeResource::create_on_bridge() {

    // Let the bridge construct the object on its end
    m_handler -> send_to_bridge(m_id, ServiceResourceCommand::S_CREATE, name().c_str(),name().length() + 1, 0);

}


BridgeResourceRegistry::BridgeResourceRegistry(const string &shared_name, size_t resource_id)
: ResourceRegistry((resource_type_t)resource_id),
  m_handler(shared_name, GlobalScheduler::current_process())
{

}

BridgeResourceRegistry::~BridgeResourceRegistry() = default;

Resource* BridgeResourceRegistry::get_resource(const string &name) {

    // Resource already opened
    auto resource = BaseResourceRegistry::get_resource(name);
    if(resource != nullptr)
        return resource;

    // Setup a temporary resource
    auto attempt_resource = new BridgeResource(name, 0, m_type);
    attempt_resource->attach_wrapper(&m_handler);

    // Try open on the server
    if (attempt_resource->get_on_bridge()) {
        register_resource(attempt_resource);
        return attempt_resource;
    }

    // Must have failed to get on server
    delete attempt_resource;
    return nullptr;
}

/**
 * @brief Create a new resource on the bridge
 *
 * @param name The name of the resource
 * @param flags The flags to pass
 * @return The new resource
 */
Resource* BridgeResourceRegistry::create_resource(const string &name, size_t flags, uintptr_t data) {

    // Create the resource
    auto resource = new BridgeResource(name, flags, m_type);

    // Creation failed
    if(!register_resource(resource)) {
        delete resource;
        return nullptr;
    }

    // Handoff setup to bridge
    resource->attach_wrapper(&m_handler);
    resource->create_on_bridge();
    return resource;
}