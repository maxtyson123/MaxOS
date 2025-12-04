//
// Created by 98max on 12/4/2025.
//

#ifndef MAXOS_COMMON_MEM_H
#define MAXOS_COMMON_MEM_H

#include <cstdint>
#include <cstddef>

namespace MaxOS::common {

	void* memcpy(void* destination, const void* source, uint64_t num);
	void* memset(void* ptr, uint32_t value, uint64_t num);
	void* memmove(void* destination, const void* source, uint64_t num);
	int memcmp(const void* ptr1, const void* ptr2, uint64_t num);

}

void* operator new(size_t size) throw();
void* operator new[](size_t size) throw();

//Placement New
void* operator new(size_t size, void* pointer);
void* operator new[](size_t size, void* pointer);

void operator delete(void* pointer);
void operator delete[](void* pointer);

void operator delete(void* pointer, size_t size);
void operator delete[](void* pointer, size_t size);

#endif //MAXOS_COMMON_MEM_H
