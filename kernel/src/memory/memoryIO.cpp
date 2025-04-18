//
// Created by 98max on 11/29/2022.
//

#include <memory/memoryIO.h>

using namespace MaxOS::memory;

MemIO::MemIO(uintptr_t address)
: m_address(address)
{

}

MemIO::~MemIO() = default;

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
void MemIO8Bit::write(uint8_t data){
    (*((volatile uint8_t*)(m_address)))=(data);
}

/**
 * @brief Reads data from the memory address
 *
 * @return the data read
 */
uint8_t MemIO8Bit::read(){
    return *((volatile uint8_t*)(m_address));
}

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
void MemIO16Bit::write(uint16_t data){
    (*((volatile uint16_t*)(m_address)))=(data);
}

/**
 * @brief Reads data from the memory address
 *
 * @return the data read
 */
uint16_t MemIO16Bit::read(){
    return *((volatile uint16_t*)(m_address));
}

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
void MemIO32Bit::write(uint32_t data){
    (*((volatile uint32_t*)(m_address)))=(data);
}

/**
 * @brief Reads data from the memory address
 *
 * @return the data read
 */
uint32_t MemIO32Bit::read(){
    return *((volatile uint32_t*)(m_address));
}

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
void MemIO64Bit::write(uint64_t data){
    (*((volatile uint64_t*)(m_address)))=(data);
}

/**
 * @brief Reads data from the memory address
 *
 * @return the data read
 */
uint64_t MemIO64Bit::read(){
    return *((volatile uint64_t*)(m_address));
}


/**
 * @brief Copies a block of memory from one location to another
 *
 * @see https://wiki.osdev.org/Meaty_Skeleton#memcpy.28.29
 *
 * @param destination The destination to copy to
 * @param source The source to copy from
 * @param num The number of bytes to copy
 * @return The destination
 */
void* memcpy(void* destination, const void* source, uint64_t num) {

    // Make sure the source and destination are not the same
    if (destination == source)
        return destination;

    // Make sure they exist
    if (destination == nullptr || source == nullptr)
            return destination;

    // Get the source and destination
    auto* dst = (unsigned char*) destination;
    const auto* src = (const unsigned char*) source;

    // Copy the data
    for (size_t i = 0; i < num; i++)
        dst[i] = src[i];

    // Usefully for easier code writing
    return destination;
}

/**
 * @brief Fills a block of memory with a specified value

 *
 * @param ptr The pointer to the block of memory
 * @param value The value to fill the block of memory with
 * @param num The number of bytes to fill
 * @return The pointer to the block of memory
 */
void* memset(void* ptr, uint32_t value, uint64_t num) {

    // Make sure the pointer exists
    if (ptr == nullptr)
        return ptr;

    auto* dst = (unsigned char*) ptr;
    for (size_t i = 0; i < num; i++)
        dst[i] = (unsigned char) value;
    return ptr;
}

/**
 * @brief Copies a block of memory from one location to another
 *
 * @param destination The destination to copy to
 * @param source The source to copy from
 * @param num The number of bytes to copy
 * @return The destination
 */
void* memmove(void* destination, const void* source, uint64_t num) {

  // Make sure the source and destination are not the same
  if (destination == source)
    return destination;

  // Make sure they exist
  if (destination == nullptr || source == nullptr)
    return destination;

  auto* dst = (unsigned char*) destination;
  const auto* src = (const unsigned char*) source;
  if (dst < src) {
      for (size_t i = 0; i < num; i++)
          dst[i] = src[i];
  } else {
      for (size_t i = num; i != 0; i--)
          dst[i-1] = src[i-1];
  }
    return destination;
}

/**
 * @brief Compares two blocks of memory
 *
 * @param ptr1 The m_first_memory_chunk block of memory
 * @param ptr2 The second block of memory
 * @param num The number of bytes to compare
 * @return 0 if the blocks of memory are equal, -1 if ptr1 < ptr2, 1 if ptr1 > ptr2
 */
int memcmp(const void* ptr1, const void* ptr2, uint64_t num) {

  // Make sure the pointers exist
  if (ptr1 == nullptr || ptr2 == nullptr)
      return 0;

  const auto *p1 = (const unsigned char *)ptr1;
  const auto *p2 = (const unsigned char *)ptr2;
  for (size_t i = 0; i < num; i++) {
    if (p1[i] < p2[i])
      return -1;
    if (p1[i] > p2[i])
      return 1;
  }
  return 0;
}