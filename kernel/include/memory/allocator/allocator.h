/**
 * @file allocator.h
 * @brief needs breif
 *
 * @author Max Tyson
 * @date 8th July 2026
 *
 * @todo Maybe have a allocation result instead of req,actual size
 */

#ifndef MAXOS_MEMORY_ALLOCATOR_H
#define MAXOS_MEMORY_ALLOCATOR_H

#include <cstddef>
#include <cstdint>

namespace MaxOS::memory::allocator {

	class MemoryAllocator {

		private:

			virtual void* expand_heap(size_t size);
			virtual void* allocate_extra_space(size_t requested_size, size_t& actual_size);

		public:
			MemoryAllocator();
			virtual ~MemoryAllocator();

			virtual void setup_region(uintptr_t address, size_t length);

			virtual void* allocate(size_t size);
			virtual void  unallocate(void* pointer);

			virtual size_t amount_used();
			virtual size_t align(size_t size);

	};

}

#endif //MAXOS_MEMORY_ALLOCATOR_H
