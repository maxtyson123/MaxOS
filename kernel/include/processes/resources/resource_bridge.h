/**
 * @file resource_bridge.h
 * @brief Defines scheduler resource classes for wrapping userspace bridge Resources in the Global Scheduler, allowing operations to be performed through the Resource interface.
 *
 * @date 18th January 2026
 * @author Max Tyson
 */

#ifndef MAXOS_PROCESSES_RESOURCES_SERVICE_RESOURCES_H
#define MAXOS_PROCESSES_RESOURCES_SERVICE_RESOURCES_H


#include <stddef.h>
#include <atomic>
#include <stdint.h>

#include <libkpi/resource.h>

#include <processes/resources/resource.h>
#include <processes/process.h>
#include <processes/scheduler.h>
#include <processes/ipc.h>
#include <memory/memorymanagement.h>
#include <hardwarecommunication/interrupts.h>

namespace MaxOS::processes::resources {

	class BridgeHandler : public memory::MemoryChunkHandler {

		private:
			size_t m_next_id = 0;
			BlockingLock m_lock;

			uintptr_t m_shared_region;
			KPI::service_message_ring_t* m_message_ring;
			uintptr_t m_data_region;

			size_t store_data(const void* buffer, size_t size, bool reserve_only);
			KPI::service_resource_message_t* aquire_slot();

			Process* m_owner_process;

		public:
			BridgeHandler(string const& shared_name, Process* owner_process);
			~BridgeHandler();

			size_t allocate_id();

			int64_t send_to_bridge(size_t id, KPI::ServiceResourceCommand command, const void* buffer, size_t size, size_t flags);
	};

	/**
	 * @class BridgeResource
	 * @brief A generic wrapper for a Resource that exists on a userspace server which exposes its operations as Resource operations
	 */
	class BridgeResource final : public Resource {

		private:
			BridgeHandler* m_handler;
			size_t m_id;

		public:
			BridgeResource(const string& name, size_t flags, resource_type_t type);
			~BridgeResource() final;

			void open(size_t flags) final;
			void close(size_t flags) final;

			int read(void* buffer, size_t size, size_t flags) final;
			int write(const void* buffer, size_t size, size_t flags) final;

			void attach_wrapper(BridgeHandler* wrapper);
			bool get_on_bridge();
			void create_on_bridge();

	};

	/**
	 * @class BridgeResourceRegistry
	 * @brief A generic wrapper for a registry of resources that exist on a userspace server
	 */
	class BridgeResourceRegistry : public ResourceRegistry<BridgeResource> {

		private:
			BridgeHandler m_handler;

		public:
			BridgeResourceRegistry(const string &shared_memory_name, size_t resource_id);
			~BridgeResourceRegistry();

			Resource* get_resource(const string& name) final;
			Resource* create_resource(const string& name, size_t flags, uintptr_t data) final;
	};

}

#endif //MAXOS_PROCESSES_RESOURCES_SERVICE_RESOURCES_H
