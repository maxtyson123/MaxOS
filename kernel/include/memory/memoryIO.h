/**
 * @file memoryIO.h
 * @brief Defines classes for memory input and output operations of various bit widths (8, 16, 32, 64 bits).
 *
 * @date 29th November 2022
 * @author Max Tyson
 */

#ifndef MAXOS_MEMORY_MEMORYINPUTOUTPUT_H
#define MAXOS_MEMORY_MEMORYINPUTOUTPUT_H

#include <cstdint>
#include <cstddef>


namespace MaxOS::memory {

	/**
	 * @class MemIO
	 * @brief base class for all memory IO
	 *
	 * @todo Not used, delete?
	 */
	class MemIO {
		protected:
			uintptr_t m_address;    ///< The memory address to read from / write to

			explicit MemIO(uintptr_t address);
			~MemIO();
	};

	/**
	 * @class MemIO8Bit
	 * @brief Handles 8 bit memory IO
	 */
	class MemIO8Bit : public MemIO {
		public:
			explicit MemIO8Bit(uintptr_t address);
			~MemIO8Bit();

			virtual void write(uint8_t data);
			virtual uint8_t read();
	};

	/**
	 * @class MemIO16Bit
	 * @brief Handles 16 bit memory IO
	 */
	class MemIO16Bit : public MemIO {
		public:
			explicit MemIO16Bit(uintptr_t address);
			~MemIO16Bit();

			virtual void write(uint16_t data);
			virtual uint16_t read();
	};

	/**
	 * @class MemIO32Bit
	 * @brief Handles 32 bit memory IO
	 */
	class MemIO32Bit : public MemIO {
		public:
			explicit MemIO32Bit(uintptr_t address);
			~MemIO32Bit();

			virtual void write(uint32_t data);
			virtual uint32_t read();
	};

	/**
	 * @class MemIO64Bit
	 * @brief Handles 64 bit memory IO
	 */
	class MemIO64Bit : public MemIO {
		public:
			explicit MemIO64Bit(uintptr_t address);
			~MemIO64Bit();

			virtual void write(uint64_t data);
			virtual uint64_t read();
	};

}

#endif //MAXOS_MEMORY_MEMORYINPUTOUTPUT_H
