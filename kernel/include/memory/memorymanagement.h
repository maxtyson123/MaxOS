/**
 * @file memorymanagement.h
 * @brief Defines a MemoryManager class for handling memory allocation and deallocation
 *
 * @date 20th October 2022
 * @author Max Tyson
 *
 */

#ifndef MAXOS_SYSTEM_MEMORYMANAGEMENT_H
#define MAXOS_SYSTEM_MEMORYMANAGEMENT_H

#include <cstddef>
#include <cstdint>

#include <libcommon/assert.h>

#include <memory/virtual.h>
#include <memory/allocator/allocator.h>
#include <memory/allocator/bump.h>
#include <system/multiboot.h>

namespace MaxOS::memory {


	class BumpMemoryManager : public allocator::BumpAllocator {

		private:

			VirtualMemoryManager* m_virtual_memory_manager;

			void* allocate_extra_space(size_t size) final;

		public:

			BumpMemoryManager(VirtualMemoryManager* virtual_memory_manager = nullptr);
			~BumpMemoryManager();

			// Virtual Memory Management
			VirtualMemoryManager* vmm();
			static void switch_active_memory_manager(BumpMemoryManager* manager);
	};

	/**
	 * @class MemoryManager
	 * @brief Handles memory allocation and deallocation
	 */
	class MemoryManager : public BumpMemoryManager {


		public:
			inline static MemoryManager* s_kernel_memory_manager = nullptr;             ///< The memory manager for any kernel processes and all kernel allocations

			explicit MemoryManager(VirtualMemoryManager* virtual_memory_manager = nullptr);
			~MemoryManager();

			// Public Memory Management
			static void* malloc(size_t size);
			static void  free(void* pointer);

			// Kernel Memory Management
			static void* kmalloc(size_t size);
			static void  kfree(void* pointer);
	};
}

#endif //MAXOS_SYSTEM_MEMORYMANAGEMENT_H
