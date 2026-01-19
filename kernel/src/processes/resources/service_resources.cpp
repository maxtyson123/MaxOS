/**
 * @file service_resources.cpp
 * @brief Implements scheduler resource classes for wrapping userspace service Resources in the Global Scheduler, allowing operations to be performed through the Resource interface.
 *
 * @date 18th January 2026
 * @author Max Tyson
 */

#include <processes/resources/service_resources.h>

#include "processes/ipc.h"

using namespace MaxOS;
using namespace MaxOS::processes;
using namespace MaxOS::processes::resources;

ServiceResource::ServiceResource(const string &name, size_t flags, resource_type_t type)
: Resource(name, flags, type)
{
}

ServiceResource::~ServiceResource() = default;

/**
 * @copydoc Resource::open
 */
void ServiceResource::open(size_t flags) {
    send_to_service(ServiceResourceCommand::R_OPEN, 0,0, flags);
}

/**
 * @copydoc Resource::close
 */
void ServiceResource::close(size_t flags) {
    send_to_service(ServiceResourceCommand::R_CLOSE, 0,0, flags);
}

/**
 * @copydoc Resource::read
 */
int ServiceResource::read(void *buffer, size_t size, size_t flags) {
    send_to_service(ServiceResourceCommand::R_READ, buffer, size, flags);
}

/**
 * @copydoc Resource::write
 */
int ServiceResource::write(const void *buffer, size_t size, size_t flags) {
    send_to_service(ServiceResourceCommand::R_WRITE, buffer, size, flags);
}

/**
 * @brief Attempt to get a resource on a service endpoint that may not be registered yet
 *
 * @param endpoint The message endpoint to communicate with the endpoint on
 * @param id The id allocated for this resource
 * @return True if the resource can be fetched, False otherwise
 */
bool ServiceResource::get_on_service(string endpoint, size_t id) {

    // Store endpoint info
    m_endpoint = endpoint;
    m_id = id;

    // Let the service get the object on its end
    send_to_service(ServiceResourceCommand::S_GET, name().c_str(),name().length() + 1, 0);


}

/**
 * @brief Passes the creation of a resource to
 *
 * @param endpoint The message endpoint to communicate with the endpoint on
 * @param id The id allocated for this resource
 */
void ServiceResource::construct_on_service(string endpoint, size_t id) {

    // Store endpoint info
    m_endpoint = endpoint;
    m_id = id;

    // Let the service construct the object on its end
    send_to_service(ServiceResourceCommand::S_CREATE, name().c_str(),name().length() + 1, 0);

}

/**
 * @brief Send a command and its associated data to the endpoint associated with this resource
 *
 * @param command The command to execute
 * @param buffer The data to pass along with the command
 * @param size The size of the data buffer
 * @param flags Flags associated with the command
 */
void ServiceResource::send_to_service(ServiceResourceCommand command, const void *buffer, size_t size, size_t flags) {

    // Construct the header
    service_resource_message_t header {
        .resource_id    = m_id,
        .flags          =  flags,
        .command        = (size_t)command
    };

    // Get the endpoint
    auto endpoint = GlobalResourceRegistry::get_registry(KPI::ResourceType::MESSAGE_ENDPOINT)->get_resource(m_endpoint);
    auto message_queue     = (SharedMessageEndpoint*)endpoint;
    if (!message_queue)
        return;

    // Send the message
    ipc_iovec_t vec[2] = {
        { &header, sizeof(header) },
       { buffer,  size }
    };
    message_queue->send(vec,2);

}


ServiceResourceRegistry::ServiceResourceRegistry(string endpoint, size_t resource_id)
: ResourceRegistry<ServiceResource>((resource_type_t)resource_id),
  m_endpoint(endpoint),
  m_next_id(0)
{

}

ServiceResourceRegistry::~ServiceResourceRegistry() = default;

Resource* ServiceResourceRegistry::get_resource(const string &name) {

    // Resource already opened
    auto resource = BaseResourceRegistry::get_resource(name);
    if(resource != nullptr)
        return resource;

    // Open on the server
    auto attempt_resource = new ServiceResource(name, 0, m_type);
    if (attempt_resource->get_on_service(m_endpoint, m_next_id++)) {
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
    resource -> construct_on_service(m_endpoint, m_next_id++);
    return resource;
}