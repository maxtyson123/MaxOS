//
// Created by 98max on 8/31/2025.
//

#ifndef SYSCORE_IPC_SHAREDMEMORY_H
#define SYSCORE_IPC_SHAREDMEMORY_H

#include <stdint.h>
#include <stddef.h>
#include <syscalls.h>

namespace syscore {
	namespace ipc {

		void* create_shared_memory(const char* name, size_t size);

		void* open_shared_memory(const char* name);
	}
}

#endif //SYSCORE_IPC_SHAREDMEMORY_H
