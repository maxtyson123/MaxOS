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

        //TODO: Can prob convert these to classes, Lock free, LibIPC

        typedef struct IPCSharedMemory {
            uintptr_t physical_address;
            size_t size;
            uint64_t use_count;
            uint64_t owner_pid;
            string* name;
        } ipc_shared_memory_t;


        typedef struct IPCMessage{
            void* message_buffer;
            size_t message_size;
            uintptr_t next_message;
        } ipc_message_t;

        typedef struct IPCMessageQueue{
            ipc_message_t* messages;
        } ipc_message_queue_t;

        typedef struct IPCMessageEndpoint {
            ipc_message_queue_t* queue;
            uint64_t owner_pid;
            string* name;
            common::Spinlock message_lock;
        } ipc_message_endpoint_t;

        /**
         * @class IPC
         * @brief Manages the Inter-Process Communication (IPC) between processes via shared memory and message passing
         */
        class IPC {

          common::Vector<ipc_shared_memory_t*> m_shared_memory_blocks;
          common::Vector<ipc_message_endpoint_t*> m_message_endpoints;

          common::Spinlock m_lock;

          public:

              IPC();
              ~IPC();

              ipc_shared_memory_t*  alloc_shared_memory(size_t size, string name);
              ipc_shared_memory_t*  get_shared_memory(const string& name);
              void                  free_shared_memory(const string& name);
              void                  free_shared_memory(uintptr_t physical_address);
              void                  free_shared_memory(ipc_shared_memory_t* block);

              ipc_message_endpoint_t* create_message_endpoint(const string& name);
              ipc_message_endpoint_t* get_message_endpoint(const string& name);
              void                    free_message_endpoint(const string& name);
              static void                    free_message_endpoint(ipc_message_endpoint_t* endpoint);
              void                    send_message(const string& name, void* message, size_t size);
              static void                    send_message(ipc_message_endpoint_t* endpoint, void* message, size_t size);


          };

    }

}

#endif // MAXOS_PROCESSES_IPC_H
