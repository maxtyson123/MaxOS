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

        class SharedMemory final : public Resource {

            private:
                uintptr_t m_physical_address;
                size_t m_size;

				common::Map<size_t, uintptr_t> m_mappings;

            public:
                SharedMemory(const string& name, size_t size, resource_type_t type);
                ~SharedMemory() final;

                string name;
                uint64_t use_count = 1;

				void 	open(size_t flags) final;
				void 	close(size_t flags) final;

				int 	read(void* buffer, size_t size, size_t flags) final;

                [[nodiscard]] uintptr_t physical_address() const;
                [[nodiscard]] size_t size() const;
        };

        class SharedMessageEndpoint final : public Resource{

            private:
                common::Vector<common::buffer_t*> m_queue {};
                common::Spinlock m_message_lock;

            public:
              SharedMessageEndpoint(const string& name, size_t size, resource_type_t type);
              ~SharedMessageEndpoint() final;

			  int read(void* buffer, size_t size, size_t flags) final;
			  int write(const void* buffer, size_t size, size_t flags) final;
        };
    }

}

#endif // MAXOS_PROCESSES_IPC_H
