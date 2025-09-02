//
// Created by 98max on 30/07/2025.
//

#include <common/buffer.h>

using namespace MaxOS;
using namespace MaxOS::common;

/**
 * @brief Creates a buffer of the specified size
 *
 * @param size
 */
Buffer::Buffer(size_t size, bool update_offset)
: m_capacity(size),
  update_offset(update_offset)
{

	// Create the buffer
	m_bytes = new uint8_t[size];

}

/**
 * @brief Creates a buffer pointing to a source
 *
 * @param source The source
 * @param size The capacity of the source
 */
Buffer::Buffer(void* source, size_t size, bool update_offset)
: update_offset(update_offset)
{

	m_bytes = (uint8_t*)source;
	m_capacity = size;
	m_dont_delete = true;

}

/**
 * @brief Destroy the buffer, freeing the memory
 */
Buffer::~Buffer() {
	if(!m_dont_delete && m_bytes)
		delete[] m_bytes;
}

/**
 * @brief The raw pointer to the bytes stored in memory, use is not recommended
 * @return The address of buffer's storage
 */
uint8_t *Buffer::raw() const{

	return m_bytes;
}

/**
 * @brief Fulls the buffer with 0's and resets the offset
 */
void Buffer::clear() {

	m_offset = 0;
	memset(m_bytes, 0, m_capacity);
}

/**
 * @brief Full the buffer with a specified byte at an offset
 *
 * @param byte The byte to write
 * @param offset Where to start writing (deafults to 0)
 * @param amount
 */
void Buffer::full(uint8_t byte, size_t offset, size_t amount) {

	// Prevent writing past the buffer bounds
	if (amount == 0) amount = m_capacity - m_offset - offset;
	ASSERT(m_offset + offset + amount <= capacity(), "Buffer overflow");

	memset(m_bytes + m_offset + offset, byte, amount);

}

/**
 * @brief The max bytes the buffer can currently store. @see Buffer:grow(size_t size) to increase this.
 * @return The length of the buffer storage array
 */
size_t Buffer::capacity() const {
	return m_capacity;
}

/**
 * @brief Grow the buffer to fit a new size
 *
 * @param size The new max capacity of the buffer
 */
void Buffer::resize(size_t size) {

	// Create the new buffer
	auto* new_buffer = new uint8_t[size];

	// Copy the old buffer
	if(m_bytes)
		memcpy(new_buffer, m_bytes, m_capacity);

	// Store the new buffer
	if(!m_dont_delete && m_bytes)
		delete[] m_bytes;

	m_bytes         = new_buffer;
	m_capacity      = size;
	m_dont_delete   = true;

}

/**
 * @brief Set the offset for where operations should begin from
 *
 * @param offset The new offset
 */
void Buffer::set_offset(size_t offset) {

	if(update_offset)
		m_offset = offset;

}

/**
 * @brief Safely writes a byte to the buffer at the current offset
 *
 * @param byte The byte to write
 */
void Buffer::write(uint8_t byte) {
	m_bytes[m_offset] = byte;
	set_offset(m_offset + 1);
}


/**
 * @brief Safely writes a byte to the buffer at the specified offset
 *
 * @param offset The offset into the buffer storage array
 * @param byte The byte to write
 */
void Buffer::write(size_t offset, uint8_t byte) {

	// Prevent writing past the buffer bounds
	ASSERT(m_offset + offset < capacity() && offset >= 0, "Buffer overflow");

	// Set the byte
	m_bytes[m_offset + offset] = byte;
	set_offset(m_offset + 1);

}

/**
 * @brief Safely reads a byte from the buffer at the current offset
 *
 * @param offset The offset into the buffer storage array
 */
uint8_t Buffer::read() {
	set_offset(m_offset + 1);
	return m_bytes[m_offset - 1];
}

/**
 * @brief Safely reads a byte from the buffer at the specified offset
 *
 * @param offset The offset into the buffer storage array
 */
uint8_t Buffer::read(size_t offset) {

	// Prevent writing past the buffer bounds
	ASSERT(m_offset + offset < capacity() && offset >= 0, "Buffer overflow");

	// Set the byte
	set_offset(m_offset + 1);
	return m_bytes[(m_offset - 1) + offset];
}

/**
 * @brief Copies all the bytes from another buffer into this buffer.
 *
 * @param buffer Where to read from
 */
void Buffer::copy_from(const Buffer* buffer) {

	// Copy the buffer
	ASSERT(buffer -> capacity() <= capacity(), "Copy exceeds buffer capacity");
	copy_from(buffer -> raw(), buffer -> capacity() );

}

/**
 * @brief Copies a range of bytes from another buffer into this buffer.
 *
 * @param buffer Where to read from
 * @param length How much to read
 */
void Buffer::copy_from(const Buffer *buffer, size_t length) {

	// Copy the buffer
	ASSERT(length <= buffer -> capacity(), "Copy exceeds external buffer capacity");
	copy_from(buffer -> raw(), length);

}

/**
 * @brief Copies a range of bytes from another buffer into this buffer at a specified offset.
 * @param source Where to read from
 * @param length How much to read
 * @param offset Where to start writing into this at
 */
void Buffer::copy_from(const Buffer *buffer, size_t length, size_t offset) {

	// Copy the buffer
	ASSERT(length <= buffer -> capacity(), "Copy exceeds external buffer capacity");
	copy_from(buffer -> raw(), length, offset);

}

/**
 * @brief Copies a range of bytes from another buffer into this buffer at a specified offset for both.
 *
 * @param source Where to read from
 * @param length How much to read
 * @param offset Where to start writing into this at
 * @param offset Where to start reading from the other buffer
 */
void Buffer::copy_from(const Buffer *buffer, size_t length, size_t offset, size_t offset_other) {

	// Copy the buffer
	ASSERT(length + offset_other <= buffer -> capacity(), "Copy exceeds external buffer capacity");
	copy_from(buffer -> raw() + offset_other, length, offset);

}

/**
 * @brief Copies a range of bytes from a source into this buffer.
 *
 * @param source Where to read from
 * @param length How much to read
 */
void Buffer::copy_from(void const *source, size_t length) {

	// Copy the bytes
	ASSERT(length + m_offset <= m_capacity, "Copy exceeds buffer capacity");
	memcpy(m_bytes + m_offset, source, length);
	set_offset(m_offset + length);
}

/**
 * @brief Copies a range of bytes from a source into this buffer at a specified offset.
 *
 * @param source Where to read from
 * @param length How much to read
 * @param offset Where to start writing at
 */
void Buffer::copy_from(void const *source, size_t length, size_t offset) {

	// Copy the bytes
	ASSERT((length  + offset + m_offset) <= m_capacity, "Copy exceeds buffer capacity");
	memcpy(m_bytes + offset + m_offset, source, length);
	set_offset(m_offset + length);

}

/**
 * @brief Copies all the bytes from this buffer into another buffer.
 *
 * @param buffer Where to write to
 */
void Buffer::copy_to(Buffer *buffer) {

	// Copy the buffer
	ASSERT(capacity() <= buffer -> capacity(), "Copy exceeds external buffer capacity");
	copy_to(buffer -> raw(), capacity());

}

/**
 * @brief Writes a range of bytes from this buffer into another buffer.
 *
 * @param buffer Where to write to
 * @param length How much to write
 */
void Buffer::copy_to(Buffer *buffer, size_t length) {

	// Copy the buffer
	ASSERT(length <= buffer -> capacity(), "Copy exceeds external buffer capacity");
	copy_to(buffer -> raw(), length);

}

/**
 * @brief Copies a range of bytes from this buffer into another buffer at a specified offset.
 *
 * @param source Where to write to
 * @param length How much to write
 * @param offset Where to start reading at
 */
void Buffer::copy_to(Buffer *buffer, size_t length, size_t offset) {

	// Copy the bytes
	ASSERT((length  + offset) <= buffer -> capacity(), "Copy exceeds external buffer capacity");
	copy_to(buffer -> raw(), length, offset);

}

/**
 * @brief Copies a range of bytes from this buffer into another at an offset for each.
 *
 * @param destination Where to write to
 * @param length How much to write
 * @param offset Where to start reading from
 * @param offset_other Where to start writing to
 */
void Buffer::copy_to(Buffer *buffer, size_t length, size_t offset, size_t offset_other) {

	// Copy the bytes
	ASSERT((length  + offset) <= buffer -> capacity(), "Copy exceeds external buffer capacity");
	copy_to(buffer -> raw() + offset_other, length, offset);

}


/**
 * @brief Copies a range of bytes from this buffer into a source.
 *
 * @param destination Where to write to
 * @param length How much to write
 */
void Buffer::copy_to(void* destination, size_t length) {

	// Copy the bytes
	ASSERT(length + m_offset <= (m_capacity), "Copy exceeds buffer capacity");
	memcpy(destination, m_bytes + m_offset, length);
	set_offset(m_offset + length);

}

/**
 * @brief Copies a range of bytes from this buffer at an offset into a source.
 *
 * @param destination Where to write to
 * @param length How much to write
 * @param offset Where to start reading from
 */
void Buffer::copy_to(void *destination, size_t length, size_t offset) {

	// Copy the bytes
	ASSERT((length  + offset + m_offset) <= m_capacity, "Copy exceeds buffer capacity");
	memcpy(destination, m_bytes + offset + m_offset, length);
	set_offset(m_offset + length);

}