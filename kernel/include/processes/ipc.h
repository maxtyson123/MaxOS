//
// Created by 98max on 24/03/2025.
//

#ifndef MAXOS_PROCESSES_IPC_H
#define MAXOS_PROCESSES_IPC_H

#include <stdint.h>
#include <stddef.h>
#include <common/vector.h>
#include <common/string.h>
#include <common/buffer.h>
#include <common/spinlock.h>
#include <memory/physical.h>
#include <memory/memoryIO.h>
#include <processes/resource.h>

namespace MaxOS {
    namespace processes {

        //TODO: LibIPC

        class SharedMemory final : public Resource {

            private:
                uintptr_t m_physical_address;
                size_t m_size;
                uint64_t m_owner_pid;

            public:
                SharedMemory(const string& name, size_t size, ResourceType type);
                ~SharedMemory() final;

                string name;
                uint64_t use_count = 1;

				size_t read(void* buffer, size_t size, size_t flags) final;

                [[nodiscard]] uintptr_t physical_address() const;
                [[nodiscard]] size_t size() const;
        };

        class SharedMessageEndpoint final : public Resource{

            private:
                common::Vector<common::buffer_t*> m_queue {};
                common::Spinlock m_message_lock;

            public:
              SharedMessageEndpoint(const string& name, size_t size, ResourceType type);
              ~SharedMessageEndpoint() final;

			  size_t read(void* buffer, size_t size, size_t flags) final;
			  size_t write(const void* buffer, size_t size, size_t flags) final;

              void queue_message(void const* message, size_t size);
        };
    }

}

#endif // MAXOS_PROCESSES_IPC_H
