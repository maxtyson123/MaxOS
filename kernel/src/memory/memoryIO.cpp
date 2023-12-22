//
// Created by 98max on 11/29/2022.
//

#include <memory/memoryIO.h>

using namespace maxOS::memory;

MemIO::MemIO(uint32_t address)
: m_address(address)
{

}

MemIO::~MemIO() {

}

MemIO8Bit::MemIO8Bit(uint32_t address)
: MemIO(address)
{
}

MemIO8Bit::~MemIO8Bit() {

}

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

MemIO16Bit::MemIO16Bit(uint32_t address)
: MemIO(address)
{
}

MemIO16Bit::~MemIO16Bit() {

}

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

MemIO32Bit::MemIO32Bit(uint32_t address)
        : MemIO(address)
{
}

MemIO32Bit::~MemIO32Bit() {

}

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

MemIO64Bit::MemIO64Bit(uint32_t address)
        : MemIO(address)
{
}

MemIO64Bit::~MemIO64Bit() {

}

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
void* maxOS::memory::memcpy(void* destination, const void* source, uint32_t num) {

    unsigned char* dst = (unsigned char*) destination;
    const unsigned char* src = (const unsigned char*) source;
    for (size_t i = 0; i < num; i++)
        dst[i] = src[i];
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
void* maxOS::memory::memset(void* ptr, int value, uint32_t num) {
    unsigned char* dst = (unsigned char*) ptr;
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
void* maxOS::memory::memmove(void* destination, const void* source, uint32_t num) {
    unsigned char* dst = (unsigned char*) destination;
    const unsigned char* src = (const unsigned char*) source;
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
int maxOS::memory::memcmp(const void* ptr1, const void* ptr2, uint32_t num) {
  const unsigned char *p1 = (const unsigned char *)ptr1;
  const unsigned char *p2 = (const unsigned char *)ptr2;
  for (size_t i = 0; i < num; i++) {
    if (p1[i] < p2[i])
      return -1;
    if (p1[i] > p2[i])
      return 1;
  }
  return 0;
}