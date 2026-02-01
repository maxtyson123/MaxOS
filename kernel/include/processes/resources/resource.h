/**
 * @file resource.h
 * @brief Defines classes for managing process resources such as files and devices
 *
 * @date 26th August 2025
 * @author Max Tyson
 */

#ifndef MAXOS_PROCESSES_RESOURCES_RESOURCE_H
#define MAXOS_PROCESSES_RESOURCES_RESOURCE_H

#include <cstddef>
#include <map.h>
#include <vector.h>
#include <string.h>
#include <common/logger.h>
#include <syscalls.h>
#include <mem.h>
#include <libkpi/include/resource.h>

namespace MaxOS::processes::resources {

	typedef KPI::ResourceType resource_type_t;	///< Alias to make the libsyscore ResourceType accessible here
	typedef KPI::Resource Resource;				///< Alias to make the libsyscore Resource accessible here

	/**
	 * @class BaseResourceRegistry
	 * @brief Manages the creation, retention and destruction of resources of a certain type. Should be subclassed for each resource type
	 */
	class BaseResourceRegistry {

		protected:
			common::Map<string, Resource*> m_resources;         ///< The map of resource names to resource instances
			common::Map<string, uint64_t> m_resource_uses;      ///< The map of resource names to how many processes are using them

			resource_type_t m_type;                             ///< The resource type that this registry manages

		public:
			explicit BaseResourceRegistry(resource_type_t type);
			~BaseResourceRegistry();

			resource_type_t type();

			bool register_resource(Resource* resource);
			void close_resource(Resource* resource, size_t flags);

			virtual Resource* get_resource(const string& name);
			virtual Resource* create_resource(const string& name, size_t flags, uintptr_t data);
	};

	/**
	 * @class ResourceRegistry
	 * @brief A resource registry for a specific resource type
	 *
	 * @tparam Type The resource type that this registry manages
	 */
	template<class Type> class ResourceRegistry : public BaseResourceRegistry {

		public:
			explicit ResourceRegistry(resource_type_t type);
			~ResourceRegistry() = default;

			/**
			 * @brief Create a new resource of the specific type
			 *
			 * @param name The name of the resource
			 * @param flags The flags to open the resource with
			 * @return The created resource, or nullptr on failure
			 */
			Resource* create_resource(const string& name, size_t flags, uintptr_t data) override {

				auto resource = new Type(name, flags, type());

				// Creation failed
				if(!register_resource(resource)) {
					delete resource;
					return nullptr;
				}

				return resource;
			}
	};

	/**
	 * @brief Constructor for ResourceRegistry of a specific type
	 *
	 * @tparam Type The resource type that this registry manages
	 * @param type The resource type enum value
	 */
	template<class Type> ResourceRegistry<Type>::ResourceRegistry(resource_type_t type)
			:BaseResourceRegistry(type) { }

	/**
	 * @class GlobalResourceRegistry
	 * @brief Manages all the resource registries for each resource type
	 */
	class GlobalResourceRegistry {

		private:
			common::Map<resource_type_t, BaseResourceRegistry*> m_registries;
			inline static GlobalResourceRegistry* s_current;

		public:
			GlobalResourceRegistry();
			~GlobalResourceRegistry();

			static BaseResourceRegistry* get_registry(resource_type_t type);

			static void add_registry(resource_type_t type, BaseResourceRegistry* registry);
			static void remove_registry(BaseResourceRegistry* registry);
	};

	/**
	 * @class ResourceManager
	 * @brief Manages the open resources for a process
	 */
	class ResourceManager {

		private:
			common::Map<uint64_t, Resource*> m_resources;

			uint64_t m_next_handle = 1;

		public:
			ResourceManager();
			~ResourceManager();

			common::Map<uint64_t, Resource*> resources();

			uint64_t open_resource(resource_type_t type, const string& name, size_t flags);
			void close_resource(uint64_t handle, size_t flags);

			Resource* get_resource(uint64_t handle);
			Resource* get_resource(const string& name);
	};
}


#endif //MAXOS_PROCESSES_RESOURCES_RESOURCE_H
