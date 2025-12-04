//
// Created by 98max on 8/31/2025.
//

#ifndef MAXOS_KPI_IPC_SHAREDMEMORY_H
#define MAXOS_KPI_IPC_SHAREDMEMORY_H

#include <cstdint>
#include <cstddef>
#include <syscalls.h>


namespace MaxOS::KPI::ipc {

	void* create_shared_memory(const char* name, size_t size);

	void* open_shared_memory(const char* name);
}


#endif //MAXOS_KPI_IPC_SHAREDMEMORY_H
