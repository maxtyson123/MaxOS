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

        //TODO: Can prob convert these to classes, Lock free, LibIPC, clean endpoints on proc kills

        typedef struct IPCSharedMemory {
            uintptr_t physical_address;
            size_t size;
            uint64_t use_count;
            uint64_t owner_pid;
            string* name;
        } ipc_shared_memory_t;


        // TODO: Add these two to a LibIPC and link the kernel against it for easier use in userspace
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
         * @brief Handles Inter Process Communication
         */
        class IPC {

          common::Vector<ipc_shared_memory_t*> m_shared_memory_blocks;
          common::Vector<ipc_message_endpoint_t*> m_message_endpoints;

          common::Spinlock m_lock;

          public:

              IPC();
              ~IPC();

              ipc_shared_memory_t*  alloc_shared_memory(size_t size, string name);
              ipc_shared_memory_t*  get_shared_memory(string name);
              void                  free_shared_memory(string name);
              void                  free_shared_memory(uintptr_t physical_address);
              void                  free_shared_memory(ipc_shared_memory_t* block);

              ipc_message_endpoint_t* create_message_endpoint(string name);
              ipc_message_endpoint_t* get_message_endpoint(string name);
              void                    free_message_endpoint(string name);
              void                    free_message_endpoint(ipc_message_endpoint_t* endpoint);
              void                    send_message(string name, void* message, size_t size);
              void                    send_message(ipc_message_endpoint_t* endpoint, void* message, size_t size);


          };

    }

}

#endif // MAXOS_PROCESSES_IPC_H
