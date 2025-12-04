/**
 * @file cplusplus.h
 * @brief Defines C++ runtime functions and structures needed for linking C++ code in the kernel
 *
 * @see https://wiki.osdev.org/C%2B%2B - code adapted from here
 *
 * @date 30th March 2025
 * @author Max Tyson
 */

#ifndef MAXOS_RUNTIME_CPLUSPLUS_H
#define MAXOS_RUNTIME_CPLUSPLUS_H

#include <cstdint>
#include <assert.h>

namespace MaxOS::runtime {

	typedef void (* constructor)();     ///< A pointer to a constructor function
	typedef unsigned uarch_t;           ///< An unsigned architecture specific type

	extern "C" {

	/**
	 * @struct atexit_func_entry_t
	 * @brief An entry in the list of functions to call at program exit (not used in kernel but needed for linking)
	 */
	struct atexit_func_entry_t {
		void (* destructor_func)(void*);        ///< The destructor function to call
		void* obj_ptr;                          ///< The pointer to the object to destroy
		void* dso_handle;                       ///< The DSO handle (not used)
	};

	/**
	 * @brief Register a function to be called at program exit
	 *
	 * @param f The function to call
	 * @param objptr The object pointer to pass to the function
	 * @param dso The DSO handle (not used)
	 * @return int 0 on success, non-zero on failure
	 */
	int __cxa_atexit(void (* f)(void*), void* objptr, void* dso);

	/**
	 * @brief Finalise all global objects by calling their destructors
	 *
	 * @param f The DSO handle to finalise (nullptr for all)
	 */
	void __cxa_finalize(void* f);

	/// The stack canary value used to detect stack buffer overflows (@todo make random at runtime)
	uintptr_t __stack_chk_guard = 0x595e9fbd94fda766;

	/**
	 * @brief Called when a stack buffer overflow is detected
	 */
	void __stack_chk_fail(void);
	}
}


#endif // MAXOS_RUNTIME_CPLUSPLUS_H
