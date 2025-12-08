/**
 * @file ethernet.h
 * @brief Defines an EthernetDriver class for managing Ethernet communication, including sending and receiving data, handling MAC addresses, and event management.
 *
 * @date 1st December 2022
 * @author Max Tyson
 */

#ifndef LIBDRIVER_GENERIC_ETHERNET_H
#define LIBDRIVER_GENERIC_ETHERNET_H

#include <cstdint>
#include <driver.h>

namespace LibDriver::generic {

	/// Used to make MAC addresses more readable @todo make a MacAddress class and use mac_t
	typedef uint64_t MediaAccessControlAddress;

	/**
	 * @class EthernetDriver
	 * @brief Driver for the Ethernet Controller, manages the sending and receiving of data, the mac address, and the events
	 */
	class EthernetDriver : public LibDriver::Driver {
		protected:
			virtual void do_send(uint8_t* buffer, uint32_t size);

		public:
			EthernetDriver();
			~EthernetDriver();

			static MediaAccessControlAddress create_media_access_control_address(uint8_t digit1, uint8_t digit2, uint8_t digit3, uint8_t digit4, uint8_t digit5, uint8_t digit6);
			virtual MediaAccessControlAddress get_media_access_control_address();

			void send(uint8_t* buffer, uint32_t size);
	};

}


#endif //LIBDRIVER_GENERIC_ETHERNET_H
