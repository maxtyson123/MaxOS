//
// Created by 98max on 1/11/2025.
//

#ifndef MAXOS_COMMON_MACROS_H
#define MAXOS_COMMON_MACROS_H

namespace MaxOS{

	#ifdef DOXYGEN
		/// Ensure no padding added to the struct
		#define PACKED
	#else
		/// Ensure no padding added to the struct
		#define PACKED __attribute__((packed))
	#endif

	#ifdef DOXYGEN
		/// Align to page size (4096 bytes)
		#define PAGE_ALIGNED
	#else
		/// Align to page size (4096 bytes)
		#define PAGE_ALIGNED __attribute__((aligned(4096)))
	#endif

	#define CONCATENATE(a, b) CONCATENATE_IMPL(a, b)
	#define CONCATENATE_IMPL(a, b) a##b
}



#endif //MAXOS_COMMON_MACROS_H
