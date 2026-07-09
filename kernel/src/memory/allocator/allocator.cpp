//
// Created by 98max on 8/07/2026.
//

#include <memory/allocator/allocator.h>

using namespace MaxOS;
using namespace MaxOS::memory;
using namespace MaxOS::memory::allocator;

MemoryAllocator::MemoryAllocator() = default;

MemoryAllocator::~MemoryAllocator() = default;

void* MemoryAllocator::expand_heap(size_t size) {
	return nullptr;
}

void* MemoryAllocator::allocate_extra_space(size_t size) {
	return nullptr;
}

void MemoryAllocator::setup_region(uintptr_t address, size_t length) {
}

void* MemoryAllocator::allocate(size_t size) {
	return nullptr;
}

void MemoryAllocator::unallocate(void* pointer) {
}

size_t MemoryAllocator::amount_used() {
	return 0;
}

size_t MemoryAllocator::align(size_t size) {
	return 0;
}




