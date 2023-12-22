//
// Created by 98max on 10/4/2022.
//

#include <hardwarecommunication/port.h>

using namespace maxOS::hardwarecommunication;

Port::Port(uint16_t port_number)
: m_port_number(port_number)
{
}

Port::~Port(){

}

Port8Bit::Port8Bit(uint16_t port_number)
: Port(port_number)
{
}

Port8Bit::~Port8Bit() {

}

/**
 * @brief write a byte to the port
 *
 * @param data the byte to write
 */
void Port8Bit::write(uint8_t data){
    __asm__ volatile("outb %0, %1" : : "a" (data), "Nd" (m_port_number));
}

/**
 * @brief read a byte from the port
 *
 * @return the byte read
 */
uint8_t Port8Bit::read(){
    uint8_t result;
    __asm__ volatile("inb %1, %0" : "=a" (result) : "Nd" (m_port_number));
    return result;
}

Port8BitSlow::Port8BitSlow(uint16_t port_number)
: Port8Bit(port_number)
{
}

Port8BitSlow::~Port8BitSlow() {

}

/**
 * @brief write a byte to the port (slow)
 *
 * @param data the byte to write
 */
void Port8BitSlow::write(uint8_t data){
    __asm__ volatile("outb %0, %1\njmp 1f\n1: jmp 1f\n1:" : : "a" (data), "Nd" (m_port_number));
}


Port16Bit::Port16Bit(uint16_t port_number)
: Port(port_number)
{
}

Port16Bit::~Port16Bit() {

}

/**
 * @brief write a word to the port
 *
 * @param data the word to write
 */
void Port16Bit::write(uint16_t data){
    __asm__ volatile("outw %0, %1" : : "a" (data), "Nd" (m_port_number));
}

/**
 * @brief read a word from the port
 *
 * @return the word read
 */
uint16_t Port16Bit::read(){
    uint16_t result;
    __asm__ volatile("inw %1, %0" : "=a" (result) : "Nd" (m_port_number));
    return result;
}


Port32Bit::Port32Bit(uint16_t port_number)
: Port(port_number)
{
}

Port32Bit::~Port32Bit() {

}

/**
 * @brief write a double word to the port (32Bit)
 *
 * @param data the double word to write
 */
void Port32Bit::write(uint32_t data){
    __asm__ volatile("outl %0, %1" : : "a" (data), "Nd" (m_port_number));
}

/**
 * @brief read a double word from the port (32Bit)
 *
 * @return the double word read
 */
uint32_t Port32Bit::read(){
    uint32_t result;
    __asm__ volatile("inl %1, %0" : "=a" (result) : "Nd" (m_port_number));
    return result;
}