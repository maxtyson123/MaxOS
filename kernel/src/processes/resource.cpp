//
// Created by 98max on 8/26/2025.
//
#include <processes/resource.h>

using namespace MaxOS;
using namespace MaxOS::processes;

Resource::Resource(const string& name, size_t flags, ResourceType type)
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
 * @param flags Optional flags to pass (unused by default but resource type specific)
 * @return How many bytes were successfully read
 */
size_t Resource::read(void* buffer, size_t size, size_t flags) {
	return 0;
}

/**
 * @brief Write a certain amount of bytes to a resource
 *
 * @param buffer The buffer to read from
 * @param size How many bytes to write
 * @param flags Optional flags to pass (unused by default but resource type specific)
 * @return How many bytes were successfully written
 */
size_t Resource::write(void const* buffer, size_t size, size_t flags) {

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

BaseResourceRegistry::BaseResourceRegistry(ResourceType type)
: m_type(type)
{
	GlobalResourceRegistry::add_registry(type, this);
}

BaseResourceRegistry::~BaseResourceRegistry(){

	GlobalResourceRegistry::remove_registry(this);
}

/**
 * @brief Gets the type of resources this registry handles
 *
 * @return The type
 */
ResourceType BaseResourceRegistry::type() {

	return m_type;
}

/**
 * @brief Gets a resource from the registry by name
 *
 * @param name The name of the resource to find
 * @return The resource or nullptr if the resource was not found
 */
Resource* BaseResourceRegistry::get_resource(string const& name) {

	// Resource isn't stored in this registry
	if(m_resources.find(name) == m_resources.end())
		return nullptr;

	// Increment the use count of the resource and return it
	m_resource_uses[name]++;
	return m_resources[name];

}

/**
 * @brief Registers a resource in the registry
 *
 * @param resource The resource to store
 * @param name The name of the resource (must not already be in use)
 * @return True if the register was successful, false if not
 */
bool BaseResourceRegistry::register_resource(Resource* resource) {

	// Resource name is already stored in this registry
	if(m_resources.find(resource->name()) != m_resources.end())
		return false;

	m_resources.insert(resource->name(), resource);
	m_resource_uses.insert(resource->name(), 0);
	return true;
}


/**
 * @brief Close the resource provided, if it exists in this registry
 *
 * @param resource The resource to close
 * @param flags Optional flags to pass (unused by default but registry type specific)
 */
void BaseResourceRegistry::close_resource(Resource* resource, size_t flags) {

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
 * @brief Try to create a new resource with the specified name, will fail if the name is in use
 *
 * @param name The name of the resource to create
 * @param flags Optional flags to pass (unused by default but registry type specific)
 * @return The resource created or nullptr if failed to create the resource
 */
Resource* BaseResourceRegistry::create_resource(string const& name, size_t flags) {
	return nullptr;
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
BaseResourceRegistry* GlobalResourceRegistry::get_registry(ResourceType type) {

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
void GlobalResourceRegistry::add_registry(ResourceType type, BaseResourceRegistry* registry) {

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
void GlobalResourceRegistry::remove_registry(BaseResourceRegistry* registry) {

	// Does it already exist?
	if(s_current->m_registries.find(registry->type()) == s_current->m_registries.end())
		return;

	s_current->m_registries.erase(registry->type());

}

ResourceManager::ResourceManager() = default;

ResourceManager::~ResourceManager(){

	// Collect all resources (as closing will break iteration)
	common::Vector<uint64_t> handles;
	for (const auto& kv : m_resources)
		handles.push_back(kv.first);

	// Close the resources
	for (auto h : handles)
		close_resource(h, 0);

};

/**
 * @brief Get the resources currently open
 *
 * @return The resources
 */
common::Map<uint64_t, Resource*> ResourceManager::resources() {

	return m_resources;
}

/**
 * @brief Registers a resource with the resource manager and then opens it
 *
 * @param resource The resource to register
 * @param name The name of the resource
 * @return The handle id of the resource or 0 if failed
 */
uint64_t ResourceManager::open_resource(ResourceType type, string const& name, size_t flags) {

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
 *
 * @param handle The handle number of the resource
 * @param flags Flags to pass to the closing
 */
void ResourceManager::close_resource(uint64_t handle, size_t flags) {

	auto it = m_resources.find(handle);
	if(it == m_resources.end())
		return;

	// Remove it
	Resource* resource = it->second;
	m_resources.erase(handle);

	// Close it
	GlobalResourceRegistry::get_registry(resource->type()) -> close_resource(resource, flags);
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


