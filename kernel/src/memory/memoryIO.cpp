//
// Created by 98max on 11/29/2022.
//

#include <memory/memoryIO.h>

using namespace maxOS::memory;

MemIO::MemIO(uint64_t address) {
    this->address = address;

}

MemIO::~MemIO() {

}


// 8 BIT

MemIO8Bit::MemIO8Bit(uint64_t address)
        : MemIO(address)
{
}

MemIO8Bit::~MemIO8Bit() {

}

/**
 * @details Writes data to the memory address
 *
 * @param data  the data to write
 */
void MemIO8Bit::Write(uint8_t data){
    (*((volatile uint8_t*)(address)))=(data);
}

/**
 * @details Reads data from the memory address
 *
 * @return the data read
 */

uint8_t MemIO8Bit::Read(){
    return *((volatile uint8_t*)(address));
}

//16 BIT

MemIO16Bit::MemIO16Bit(uint64_t address)
        : MemIO(address)
{
}

MemIO16Bit::~MemIO16Bit() {

}

/**
 * @details Writes data to the memory address
 *
 * @param data  the data to write
 */
void MemIO16Bit::Write(uint16_t data){
    (*((volatile uint16_t*)(address)))=(data);
}

/**
 * @details Reads data from the memory address
 *
 * @return the data read
 */

uint16_t MemIO16Bit::Read(){
    return *((volatile uint16_t*)(address));
}

//32 BIT

MemIO32Bit::MemIO32Bit(uint64_t address)
        : MemIO(address)
{
}

MemIO32Bit::~MemIO32Bit() {

}

/**
 * @details Writes data to the memory address
 *
 * @param data  the data to write
 */
void MemIO32Bit::Write(uint32_t data){
    (*((volatile uint32_t*)(address)))=(data);
}

/**
 * @details Reads data from the memory address
 *
 * @return the data read
 */

uint32_t MemIO32Bit::Read(){
    return *((volatile uint32_t*)(address));
}

//64 BIT

MemIO64Bit::MemIO64Bit(uint64_t address)
        : MemIO(address)
{
}

MemIO64Bit::~MemIO64Bit() {

}

/**
 * @details Writes data to the memory address
 *
 * @param data  the data to write
 */
void MemIO64Bit::Write(uint64_t data){
    (*((volatile uint64_t*)(address)))=(data);
}

/**
 * @details Reads data from the memory address
 *
 * @return the data read
 */
uint64_t MemIO64Bit::Read(){
    return *((volatile uint64_t*)(address));
}

/**
 * Memcpy from https://wiki.osdev.org/Meaty_Skeleton
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
