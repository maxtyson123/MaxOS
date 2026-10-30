/**
 * @file arp.h
 * @brief Defines the Address Resolution Protocol (ARP) for resolving IP addresses to MAC addresses
 *
 * @date 11th November 2022
 * @author Max Tyson
 */

#ifndef MAXOS_NET_ARP_H
#define MAXOS_NET_ARP_H

#include <cstdint>

#include <net/ipv4.h>


namespace MaxOS::net {

	/**
	 * @struct ARPMessage
	 * @brief An ARP message
	 *
	 * @typedef arp_message_t
	 * @brief Alias for ARPMessage struct
	 */
	typedef struct PACKED ARPMessage {

		uint16_t hardware_type;          ///< The type of device that sent the ARP message (1 = Ethernet)
		uint16_t protocol;              ///< The protocol being used (0x0800 = IPv4)
		uint8_t hardware_address_size;    ///< The size of the hardware address (6 for MAC)
		uint8_t protocol_address_size;    ///< The size of the protocol address (4 for IPv4)

		uint16_t command;               ///< The ARP command (1 = request, 2 = reply)

		uint64_t src_mac : 48;            ///< The MAC address of the sender
		uint32_t src_ip;                 ///< The IP address of the sender
		uint64_t dst_mac : 48;            ///< The MAC address of the target
		uint32_t dst_ip;                 ///< The IP address of the target


	} arp_message_t;

	/**
	 * @class AddressResolutionProtocol
	 * @brief Handles ARP requests and replies
	 */
	class AddressResolutionProtocol : public EthernetFramePayloadHandler, public IPV4AddressResolver {

		private:
			common::Map<InternetProtocolAddress, drivers::ethernet::MediaAccessControlAddress> address_cache;
			InternetProtocolHandler* internet_protocol_handler;
			common::OutputStream* error_messages;

		public:
			AddressResolutionProtocol(EthernetFrameHandler* ethernet_frame_handler, InternetProtocolHandler* internet_protocol_handler, common::OutputStream* error_messages);
			~AddressResolutionProtocol();

			bool handle_ethernetframe_payload(uint8_t* etherframe_payload, uint32_t size) final;

			void request_mac_address(InternetProtocolAddress address);
			drivers::ethernet::MediaAccessControlAddress resolve(InternetProtocolAddress address) final;
			void store(InternetProtocolAddress internet_protocol_address, drivers::ethernet::MediaAccessControlAddress media_access_control_address) final;
	};

}


#endif //MAXOS_NET_ARP_H
