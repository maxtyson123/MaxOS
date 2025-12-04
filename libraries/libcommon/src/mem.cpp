/**
 * @file mem.cpp
 * @brief Implements memory management functions
 *
 * @date 4th December 2025
 * @author Max Tyson
*/

#include <mem.h>

namespace MaxOS::common {


	/**
	 * @brief Copies a block of memory from one location to another
	 *
	 * @see https://wiki.osdev.org/Meaty_Skeleton#memcpy.28.29
	 *
	 * @param destination The destination to copy to
	 * @param source The source to copy from
	 * @param num The number of bytes to copy
	 * @return The destination
	 */
	void* memcpy(void* destination, const void* source, uint64_t num) {

		// Make sure the source and destination are not the same
		if (destination == source)
			return destination;

		// Make sure they exist
		if (destination == nullptr || source == nullptr)
			return destination;

		// Get the source and destination
		auto* dst = (unsigned char*) destination;
		const auto* src = (const unsigned char*) source;

		// Copy the data
		for (size_t i = 0; i < num; i++)
			dst[i] = src[i];

		// Usefully for easier code writing
		return destination;
	}

	/**
	 * @brief Fills a block of memory with a specified value
	 *
	 * @param ptr The pointer to the block of memory
	 * @param value The value to fill the block of memory with
	 * @param num The number of bytes to fill
	 * @return The pointer to the block of memory
	 */
	void* memset(void* ptr, unsigned char value, uint64_t num) {

		// Make sure the pointer exists
		if (ptr == nullptr)
			return ptr;

		auto* dst = (unsigned char*) ptr;
		for (size_t i = 0; i < num; i++)
			dst[i] = (unsigned char) value;
		return ptr;
	}

	/**
	 * @brief Fills a block of memory with a specified value
	 *
	 * @param ptr The pointer to the block of memory
	 * @param value The value to fill the block of memory with
	 * @param num The number of bytes to fill
	 * @return The pointer to the block of memory
	 */
	void* memset(void* ptr, uint32_t value, uint64_t num) {

		// Make sure the pointer exists
		if (ptr == nullptr)
			return ptr;

		auto* dst = (uint32_t*) ptr;
		for (size_t i = 0; i < num; i++)
			dst[i] = value;
		return ptr;
	}

	/**
	 * @brief Copies a block of memory from one location to another
	 *
	 * @param destination The destination to copy to
	 * @param source The source to copy from
	 * @param num The number of bytes to copy
	 * @return The destination
	 */
	void* memmove(void* destination, const void* source, uint64_t num) {

		// Make sure the source and destination are not the same
		if (destination == source)
			return destination;

		// Make sure they exist
		if (destination == nullptr || source == nullptr)
			return destination;

		auto* dst = (unsigned char*) destination;
		const auto* src = (const unsigned char*) source;
		if (dst < src) {
			for (size_t i = 0; i < num; i++)
				dst[i] = src[i];
		} else {
			for (size_t i = num; i != 0; i--)
				dst[i - 1] = src[i - 1];
		}
		return destination;
	}

	/**
	 * @brief Compares two blocks of memory
	 *
	 * @param ptr1 The m_first_memory_chunk block of memory
	 * @param ptr2 The second block of memory
	 * @param num The number of bytes to compare
	 * @return 0 if the blocks of memory are equal, -1 if ptr1 < ptr2, 1 if ptr1 > ptr2
	 */
	int memcmp(const void* ptr1, const void* ptr2, uint64_t num) {

		// Make sure the pointers exist
		if (ptr1 == nullptr || ptr2 == nullptr)
			return 0;

		const auto* p1 = (const unsigned char*) ptr1;
		const auto* p2 = (const unsigned char*) ptr2;
		for (size_t i = 0; i < num; i++) {
			if (p1[i] < p2[i])
				return -1;
			if (p1[i] > p2[i])
				return 1;
		}
		return 0;
	}


}

// Required includes
#ifdef MAXOS_KERNEL
	#include <memory/memorymanagement.h>
#else
	#include <syscalls.h>
#endif

/**
 * @brief Overloaded new operator, allocates memory using the KERNEL memory manager
 *
 * @param size The size of the memory to allocate
 * @return The pointer to the allocated memory
 */
void* operator new(size_t size) throw() {

	// Handle the memory allocation
#ifdef MAXOS_KERNEL
	return MaxOS::memory::MemoryManager::kmalloc(size);
#else
	return MaxOS::KPI::allocate_memory(size);
#endif
}

/**
 * @brief Overloaded new operator, allocates memory using the KERNEL memory manager
 *
 * @param size The size of the memory to allocate
 * @return The pointer to the allocated memory
 */
void* operator new[](size_t size) throw() {

	// Handle the memory allocation
#ifdef MAXOS_KERNEL
	return MaxOS::memory::MemoryManager::kmalloc(size);
#else
	return MaxOS::KPI::allocate_memory(size);
#endif
}

/**
 * @brief Overloaded new operator, allocates memory using the KERNEL memory manager
 *
 * @param pointer The pointer to the memory to allocate
 * @param size The size of the memory to free - ignored in this implementation
 * @return The pointer to the memory
 */
void* operator new(size_t size, void* pointer) {

	return pointer;
}

/**
 * @brief Overloaded new operator, allocates memory using the KERNEL memory manager
 *
 * @param size The size of the memory to free - ignored in this implementation
 * @param pointer The pointer to the memory to allocate
 * @return The pointer to the memory
 */
void* operator new[](size_t size, void* pointer) {


	return pointer;
}

/**
 * @brief Overloaded delete operator, frees memory using the KERNEL memory manager
 *
 * @param pointer The pointer to the memory to free
 */
void operator delete(void* pointer) {

	// Handle the memory freeing
#ifdef MAXOS_KERNEL
	return MaxOS::memory::MemoryManager::kfree(pointer);
#else
	return MaxOS::KPI::free_memory(pointer);
#endif
}

/**
 * @brief Overloaded delete operator, frees memory using the KERNEL memory manager
 *
 * @param pointer The pointer to the memory to free
 */
void operator delete[](void* pointer) {

	// Handle the memory freeing
#ifdef MAXOS_KERNEL
	return MaxOS::memory::MemoryManager::kfree(pointer);
#else
	return MaxOS::KPI::free_memory(pointer);
#endif
}

/**
 * @brief Overloaded delete operator, frees memory using the KERNEL memory manager
 *
 * @param pointer The pointer to the memory to free
 * @param size The size of the memory to free - ignored in this implementation
 */
void operator delete(void* pointer, size_t) {

	// Handle the memory freeing
#ifdef MAXOS_KERNEL
	return MaxOS::memory::MemoryManager::kfree(pointer);
#else
	return MaxOS::KPI::free_memory(pointer);
#endif
}

/**
 * @brief Overloaded delete operator, frees memory using the KERNEL memory manager
 *
 * @param pointer The pointer to the memory to free
 * @param size The size of the memory to free - ignored in this implementation
 */
void operator delete[](void* pointer, size_t size) {

	// Handle the memory freeing
#ifdef MAXOS_KERNEL
	return MaxOS::memory::MemoryManager::kfree(pointer);
#else
	return MaxOS::KPI::free_memory(pointer);
#endif
}