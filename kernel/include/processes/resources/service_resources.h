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
#include <memory/memorymanagement.h>
#include <libkpi/include/processes/process.h>
#include <libkpi/include/processes/thread.h>

#include <stddef.h>
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
	} service_resource_message_t;

	/**
	 * @class ServiceResource
	 * @brief A generic wrapper for a Resource that exists on a userspace server which exposes its operations as Resource operations
	 */
	class ServiceResource final : public Resource {

		private:
			string m_endpoint;
			size_t m_id;


		public:
			ServiceResource(const string& name, size_t flags, resource_type_t type);
			~ServiceResource() final;

			void open(size_t flags) final;
			void close(size_t flags) final;

			int read(void* buffer, size_t size, size_t flags) final;
			int write(const void* buffer, size_t size, size_t flags) final;

			bool get_on_service(string endpoint, size_t id);
			void construct_on_service(string endpoint, size_t id);
			void send_to_service(ServiceResourceCommand command, const void* buffer, size_t size, size_t flags);

	};

	class ServiceResourceRegistry : public ResourceRegistry<ServiceResource> {

		private:
			string m_endpoint;
			size_t m_next_id;
			size_t m_service_flags;


		public:
			ServiceResourceRegistry(string endpoint, size_t resource_id);
			~ServiceResourceRegistry();

			Resource* get_resource(const string& name) final;
			Resource* create_resource(const string& name, size_t flags) final;
	};

}

#endif //MAXOS_PROCESSES_RESOURCES_SERVICE_RESOURCES_H
