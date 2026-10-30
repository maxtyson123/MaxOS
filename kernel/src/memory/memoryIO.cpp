/**
 * @file memoryIO.cpp
 * @brief Implementation of memory-mapped I/O operations
 *
 * @date 29th November 2022
 * @author Max Tyson
 */

#include <memory/memoryIO.h>

using namespace MaxOS::memory;

/**
 * @brief Construct a new Mem IO object
 *
 * @param address The memory address to read from / write to
 */
MemIO::MemIO(uintptr_t address)
: m_address(address)
{

}

MemIO::~MemIO() = default;

/**
 * @brief Construct a new Mem IO object for 8 bit reads/writes
 *
 * @param address The memory address to read from / write to
 */
MemIO8Bit::MemIO8Bit(uintptr_t address)
: MemIO(address)
{
}

MemIO8Bit::~MemIO8Bit() = default;

/**
 * @brief Writes data to the memory address
 *
 * @param data  the data to write
 */
void MemIO8Bit::write(uint8_t data) {

	(*((volatile uint8_t*) (m_address))) = (data);
}

/**
 * @brief Reads data from the memory address
 *
 * @return the data read
 */
uint8_t MemIO8Bit::read() {

	return *((volatile uint8_t*) (m_address));
}

/**
 * @brief Construct a new Mem IO object for 16 bit reads/writes
 *
 * @param address The memory address to read from / write to
 */
MemIO16Bit::MemIO16Bit(uintptr_t address)
: MemIO(address)
{
}

MemIO16Bit::~MemIO16Bit() = default;

/**
 * @brief Writes data to the memory address
 *
 * @param data  the data to write
 */
void MemIO16Bit::write(uint16_t data) {

	(*((volatile uint16_t*) (m_address))) = (data);
}

/**
 * @brief Reads data from the memory address
 *
 * @return the data read
 */
uint16_t MemIO16Bit::read() {

	return *((volatile uint16_t*) (m_address));
}

/**
 * @brief Construct a new Mem IO object for 32 bit reads/writes
 *
 * @param address The memory address to read from / write to
 */
MemIO32Bit::MemIO32Bit(uintptr_t address)
: MemIO(address)
{
}

MemIO32Bit::~MemIO32Bit() = default;

/**
 * @brief Writes data to the memory address
 *
 * @param data  the data to write
 */
void MemIO32Bit::write(uint32_t data) {

	(*((volatile uint32_t*) (m_address))) = (data);
}

/**
 * @brief Reads data from the memory address
 *
 * @return the data read
 */
uint32_t MemIO32Bit::read() {

	return *((volatile uint32_t*) (m_address));
}

/**
 * @brief Construct a new Mem IO object for 64 bit reads/writes
 *
 * @param address The memory address to read from / write to
 */
MemIO64Bit::MemIO64Bit(uintptr_t address)
: MemIO(address)
{
}

MemIO64Bit::~MemIO64Bit() = default;

/**
 * @brief Writes data to the memory address
 *
 * @param data  the data to write
 */
void MemIO64Bit::write(uint64_t data) {

	(*((volatile uint64_t*) (m_address))) = (data);
}

/**
 * @brief Reads data from the memory address
 *
 * @return the data read
 */
uint64_t MemIO64Bit::read() {

	return *((volatile uint64_t*) (m_address));
}