/**
 * @file syscalls.h
 * @brief Defines the syscalls that can be made to the kernel from user space
 *
 * @date 31st August 2025
 * @author Max Tyson
 */

#ifndef MAXOS_KPI_SYSCALLS_H
#define MAXOS_KPI_SYSCALLS_H

#include <cstdint>
#include <cstddef>
#include <resource.h>

namespace MaxOS::KPI{


	enum class SyscallType{
		KLOG,

		ALLOCATE_MEMORY,
		FREE_MEMORY,

		RESOURCE_CREATE,
		RESOURCE_OPEN,
		RESOURCE_CLOSE,
		RESOURCE_WRITE,
		RESOURCE_READ,

		RESOURCE_REGISTRY_CREATE,
	};

	void* make_syscall(SyscallType type, uint64_t arg0, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5);

	void klog(const char* format, ...);

	void* allocate_memory(size_t size);
	void free_memory(void* address);

	bool resource_create(ResourceType type, const char* name, size_t flags, uintptr_t data);
	uint64_t resource_open(ResourceType type, const char* name, size_t flags);
	void resource_close(uint64_t handle, size_t flags);
	size_t resource_write(uint64_t handle, const void* buffer, size_t size, size_t flags);
	size_t resource_read(uint64_t handle, void* buffer, size_t size, size_t flags);

	bool resource_registry_create(char* name, size_t resource_id);
}

#endif //MAXOS_KPI_SYSCALLS_H
