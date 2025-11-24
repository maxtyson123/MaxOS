/**
 * @file kasan.cpp
 * @brief Implementation of the Kernel Address Sanitizer (KASan) handler
 *
 * @date 22nd November 2025
 * @author Max Tyson
 */

#include <runtime/kasan.h>
#include <common/logger.h>

using namespace MaxOS;
using namespace MaxOS::runtime;

KASanHandler::KASanHandler() = default;

KASanHandler::~KASanHandler() = default;

/**
 * @brief Handles a KASan error by panicking and printing the details
 *
 * @param write Whether the access was a write (true) or read (false)
 * @param address The address that was accessed
 * @param size The size of the access in bytes
 * @param rip The instruction pointer where the access occurred
 */
void KASanHandler::handle(bool write, uintptr_t address, size_t size, void* rip) {
	ASSERT(false, "KASAN ERROR: %s of size %zu at address 0x%lx from RIP 0x%p", write ? "WRITE" : "READ", size, address, rip);
}

/**
 * @brief Initializes the KASan handler by setting up the shadow memory
 */
void KASanHandler::initialize() {

}

/**
 * @brief Marks a region of stack memory as poisoned (inaccessible)
 *
 * @param address The starting address of the region
 * @param size The size of the region in bytes
 */
void KASanHandler::poison_stack(uintptr_t address, size_t size) {

}

/**
 * @brief Marks a region of stack memory as unpoisoned (accessible)
 *
 * @param address The starting address of the region
 * @param size The size of the region in bytes
 */
void KASanHandler::unpoison_stack(uintptr_t address, size_t size) {

}

/**
 * @brief Triggered when a memory load access violation of 1 byte is detected
 *
 * @param address The address that was accessed
 */
extern "C" void __asan_load1_noabort(uintptr_t address) {
	KASanHandler::handle(false, address, 1, __builtin_return_address(0));
}

/**
 * @brief Triggered when a memory load access violation of 2 bytes is detected
 *
 * @param address The address that was accessed
 */
extern "C" void __asan_load2_noabort(uintptr_t address) {
	KASanHandler::handle(false, address, 2, __builtin_return_address(0));
}

/**
 * @brief Triggered when a memory load access violation of 4 bytes is detected
 *
 * @param address The address that was accessed
 */
extern "C" void __asan_load4_noabort(uintptr_t address) {
	KASanHandler::handle(false, address, 4, __builtin_return_address(0));
}

/**
 * @brief Triggered when a memory load access violation of 8 bytes is detected
 *
 * @param address The address that was accessed
 */
extern "C" void __asan_load8_noabort(uintptr_t address) {
	KASanHandler::handle(false, address, 8, __builtin_return_address(0));
}

/**
 * @brief Triggered when a memory load access violation of 16 bytes is detected
 *
 * @param address The address that was accessed
 */
extern "C" void __asan_load16_noabort(uintptr_t address) {
	KASanHandler::handle(false, address, 16, __builtin_return_address(0));
}

/**
 * @brief Triggered when a memory load access violation of arbitrary size is detected
 *
 * @param address The address that was accessed
 * @param size The size of the access in bytes
 */
extern "C" void __asan_loadN_noabort(uintptr_t address, size_t size) {
	KASanHandler::handle(false, address, size, __builtin_return_address(0));
}

/**
 * @brief Triggered when a memory store access violation of 1 byte is detected
 *
 * @param address The address that was accessed
 */
extern "C" void __asan_store1_noabort(uintptr_t address) {
	KASanHandler::handle(true, address, 1, __builtin_return_address(0));
}

/**
 * @brief Triggered when a memory store access violation of 2 bytes is detected
 *
 * @param address The address that was accessed
 */
extern "C" void __asan_store2_noabort(uintptr_t address) {
	KASanHandler::handle(true, address, 2, __builtin_return_address(0));
}

/**
 * @brief Triggered when a memory store access violation of 4 bytes is detected
 *
 * @param address The address that was accessed
 */
extern "C" void __asan_store4_noabort(uintptr_t address) {
	KASanHandler::handle(true, address, 4, __builtin_return_address(0));
}

/**
 * @brief Triggered when a memory store access violation of 8 bytes is detected
 *
 * @param address The address that was accessed
 */
extern "C" void __asan_store8_noabort(uintptr_t address) {
	KASanHandler::handle(true, address, 8, __builtin_return_address(0));
}

/**
 * @brief Triggered when a memory store access violation of 16 bytes is detected
 *
 * @param address The address that was accessed
 */
extern "C" void __asan_store16_noabort(uintptr_t address) {
	KASanHandler::handle(true, address, 16, __builtin_return_address(0));
}

/**
 * @brief Triggered when a memory store access violation of arbitrary size is detected
 *
 * @param address The address that was accessed
 * @param size The size of the access in bytes
 */
extern "C" void __asan_storeN_noabort(uintptr_t address, size_t size) {
	KASanHandler::handle(false, address, size, __builtin_return_address(0));
}

/**
 * @brief Triggered when a function that is expected to not return does return
 */
extern "C" void __asan_handle_no_return() {
	ASSERT(false, "KASAN ERROR: Function marked 'noreturn' has returned");
}

/**
 * @brief Poisons a region of stack memory allocated with alloca
 *
 * @param address The starting address of the allocated memory
 * @param size The size of the allocated memory in bytes
 */
extern "C" void __asan_alloca_poison(uintptr_t address, size_t size) {
	//TODO: Implement stack poisoning
	ASSERT(false, "KASAN ERROR: Stack poisoning not implemented for address 0x%lx of size %zu", address, size);
}

/**
 * @brief Unpoisons a region of stack memory allocated with alloca
 *
 * @param stack_top The top address of the allocated memory
 * @param stack_bottom The bottom address of the allocated memory
 */
extern "C" void __asan_alloca_unpoison(void* stack_top, void* stack_bottom) {
	//TODO: Implement stack unpoisoning
	ASSERT(false, "KASAN ERROR: Stack unpoisoning not implemented for range 0x%p - 0x%p", stack_bottom, stack_top);
}

/**
 * @brief Unpoisons a region of memory, marking it as accessible
 *
 * @param pointer The starting address of the memory
 * @param size The size of the memory region in bytes
 */
extern "C" void __asan_set_shadow_00(void* pointer, size_t size) {

	// Clear the memory to mark it as unpoisoned
	for(int i = 0; i < size; ++i)
		((int8_t*)pointer)[i] = 0;

}

/**
 * @brief Poisons a region of memory, marking it as inaccessible
 *
 * @param pointer The starting address of the memory
 * @param size The size of the memory region in bytes
 */
extern "C" void __asan_set_shadow_f8(void *pointer, size_t size){

	// Fill the memory to mark it as poisoned
	for(int i = 0; i < size; ++i)
		((int8_t*)pointer)[i] = 0xF8;

}

/**
 * @brief Called before dynamic initialization of a module
 *
 * @param module_name The name of the module being initialized
 */
extern "C" void __asan_before_dynamic_init(const void *module_name) {
	// Nothing to do
}

/**
 * @brief Called after dynamic initialization of a module
 *
 * @param module_name The name of the module that was initialized
 */
 extern "C" void __asan_after_dynamic_init(const void *module_name) {
	// Nothing to do
 }