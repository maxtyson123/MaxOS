//
// Created by 98max on 8/31/2025.
//

#include <ipc/sharedmemory.h>

namespace syscore{
	namespace ipc{

		/**
		 * @brief Create a new region of shared memory
		 *
		 * @param name The name of the region
		 * @param size The size of the region
		 * @return The start address of the region or nullptr if it failed to create
		 */
		void* create_shared_memory(const char* name, size_t size){

			// Create the resource
			if(!resource_create(ResourceType::SHARED_MEMORY, name, size))
				return nullptr;

			return open_shared_memory(name);
		}

		void* open_shared_memory(const char* name){

			// Open the resource
			uint64_t handle = resource_open(ResourceType::SHARED_MEMORY, name, 0);
			if(!handle)
				return nullptr;

			// Get the address
			auto address = resource_read(handle, nullptr, 0, 0);
			if(!address)
				return nullptr;

			return (void*)address;
		}
	}
}
