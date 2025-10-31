//
// Created by 98max on 9/10/2025.
//
#include <common/symbols.h>

namespace MaxOS{

	namespace common{

		/**
		 * @brief Resolve an instruction pointer (rip) to a symbol name and optional offset.
		 *
		 * @param rip Instruction pointer
		 * @param link_base Where the kernel is linked at
		 * @param load_base Where the kernel is loaded at
		 *
		 * @return The symbol name
		 */
		const char* resolve_symbol(uintptr_t rip, uintptr_t link_base, uintptr_t load_base)
		{

			// No symbols
			if (KERNEL_SYMBOLS_COUNT == 0)
				return nullptr;

			// Convert load to linked
			uintptr_t offset = 0;
			if (load_base >= link_base)
				offset = load_base - link_base;

			// Get the adjusted address
			uintptr_t adjusted_address = rip;
			if (rip >= offset)
				adjusted_address = rip - offset;

			// Binary Search for symbol
			size_t left = 0;
			size_t right = KERNEL_SYMBOLS_COUNT;
			while (left < right) {
				size_t middle = (left + right) >> 1;
				if (kernel_symbols[middle].address <= adjusted_address)
					left = middle + 1;
				else
					right = middle;
			}

			return left == 0 ? nullptr : kernel_symbols[left - 1].name;
		}
	}
}