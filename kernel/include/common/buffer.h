/**
 * @file buffer.h
 * @brief Defines a Buffer class for safe memory operations
 *
 * @date 30th July 2025
 * @author Max Tyson
 */

#ifndef MAXOS_COMMON_BUFFER_H
#define MAXOS_COMMON_BUFFER_H

#include <cstdint>
#include <cstddef>
#include <memory/memoryIO.h>
#include <common/logger.h>


namespace MaxOS::common {

	/**
	 * @class Buffer
	 * @brief Wrapper class for a region of bytes in memor in an attempt to add some memory safety. Automatically
	 * allocates the size specified and frees it once done, adds boundary to I/O.
	 */
	class Buffer {

		private:
			uint8_t* m_bytes = nullptr;
			size_t m_capacity;
			bool m_dont_delete = false;

			size_t m_offset = 0;

		public:
			explicit Buffer(size_t size, bool update_offset = true);
			Buffer(void* source, size_t size, bool update_offset = true);
			~Buffer();

			[[nodiscard]] uint8_t* raw() const;

			void clear();
			void full(uint8_t byte, size_t offset = 0, size_t amount = 0);

			bool update_offset = true; ///< Should operations write/read/copy update the offset in the buffer?
			void set_offset(size_t offset);

			[[nodiscard]] size_t capacity() const;
			void resize(size_t size);

			void write(uint8_t byte);
			void write(size_t offset, uint8_t byte);
			uint8_t read();
			uint8_t read(size_t offset);

			void copy_from(const Buffer* buffer);
			void copy_from(const Buffer* buffer, size_t length);
			void copy_from(const Buffer* buffer, size_t length, size_t offset);
			void copy_from(const Buffer* buffer, size_t length, size_t offset, size_t offset_other);
			void copy_from(const void* source, size_t length);
			void copy_from(const void* source, size_t length, size_t offset);

			void copy_to(Buffer* buffer);
			void copy_to(Buffer* buffer, size_t length);
			void copy_to(Buffer* buffer, size_t length, size_t offset);
			void copy_to(Buffer* buffer, size_t length, size_t offset, size_t offset_other);
			void copy_to(void* destination, size_t length);
			void copy_to(void* destination, size_t length, size_t offset);
	};

	typedef Buffer buffer_t;    ///< Typedef for Buffer class

}


#endif //MAXOS_COMMON_BUFFER_H
