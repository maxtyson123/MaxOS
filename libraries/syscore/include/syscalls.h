//
// Created by 98max on 8/31/2025.
//

#ifndef SYSCORE_SYSCALLS_H
#define SYSCORE_SYSCALLS_H

#include <cstdint>
#include <cstddef>

namespace syscore{

	enum class ResourceType{
		MESSAGE_ENDPOINT,
		SHARED_MEMORY,
		FILESYSTEM,
		PROCESS,
		THREAD,
	};

	enum class ResourceErrorBase{
		SHOULD_BLOCK = 1,

		_END // Use this to extend errors
	};
	int as_error(ResourceErrorBase code);

	enum class SyscallType{
		KLOG,

		ALLOCATE_MEMORY,
		FREE_MEMORY,

		RESOURCE_CREATE,
		RESOURCE_OPEN,
		RESOURCE_CLOSE,
		RESOURCE_WRITE,
		RESOURCE_READ,

		YEILD,
	};

	void* make_syscall(SyscallType type, uint64_t arg0, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5);

	void* allocate_memory(size_t size);
	void free_memory(void* address);

	bool resource_create(ResourceType type, const char* name, size_t flags);
	uint64_t resource_open(ResourceType type, const char* name, size_t flags);
	void resource_close(uint64_t handle, size_t flags);
	size_t resource_write(uint64_t handle, const void* buffer, size_t size, size_t flags);
	size_t resource_read(uint64_t handle, void* buffer, size_t size, size_t flags);

	void yeild();
}

#endif //SYSCORE_SYSCALLS_H
