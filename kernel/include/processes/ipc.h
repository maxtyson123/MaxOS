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

        class IPCSharedMemory{

            private:
                uintptr_t m_physical_address;
                size_t m_size;
                uint64_t m_owner_pid;


            public:
                explicit IPCSharedMemory(string name, size_t size);
                ~IPCSharedMemory();

                string* name;
                uint64_t use_count = 1;

                [[nodiscard]] uintptr_t physical_address() const;
                [[nodiscard]] size_t size() const;
        };


        typedef struct IPCMessage{
            void* message_buffer;
            size_t message_size;
            uintptr_t next_message;
        } ipc_message_t;

        typedef struct IPCMessageQueue{
            ipc_message_t* messages;
        } ipc_message_queue_t;

        class IPCMessageEndpoint{

            private:
                ipc_message_queue_t* m_queue = nullptr;
                uint64_t m_owner_pid;
                common::Spinlock m_message_lock;

            public:
              explicit IPCMessageEndpoint(string name);
              ~IPCMessageEndpoint();

              string* name;
              [[nodiscard]] ipc_message_queue_t* queue() const;

              void queue_message(void* message, size_t size);
              [[nodiscard]] bool owned_by_current_process() const;
        };

        /**
         * @class IPC
         * @brief Manages the Inter-Process Communication (IPC) between processes via shared memory and message passing
         */
        class IPC {

          common::Vector<IPCSharedMemory*> m_shared_memory_blocks;
          common::Vector<IPCMessageEndpoint*> m_message_endpoints;

          common::Spinlock m_lock;

          public:

              IPC();
              ~IPC();

              IPCSharedMemory*  alloc_shared_memory(size_t size, string name);
              IPCSharedMemory*  get_shared_memory(const string& name);
              void              free_shared_memory(const string& name);
              void              free_shared_memory(uintptr_t physical_address);
              void              free_shared_memory(IPCSharedMemory* block);

              IPCMessageEndpoint* create_message_endpoint(const string& name);
              IPCMessageEndpoint* get_message_endpoint(const string& name);
              void                free_message_endpoint(const string& name);
              static void         free_message_endpoint(IPCMessageEndpoint* endpoint);
          };

    }

}

#endif // MAXOS_PROCESSES_IPC_H
