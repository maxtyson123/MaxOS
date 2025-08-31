//
// Created by 98max on 8/26/2025.
//

#ifndef MAXOS_PROCESSES_RESOURCE_H
#define MAXOS_PROCESSES_RESOURCE_H

#include <stddef.h>
#include <common/map.h>
#include <common/vector.h>
#include <common/string.h>
#include <common/logger.h>

namespace MaxOS {
	namespace processes {

		enum class ResourceType{
			MESSAGE_ENDPOINT,
			SHARED_MEMORY
		};

		class Resource {

			private:

				string m_name;
				ResourceType m_type;

			public:

				Resource(const string& name, size_t flags, ResourceType type);
				virtual ~Resource();

				string name();
				ResourceType type();

				virtual void open();
				virtual void close();

				virtual size_t read(void* buffer, size_t size, size_t flags);
				virtual size_t write(const void* buffer, size_t size, size_t flags);
		};

		 class BaseResourceRegistry{

			private:
				common::Map<string, Resource*> m_resources;
				common::Map<string, uint64_t>  m_resource_uses;

				ResourceType m_type;

			public:
				explicit BaseResourceRegistry(ResourceType type);
				~BaseResourceRegistry();

				ResourceType type();

			 	virtual Resource* 	get_resource(const string& name);
			 	virtual bool 		register_resource(Resource* resource);

			 	virtual void 		close_resource(Resource* resource, size_t flags);
				virtual Resource* 	create_resource(const string& name, size_t flags);
		};

		template<class Type> class ResourceRegistry : public BaseResourceRegistry{

			public:
				explicit ResourceRegistry(ResourceType type);
				~ResourceRegistry() = default;

				Resource* create_resource(const string& name, size_t flags) final {

					auto resource = new Type(name, flags, type());

					// Creation failed
					if(!register_resource(resource)){
						delete resource;
						return nullptr;
					}

					return  resource;
				}
		};

		template <class Type> ResourceRegistry<Type>::ResourceRegistry(ResourceType type):BaseResourceRegistry(type) {}

		class GlobalResourceRegistry{

			private:
				common::Map<ResourceType, BaseResourceRegistry*> m_registries;
				inline static GlobalResourceRegistry* s_current;

			public:
				GlobalResourceRegistry();
				~GlobalResourceRegistry();

				static BaseResourceRegistry* get_registry(ResourceType type);

				static void add_registry(ResourceType type, BaseResourceRegistry* registry);
				static void remove_registry(BaseResourceRegistry* registry);
		};

		class ResourceManager{

			private:
				common::Map<uint64_t, Resource*> m_resources;

				uint64_t m_next_handle = 1;

			public:
				ResourceManager();
				~ResourceManager();

				common::Map<uint64_t, Resource*> resources();

				uint64_t open_resource(ResourceType type, const string& name, size_t flags);
				void 	 close_resource(uint64_t handle, size_t flags);

				Resource* get_resource(uint64_t handle);
				Resource* get_resource(const string& name);
		};
	}
}

#endif //MAXOS_PROCESSES_RESOURCE_H
