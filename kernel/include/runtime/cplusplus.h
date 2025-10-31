//
// Created by 98max on 30/03/2025.
//

#ifndef MAXOS_RUNTIME_CPLUSPLUS_H
#define MAXOS_RUNTIME_CPLUSPLUS_H

#include <stdint.h>

namespace MaxOS {
	namespace runtime {


		// Thanks: https://wiki.osdev.org/C%2B%2B


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
