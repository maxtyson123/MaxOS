//
// Created by 98max on 8/31/2025.
//

#include <ipc/messages.h>

using namespace system;

namespace IPC{

	/**
	 * @brief Create a new endpoint for sending and receiving messages
	 *
	 * @param name The name of the endpoint
	 * @return The handle of the new endpoint or 0 if it failed
	 */
	uint64_t create_endpoint(const char* name){

		// Create the resource
		if(!resource_create(ResourceType::MESSAGE_ENDPOINT, name, 0))
			return 0;

		return open_endpoint(name);
	}

	/**
	 * @brief Opens an endpoint under a specific name
	 *
	 * @param name The name of the endpoint to open
	 * @return The endpoint handle or 0 if it failed to open
	 */
	uint64_t open_endpoint(const char* name){
		return (uint64_t) resource_open(ResourceType::MESSAGE_ENDPOINT, name, 0);
	}

	/**
	 * @brief Closes an endpoint
	 *
	 * @param endpoint The endpoint handle to close
	 */
	void close_endpoint(uint64_t endpoint){

		if(endpoint)
			resource_close(endpoint, 0);

	}

	/**
	 * @brief Queues a message on the endpoint (FIFO)
	 *
	 * @param endpoint The endpoint handle
	 * @param buffer The message
	 * @param size The size of the message
	 */
	void send_message(uint64_t endpoint, void* buffer, size_t size){

		if(endpoint)
			resource_write(endpoint, buffer, size, 0);

	}

	/**
	 * @brief Fetches the oldest message on the endpoint (FIFO)
	 *
	 * @param endpoint The endpoint handle
	 * @param buffer Where to read the message into
	 * @param size How much of the message to read
	 */
	void read_message(uint64_t endpoint, void* buffer, size_t size){
		if(endpoint)
			resource_read(endpoint, buffer, size, 0);
	}
}
