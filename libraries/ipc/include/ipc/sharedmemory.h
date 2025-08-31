//
// Created by 98max on 8/31/2025.
//

#ifndef IPC_SHAREDMEMORY_H
#define IPC_SHAREDMEMORY_H

#include <stdint.h>
#include <stddef.h>
#include <syscalls.h>

namespace IPC{

	void*	create_shared_memory(const char* name, size_t size);
	void*	open_shared_memory(const char* name);
}

#endif //IPC_SHAREDMEMORY_H
