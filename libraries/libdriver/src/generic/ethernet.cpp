/**
 * @file ethernet.cpp
 * @brief Implementation of an Ethernet driver interface
 *
 * @date 1st December 2022
 * @author Max Tyson
 */

#include <generic/ethernet.h>

using namespace LibDriver;
using namespace LibDriver::generic;

EthernetDriver::EthernetDriver() = default;
EthernetDriver::~EthernetDriver() = default;

/**
 * @brief Get the MAC address
 *
 * @return the MAC address
 */
MediaAccessControlAddress EthernetDriver::get_media_access_control_address() {
	return 0;
}

/**
 * @brief send data to the network via the driver backend
 *
 * @param buffer  The buffer to send
 * @param size The size of the buffer
 */
void EthernetDriver::send(uint8_t* buffer, uint32_t size) {

	// Raise the event
//	raise_event(new BeforeSendEvent(buffer, size));

	do_send(buffer, size);
}

/**
 * @brief (Device Side) send the data
 *
 * @param buffer The buffer to handle
 * @param size The size of the buffer
 */
void EthernetDriver::do_send(uint8_t* buffer, uint32_t size) {
}


// if your mac address is e.g. 1c:6f:65:07:ad:1a (see output of ifconfig)
// then you would call CreateMediaAccessControlAddress(0x1c, 0x6f, 0x65, 0x07, 0xad, 0x1a)
/**
 * @brief Create a Media Access Control Address
 *
 * @param digit1 The m_first_memory_chunk digit
 * @param digit2 The second digit
 * @param digit3 The third digit
 * @param digit4 The fourth digit
 * @param digit5 The fifth digit
 * @param digit6 The last digit
 * @return The MAC address
 */
MediaAccessControlAddress EthernetDriver::create_media_access_control_address(uint8_t digit1, uint8_t digit2, uint8_t digit3, uint8_t digit4, uint8_t digit5, uint8_t digit6) {
	return // digit6 is the most significant byte
			(uint64_t) digit6 << 40
			| (uint64_t) digit5 << 32
			| (uint64_t) digit4 << 24
			| (uint64_t) digit3 << 16
			| (uint64_t) digit2 << 8
			| (uint64_t) digit1;
}