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

#include <stdint.h>

namespace MaxOS {
	namespace runtime {

		typedef void (* constructor)();
		typedef unsigned uarch_t;

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

			int __cxa_atexit(void (* f)(void*), void* objptr, void* dso);
			void __cxa_finalize(void* f);

			// Stack Gaurd
			uintptr_t __stack_chk_guard = 0x595e9fbd94fda766;
			void __stack_chk_fail(void);

		}
	}
}

#endif // MAXOS_RUNTIME_CPLUSPLUS_H
