//
// Created by 98max on 8/31/2025.
//

#include <syscalls.h>

namespace syscore{

	int as_error(ResourceErrorBase code){
		return -1 * (int)code;
	}

	/**
	 * @brief Make a syscall
	 *
	 * @param type The type of syscall
	 * @param arg0 The first argument
	 * @param arg1 The second argument
	 * @param arg2 The third argument
	 * @param arg3 The fourth argument
	 * @param arg4 The fifth argument
	 * @param arg5 The sixth argument
	 * @return The result of the syscall
	 */
	void* make_syscall(SyscallType type, uint64_t arg0, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5){

		void* result;
		asm volatile(
				"mov %[a0], %%rdi\n\t"   // arg0 -> rdi
				"mov %[a1], %%rsi\n\t"   // arg1 -> rsi
				"mov %[a2], %%rdx\n\t"   // arg2 -> rdx
				"mov %[a3], %%r10\n\t"   // arg3 -> r10
				"mov %[a4], %%r8\n\t"    // arg4 -> r8
				"mov %[a5], %%r9\n\t"    // arg5 -> r9
				"mov %[num], %%rax\n\t"  // syscall number -> rax
				"int $0x80\n\t"
				: "=a"(result)
				: [num] "r"((uint64_t)type),
				[a0] "r"(arg0),
				[a1] "r"(arg1),
				[a2] "r"(arg2),
				[a3] "r"(arg3),
				[a4] "r"(arg4),
				[a5] "r"(arg5)
				: "rdi", "rsi", "rdx", "r10", "r8", "r9", "memory"
		);

		return result;

	}

	/**
	 * @brief Log a message to the kernel log
	 *
	 * @param message The message to log
	 */
	void klog(const char* message){
		make_syscall(SyscallType::KLOG, (uint64_t)message, 0, 0, 0, 0, 0);
	}

	/**
	 * @brief Allocate memory
	 *
	 * @param size The size of memory to allocate
	 * @return Pointer to the allocated memory
	 */
	void* allocate_memory(size_t size){
		return make_syscall(SyscallType::ALLOCATE_MEMORY, size, 0, 0, 0, 0, 0);
	}

	/**
	 * @brief Free allocated memory
	 *
	 * @param address The address of the memory to free
	 */
	void free_memory(void* address){
		make_syscall(SyscallType::FREE_MEMORY, (uint64_t)address, 0, 0, 0, 0, 0);
	}

	/**
	 * @brief Create a resource
	 *
	 * @param type The type of resource
	 * @param name The name of the resource
	 * @param flags Optional flags to pass (unused by default but resource type specific)
	 * @return The handle id of the resource or 0 if failed
	 */
	bool resource_create(ResourceType type, const char* name, size_t flags){
		return (bool)make_syscall(SyscallType::RESOURCE_CREATE, (uint64_t)type, (uint64_t)name, flags, 0, 0, 0);
	}

	/**
	 * @brief Open an existing resource
	 *
	 * @param type The type of resource
	 * @param name The name of the resource
	 * @param flags Optional flags to pass
	 * @return The handle id of the resource or 0 if failed
	 */
	uint64_t resource_open(ResourceType type, const char* name, size_t flags){
		return (uint64_t)make_syscall(SyscallType::RESOURCE_OPEN, (uint64_t)type, (uint64_t)name, flags, 0, 0, 0);
	}

	/**
	 * @brief Close a resource
	 *
	 * @param handle The handle number of the resource
	 * @param flags Flags to pass to the closing
	 */
	void resource_close(uint64_t handle, size_t flags){
		make_syscall(SyscallType::RESOURCE_CLOSE, handle, flags, 0, 0, 0, 0);
	}

	/**
	 * @brief write a certain amount of bytes to a resource
	 *
	 * @param handle The handle number of the resource
	 * @param buffer The buffer to read from
	 * @param size The number of bytes to write
	 * @param flags The flags to pass to the writing
	 * @return The number of bytes successfully written
	 */
	size_t resource_write(uint64_t handle, const void* buffer, size_t size, size_t flags){

		int response = 0;
		while (true){
			response = (int)(uintptr_t)make_syscall(SyscallType::RESOURCE_WRITE, handle, (uint64_t)buffer, size, flags, 0, 0);
			if(response == as_error(ResourceErrorBase::SHOULD_BLOCK))
				yeild();
			else
				break;
		}
		return response;
	}

	/**
	 * @brief read a certain amount of bytes from a resource
	 *
	 * @param handle The handle number of the resource
	 * @param buffer The buffer to read into
	 * @param size The number of bytes to read
	 * @param flags The flags to pass to the reading
	 * @return The number of bytes successfully read
	 */
	size_t resource_read(uint64_t handle, void* buffer, size_t size, size_t flags){
		int response = 0;
		while (true){
			response = (int)(uintptr_t)make_syscall(SyscallType::RESOURCE_READ, handle, (uint64_t)buffer, size, flags, 0, 0);
			if(response == as_error(ResourceErrorBase::SHOULD_BLOCK))
				yeild();
			else
				break;
		}
		return response;
	}

	/**
	 * @brief Yield the current thread's execution
	 */
	void yeild(){
		make_syscall(SyscallType::YEILD, 0, 0, 0, 0, 0, 0);
	}
}