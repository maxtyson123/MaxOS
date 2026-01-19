/**
 * @file service_resources.h
 * @brief Defines scheduler resource classes for wrapping userspace service Resources in the Global Scheduler, allowing operations to be performed through the Resource interface.
 *
 * @date 18th January 2026
 * @author Max Tyson
 */

#ifndef MAXOS_PROCESSES_RESOURCES_SERVICE_RESOURCES_H
#define MAXOS_PROCESSES_RESOURCES_SERVICE_RESOURCES_H

#include <processes/resources/resource.h>
#include <processes/process.h>
#include <processes/ipc.h>
#include <memory/memorymanagement.h>

#include <stddef.h>
#include <stdatomic.h>

#include <stdint.h>

namespace MaxOS::processes::resources {

	/**
	 * @enum ServiceResourceCommand
	 * @brief
	 */
	enum class ServiceResourceCommand {
		S_CREATE,
		S_GET,
		R_OPEN,
		R_CLOSE,
		R_WRITE,
		R_READ,
	};


	enum class ServiceMessageSlotState : uint8_t {
		FREE,
		REQUEST,
		RESPONSE,
	};

	/**
	 * @struct ServiceResourceMessage
	 * @brief Header for a function call on a resource managed by a userspace service
	 *
	 * @typedef service_resource_message_t
	 * @brief Alais for ServiceResourceMessage
	 */
	typedef struct ServiceResourceMessage {
		size_t resource_id;
		size_t flags;
		size_t command;

		size_t data_size;
		size_t data_offset;

		int64_t response;
		ServiceMessageSlotState state;
	} service_resource_message_t;


	constexpr size_t MESSAGE_SLOTS = 0x100;						///< How manny message slots are
	constexpr size_t MAX_COPY_SIZE = 0x1000;					///< How large can be copied in to the shared memory
	constexpr size_t DATA_SIZE = MAX_COPY_SIZE * MESSAGE_SLOTS;	///< How much space to allocate for the data region

	typedef struct ServiceMessageRing {

		atomic_size_t head;
		atomic_size_t tail;

		service_resource_message_t ring_buffer[MESSAGE_SLOTS];

	} service_message_ring_t;

	class ServiceHandler : public memory::MemoryChunkHandler {

		private:
			size_t m_next_id = 0;

			uintptr_t m_shared_region;
			service_message_ring_t* m_message_ring;
			uintptr_t m_data_region;

			size_t store_data(const void* buffer, size_t size, bool reserve_only);
			service_resource_message_t* aquire_slot();

		public:
			ServiceHandler();
			~ServiceHandler();

			size_t allocate_id();

			int64_t send_to_service(size_t id, ServiceResourceCommand command, const void* buffer, size_t size, size_t flags);
	};

	/**
	 * @class ServiceResource
	 * @brief A generic wrapper for a Resource that exists on a userspace server which exposes its operations as Resource operations
	 */
	class ServiceResource final : public Resource {

		private:
			ServiceHandler* m_handler;
			size_t m_id;

		public:
			ServiceResource(const string& name, size_t flags, resource_type_t type);
			~ServiceResource() final;

			void open(size_t flags) final;
			void close(size_t flags) final;

			int read(void* buffer, size_t size, size_t flags) final;
			int write(const void* buffer, size_t size, size_t flags) final;

			void attach_wrapper(ServiceHandler* wrapper);
			bool get_on_service();
			void construct_on_service();

	};

	class ServiceResourceRegistry : public ResourceRegistry<ServiceResource> {

		private:
			ServiceHandler m_handler;

		public:
			ServiceResourceRegistry(size_t resource_id);
			~ServiceResourceRegistry();

			Resource* get_resource(const string& name) final;
			Resource* create_resource(const string& name, size_t flags) final;
	};

}

#endif //MAXOS_PROCESSES_RESOURCES_SERVICE_RESOURCES_H
