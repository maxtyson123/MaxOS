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

}



#endif //MAXOS_COMMON_MACROS_H
