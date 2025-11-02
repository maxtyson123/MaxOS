/**
 * @file cplusplus.cpp
 * @brief Implementation of C++ runtime support functions
 *
 * @date 30th March 2025
 * @author Max Tyson
 */

#include <runtime/cplusplus.h>
#include <common/logger.h>

using namespace MaxOS;
using namespace MaxOS::runtime;

/// Define static constructors (DSO = Dynamic Shared Object)
extern "C" void* __dso_handle = nullptr;

/**
 * @brief Called when a virtual function is called on an object with no implementation
 */
extern "C" void __cxa_pure_virtual() {
	ASSERT(false, "Pure virtual function call failed");
}

/**
 * @brief Called when stack smashing is detected
 */
extern "C" void __stack_chk_fail(void)
{
	ASSERT(false, "Stack Smashed");
}

extern "C" constructor start_ctors; ///< Pointer to the start of the constructors section
extern "C" constructor end_ctors;   ///< Pointer to the end of the constructors section

/**
 * @brief Calls all global constructors
 */
extern "C" void call_constructors() {
	// Loop through and initialise all the global constructorsu
	for (constructor* i = &start_ctors; i != &end_ctors; i++)
		(*i)();
}

