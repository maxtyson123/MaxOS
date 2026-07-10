//
// Created by 98max on 8/07/2026.
//

#ifndef MAXOS_MEMORY_ALLOCATOR_SLAB_H
#define MAXOS_MEMORY_ALLOCATOR_SLAB_H

#include <cstddef>
#include <cstdint>

namespace MaxOS::memory::allocator {

	typedef struct SlabHeader {
		SlabHeader* next;
		void* page;

		uint32_t used_blocks;
		uint32_t total_blocks;

		uint64_t block_state[4];

	} slab_header_t;

	class SlabCache {

		private:

			size_t m_slab_size;

			slab_header_t* m_root_slab = nullptr;
			slab_header_t* new_slab();


		public:
			SlabCache(size_t size);
			~SlabCache();

			void* allocate_slab();
			void  free_slav(void* address);


	};

	class SlabAllocator {

		private:

			/// Contains slabs of 16, 32, 64, 128, 256, 512, 1024, 2048 bytes
			SlabCache m_caches[8];
			SlabCache* get_cache(size_t size);

			virtual void* expand_heap(size_t size);
			virtual void* allocate_extra_space(size_t requested_size, size_t& actual_size);

		public:
			SlabAllocator();
			virtual ~SlabAllocator();

			virtual void setup_region(uintptr_t address, size_t length);

			virtual void* allocate(size_t size);
			virtual void  unallocate(void* pointer);

			virtual size_t amount_used();
			virtual size_t align(size_t size);

	};

}

#endif //MAXOS_MEMORY_ALLOCATOR_SLAB_H
