//
// Created by 98max on 8/31/2025.
//

#ifndef MAXOS_KPI_IPC_MESSAGES_H
#define MAXOS_KPI_IPC_MESSAGES_H

#include <cstdint>
#include <cstddef>
#include <syscalls.h>

//TODO: Flags such that RPC using enpoint is restricted to two processes

namespace MaxOS::KPI::ipc {

	uint64_t create_endpoint(const char* name);
	uint64_t open_endpoint(const char* name);
	void close_endpoint(uint64_t endpoint);

	void send_message(uint64_t endpoint, void* buffer, size_t size);

	size_t read_message(uint64_t endpoint, void* buffer, size_t size);
}


#endif //MAXOS_KPI_IPC_MESSAGES_H
