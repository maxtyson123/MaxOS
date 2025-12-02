/**
 * @file scheduler_resources.h
 * @brief Defines scheduler resource classes for wrapping Processe and Thread objects as Resources in the Global Scheduler,
 * allowing scheduler operations to be performed through the Resource interface.
 *
 * @date 1st December 2025
 * @author Max Tyson
 */

#ifndef MAXOS_PROCESSES_SCHEDULER_RESOURCES_H
#define MAXOS_PROCESSES_SCHEDULER_RESOURCES_H

#include <processes/resource.h>
#include <processes/process.h>
#include <memory/memorymanagement.h>
#include <syscore/include/processes/process.h>
#include <syscore/include/processes/thread.h>

namespace MaxOS::processes {

	/**
	 * @class ProcessResource
	 * @brief A wrapper for a Process which exposes Process operations as Resource operations
	 *
	 * @see Process
	 * @see Resource
	 *
	 * @note The process pointer will be freed by scheduler during close()
	 */
	class ProcessResource final : public Resource {

		public:
			ProcessResource(const string& name, size_t flags, resource_type_t type);
			~ProcessResource() final;

			Process* process; ///< The process that this resource handles & exposes
			uint64_t exit_code; ///< The exit code of the process once it has exited

			void close(size_t flags) final;

			int read(void* buffer, size_t size, size_t flags) final;
			int write(const void* buffer, size_t size, size_t flags) final;

	};

	/**
	 * @class ThreadResource
	 * @brief A wrapper for a Thread which exposes Thread operations as Resource operations
	 *
	 * @see Thread
	 * @see Resource
	 *
	 * @note The thread pointer will be freed by scheduler during close()
	 */
	class ThreadResource final : public Resource {

		public:
			ThreadResource(const string& name, size_t flags, resource_type_t type);
			~ThreadResource() final;

			Thread* thread; ///< The thread that this resource handles & exposes
			uint64_t exit_code; ///< The exit code of the thread once it has exited


			void close(size_t flags) final;

			int read(void* buffer, size_t size, size_t flags) final;
			int write(const void* buffer, size_t size, size_t flags) final;
	};

	class ThreadResourceRegistry : public ResourceRegistry<ThreadResource> {

		public:
			ThreadResourceRegistry();
			~ThreadResourceRegistry();

			Resource* 	get_resource(const string& name) final;
			Resource* create_resource(const string& name, size_t flags) final;
	};

	class ProcessResourceRegistry : public ResourceRegistry<ProcessResource> {

		public:
			ProcessResourceRegistry();
			~ProcessResourceRegistry();

			Resource* 	get_resource(const string& name) final;
			Resource* 	create_resource(const string& name, size_t flags) final;
	};


}

#endif //MAXOS_PROCESSES_SCHEDULER_RESOURCES_H
