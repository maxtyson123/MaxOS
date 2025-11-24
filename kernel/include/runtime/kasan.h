/**
 * @file kasan.h
 * @brief Defines structures and a handler for Kernel Address Sanitizer (KASan) runtime errors
 *
 * @date 22nd November 2025
 * @author Max Tyson
 */

#ifndef MAXOS_RUNTIME_UBSAN_H
#define MAXOS_RUNTIME_UBSAN_H

#include <cstdint>
#include <cstddef>

namespace MaxOS::runtime {

	/**
	 * @class KASanHandler
	 * @brief Handles kernel address sanitizer runtime errors
	 *
	 * @todo finish when cbf
	 */
	class KASanHandler {

		public:
			KASanHandler();
			~KASanHandler();

			static void handle(bool write, uintptr_t address, size_t size, void* rip);

			static void initialize();
			static void poison_stack(uintptr_t address, size_t size);
			static void unpoison_stack(uintptr_t address, size_t size);
	};
}


#endif // MAXOS_RUNTIME_UBSAN_H
