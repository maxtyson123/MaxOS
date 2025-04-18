//
// Created by 98max on 24/03/2025.
//

#ifndef MAXOS_PROCESSES_IPC_H
#define MAXOS_PROCESSES_IPC_H

#include <stdint.h>
#include <stddef.h>
#include <common/vector.h>
#include <common/string.h>
#include <common/spinlock.h>
#include <memory/physical.h>
#include <memory/memoryIO.h>


namespace MaxOS {
    namespace processes {

        //TODO: LibIPC

        class SharedMemory{

            private:
                uintptr_t m_physical_address;
                size_t m_size;
                uint64_t m_owner_pid;


            public:
                SharedMemory(string name, size_t size);
                ~SharedMemory();

                string* name;
                uint64_t use_count = 1;

                [[nodiscard]] uintptr_t physical_address() const;
                [[nodiscard]] size_t size() const;
        };


        typedef struct SharedMessage{
            void* message_buffer;
            size_t message_size;
            uintptr_t next_message;
        } ipc_message_t;

        typedef struct SharedMessageQueue{
            ipc_message_t* messages;
        } ipc_message_queue_t;

        class SharedMessageEndpoint{

            private:
                ipc_message_queue_t* m_queue = nullptr;
                uint64_t m_owner_pid;
                common::Spinlock m_message_lock;

            public:
              SharedMessageEndpoint(string name);
              ~SharedMessageEndpoint();

              string* name;
              [[nodiscard]] ipc_message_queue_t* queue() const;

              void queue_message(void* message, size_t size);
              [[nodiscard]] bool owned_by_current_process() const;
        };

        /**
         * @class InterProcessCommunicationManager (IPC)
         * @brief Manages the Inter-Process Communication between processes via shared memory and message passing
         */
        class InterProcessCommunicationManager {

          common::Vector<SharedMemory*> m_shared_memory_blocks;
          common::Vector<SharedMessageEndpoint*> m_message_endpoints;

          common::Spinlock m_lock;

          public:

              InterProcessCommunicationManager();
              ~InterProcessCommunicationManager();

              SharedMemory*  alloc_shared_memory(size_t size, string name);
              SharedMemory*  get_shared_memory(const string& name);
              void              free_shared_memory(const string& name);
              void              free_shared_memory(uintptr_t physical_address);
              void              free_shared_memory(SharedMemory* block);

              SharedMessageEndpoint* create_message_endpoint(const string& name);
              SharedMessageEndpoint* get_message_endpoint(const string& name);
              void                free_message_endpoint(const string& name);
              static void         free_message_endpoint(SharedMessageEndpoint* endpoint);
          };

    }

}

#endif // MAXOS_PROCESSES_IPC_H
