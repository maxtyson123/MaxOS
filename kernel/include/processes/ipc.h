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

namespace MaxOS {
    namespace processes {

        //TODO:
        typedef struct IPCSharedMemory {
            uintptr_t physical_address;
            size_t size;
            uint64_t use_count;
            string* name;
        } ipc_shared_memory_t;

        /**
         * @class IPC
         * @brief Handles Inter Process Communication
         */
        class IPC {

          common::Vector<ipc_shared_memory_t*> m_shared_memory_blocks;
          common::Spinlock m_lock;

          public:

              IPC();
              ~IPC();

              ipc_shared_memory_t*  alloc_shared_memory(size_t size, string name);
              void                  free_shared_memory(string name);
              void                  free_shared_memory(uintptr_t physical_address);
              void                  free_shared_memory(ipc_shared_memory_t* block);
              ipc_shared_memory_t*  get_shared_memory(string name);


          };

    }

}

#endif // MAXOS_PROCESSES_IPC_H
