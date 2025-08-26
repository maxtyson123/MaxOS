//
// Created by 98max on 8/26/2025.
//
#include <processes/resource.h>

using namespace MaxOS;
using namespace MaxOS::processes;

Resource::Resource(const string& name, ResourceType type)
: m_name(name),
  m_type(type)
{

}

Resource::~Resource() = default;

/**
 * @brief Opens the resource
 */
void Resource::open() {

}


/**
 * @brief Closes the resource
 */
void Resource::close() {

}

/**
 * @brief Read a certain amount of bytes from a resource
 *
 * @param buffer The buffer to read into
 * @param size How many bytes to read
 * @return How many bytes were successfully read
 */
size_t Resource::read(void* buffer, size_t size) {
	return 0;
}

/**
 * @brief Write a certain amount of bytes to a resource
 *
 * @param buffer The buffer to read from
 * @param size How many bytes to write
 * @return How many bytes were successfully written
 */
size_t Resource::write(void const* buffer, size_t size) {

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
 * @return Tje type
 */
ResourceType Resource::type() {

	return m_type;
}

ResourceRegistry::ResourceRegistry(ResourceType type)
: m_type(type)
{

	GlobalResourceRegistry::add_registry(type, this);

}

ResourceRegistry::~ResourceRegistry(){

	GlobalResourceRegistry::remove_registry(this);
}

/**
 * @brief Gets the type of resources this registry handles
 *
 * @return The type
 */
ResourceType ResourceRegistry::type() {

	return m_type;
}

/**
 * @brief Gets a resource from the registry by name
 *
 * @param name The name of the resource to find
 * @return The resource or nullptr if the resource was not found
 */
Resource* ResourceRegistry::get_resource(string const& name) {

	// Resource isn't stored in this registry
	if(m_resources.find(name) == m_resources.end())
		return nullptr;

	// Increment the use count of the resource and return it
	m_resource_uses[name]++;
	return m_resources[name];

}

/**
 * @brief Close the resource provided, if it exists in this registry
 *
 * @param resource The resource to close
 */
void ResourceRegistry::close_resource(Resource* resource) {

	// Resource isn't stored in this registry
	if(m_resources.find(resource->name()) == m_resources.end())
		return;

	m_resource_uses[resource->name()]--;

	// Don't close a resource that has more processes using it
	if(m_resource_uses[resource->name()])
		return;

	// Can safely close the resource
	resource->close();
	m_resources.erase(resource->name());
	m_resource_uses.erase(resource->name());
	delete resource;
}

/**
 * @brief Registers a resource in the registry
 *
 * @param resource The resource to store
 * @param name The name of the resource (must not already be in use)
 */
void ResourceRegistry::register_resource(Resource* resource, const string& name) {

	// Resource name is already stored in this registry
	if(m_resources.find(resource->name()) != m_resources.end())
		return;

	m_resources.insert(name, resource);
	m_resource_uses.insert(name, 0);
}

GlobalResourceRegistry::GlobalResourceRegistry() {
	s_current = this;

}

GlobalResourceRegistry::~GlobalResourceRegistry() {
	if(s_current == this)
		s_current = nullptr;

}

/**
 * @brief Gets a registry of a type
 *
 * @param type The type of registry to get
 * @return The registry or nullptr if not found
 */
ResourceRegistry* GlobalResourceRegistry::get_registry(ResourceType type) {

	auto registry = s_current->m_registries.find(type);
	if(registry == s_current->m_registries.end())
		return nullptr;

	return registry->second;
}

/**
 * @brief Adds a registry to the global list if there is not already one for that type
 *
 * @param type The type of registry being added
 * @param registry The registry to add
 */
void GlobalResourceRegistry::add_registry(ResourceType type, ResourceRegistry* registry) {

	// Does it already exist?
	if(s_current->m_registries.find(type) != s_current->m_registries.end())
		return;

	s_current->m_registries.insert(type, registry);
}

/**
 * @brief Adds a registry to the global list if there is not already one for that type
 *
 * @param type The type of registry being added
 * @param registry The registry to add
 */
void GlobalResourceRegistry::remove_registry(ResourceRegistry* registry) {

	// Does it already exist?
	if(s_current->m_registries.find(registry->type()) != s_current->m_registries.end())
		return;

	s_current->m_registries.erase(registry->type());

}

ResourceManager::ResourceManager() = default;

ResourceManager::~ResourceManager(){

	// Close all resources
	for(const auto& resource : m_resources)
		close_resource(resource.first);

};

/**
 * @brief Registers a resource with the resource manager and then opens it
 *
 * @param resource The resource to register
 * @param name The name of the resource
 * @return The handle id of the resource or 0 if failed
 */
uint64_t ResourceManager::open_resource(ResourceType type, string const& name) {

	// Get the resource
	auto resource = GlobalResourceRegistry::get_registry(type) -> get_resource(name);
	if(!resource)
		return 0;

	// Store it
	m_resources.insert(m_next_handle, resource);

	// Open it
	resource->open();
	return m_next_handle++;
}

/**
 * @brief Un registers the resource and then closes it
 * @param handle
 */
void ResourceManager::close_resource(uint64_t handle) {

	Resource* resource = m_resources[handle];

	// Remove it
	m_resources.erase(handle);

	// Close it
	GlobalResourceRegistry::get_registry(resource->type()) -> close_resource(resource);
}

/**
 * @brief Gets a resource based on the handle id
 *
 * @param handle The handle number of the resource
 * @return The resource or nullptr if not found
 */
Resource* ResourceManager::get_resource(uint64_t handle) {

	if(m_resources.find(handle) == m_resources.end())
		return nullptr;

	return m_resources[handle];
}

/**
 * @brief Finds a resource by name
 *
 * @param name The name of the resource
 * @return The resource or nullptr if not found
 */
Resource* ResourceManager::get_resource(string const& name) {

	for(const auto& resource : m_resources)
		if(resource.second->name() == name)
			return resource.second;

	return nullptr;
}