/**
 * @file resource.h
 * @brief Defines types of resources and also a ResourceServer to allow for a userspace process to provide resources to other processes
 *
 * @date 21 January 2026
 * @author Max Tyson
 */

#ifndef MAXOS_KPI_RESOURCE_H
#define MAXOS_KPI_RESOURCE_H

#include <string.h>
#include <map.h>

namespace MaxOS::KPI {

    /**
     * @brief The type of resource
     */
    enum class ResourceType{
        MESSAGE_ENDPOINT,
        SHARED_MEMORY,
        FILESYSTEM,
        PROCESS,
        THREAD,

        _END,
    };

    /**
     * @class Resource
     * @brief Represents a generic resource that can be opened, closed, read from and written to
     */
    class Resource {

        private:
            string m_name;
            ResourceType m_type;

        public:

            Resource(const string& name, size_t flags, ResourceType type);
            virtual ~Resource();

            string name();
            ResourceType type();

            virtual void open(size_t flags);
            virtual void close(size_t flags);

            virtual int read(void* buffer, size_t size, size_t flags);
            virtual int write(const void* buffer, size_t size, size_t flags);
    };


    /**
     * @enum ServiceResourceCommand
     * @brief
     */
    enum class ServiceResourceCommand : uint8_t {
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
     * @typedef bridge_resource_message_t
     * @brief Alais for BridgeResourceMessage
     */
    typedef struct ServiceResourceMessage {
        uint64_t sending_pid;
        size_t resource_id;
        size_t flags;
        uint8_t command;

        size_t data_size;
        size_t data_offset;

        int64_t response;
        uint8_t state;
    } service_resource_message_t;


    constexpr size_t MESSAGE_SLOTS = 0x100;						///< How manny message slots are
    constexpr size_t MAX_COPY_SIZE = 0x1000;					///< How large can be copied in to the shared memory
    constexpr size_t DATA_SIZE = MAX_COPY_SIZE * MESSAGE_SLOTS;	///< How much space to allocate for the data region

    typedef struct ServiceMessageRing {

        size_t head {0};
        size_t tail {0};

        service_resource_message_t ring_buffer[MESSAGE_SLOTS];

    } service_message_ring_t;

    constexpr size_t MESSAGE_SIZE				= sizeof(service_message_ring_t);	///< How much space to allocate for the message region
    constexpr size_t SERVICE_SHARED_MEM_SIZE	= MESSAGE_SIZE + DATA_SIZE;			///< Total space required for the shared memory

    /**
     * @class ResourceServer
     * @brief Manages resources and allows for this process to provide resources to other processes
     *
     * @warning Single threaded, do not call resource server functions concurrently
     */
    class ResourceServer {

        private:
            void* m_shared_region = nullptr;
            string m_server_name;

            service_message_ring_t* m_message_ring = nullptr;
            service_resource_message_t* m_current_processed_message;
            void* m_data_region;

            common::Map<uint64_t, Resource*> m_resource_map;


        public:
            ResourceServer(string server_name, size_t resource_id);
            ~ResourceServer();

            bool queue_empty();
            service_resource_message_t* peek_front();
            service_resource_message_t* dequeue_front();
            void advance_queue();

            void send_response(service_resource_message_t* message, int64_t response);

            service_resource_message_t* current_processed_message();
            void process_message(service_resource_message_t* message);
            void process_next();
            void loop();

            virtual Resource* 	get_resource(const string& name);
            virtual Resource* 	create_resource(const string& name, size_t flags, uintptr_t data);

    };


}

#endif //MAXOS_KPI_RESOURCE_H