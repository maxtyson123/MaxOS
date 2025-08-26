//
// Created by 98max on 8/26/2025.
//

#ifndef MAXOS_PROCESSES_RESOURCE_H
#define MAXOS_PROCESSES_RESOURCE_H

#include <stddef.h>
#include <common/map.h>
#include <common/vector.h>
#include <common/string.h>

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

				Resource(const string& name, ResourceType type);
				~Resource();

				string name();
				ResourceType type();

				virtual void open();
				virtual void close();

				virtual size_t read(void* buffer, size_t size);
				virtual size_t write(const void* buffer, size_t size);
		};

		class ResourceRegistry{

			private:
				common::Map<string, Resource*> m_resources;
				common::Map<string, uint64_t>  m_resource_uses;

				ResourceType m_type;

			public:
				explicit ResourceRegistry(ResourceType type);
				~ResourceRegistry();

				ResourceType type();

				Resource* get_resource(const string& name);
				void close_resource(Resource* resource);

				void register_resource(Resource* resource, const string& name);
		};

		class GlobalResourceRegistry{

			private:
				common::Map<ResourceType, ResourceRegistry*> m_registries;
				inline static GlobalResourceRegistry* s_current;

			public:
				GlobalResourceRegistry();
				~GlobalResourceRegistry();

				static ResourceRegistry* get_registry(ResourceType type);

				static void add_registry(ResourceType type, ResourceRegistry* registry);
				static void remove_registry(ResourceRegistry* registry);


		};

		class ResourceManager{

			private:
				common::Map<uint64_t, Resource*> m_resources;

				uint64_t m_next_handle = 1;

			public:
				ResourceManager();
				~ResourceManager();

				uint64_t open_resource(ResourceType type, const string& name);
				void 	 close_resource(uint64_t handle);

				Resource* get_resource(uint64_t handle);
				Resource* get_resource(const string& name);
		};
	}
}

#endif //MAXOS_PROCESSES_RESOURCE_H
