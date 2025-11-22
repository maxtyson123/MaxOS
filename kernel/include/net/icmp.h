/**
 * @file icmp.h
 * @brief Defines the Internet Control Message Protocol (ICMP) handler for network communication.
 *
 * @date 24th November 2022
 * @author Max Tyson
 */

#ifndef MAXOS_NET_ICMP_H
#define MAXOS_NET_ICMP_H

#include <net/ipv4.h>
#include <cstdint>
#include <common/outputStream.h>


namespace MaxOS::net {


	/**
	 * @struct ICMPHeader
	 * @brief The header of an ICMP packet
	 *
	 * @typedef icmp_header_t
	 * @brief Alias for ICMPHeader struct
	 */
	typedef struct PACKED ICMPHeader {

		uint8_t type;           ///< The type of ICMP message
		uint8_t code;           ///< The code of the ICMP message

		uint16_t checksum;      ///< checksum to verify integrity
		uint32_t data;          ///< Payload data (Varies by type and code)

	} icmp_header_t;

	/**
	 * @class InternetControlMessageProtocol
	 * @brief Handles ICMP packets
	 */
	class InternetControlMessageProtocol : IPV4PayloadHandler {

			common::OutputStream* error_messages;

		public:
			InternetControlMessageProtocol(InternetProtocolHandler* internet_protocol_handler, common::OutputStream* error_messages);
			~InternetControlMessageProtocol();

			bool handle_internet_protocol_payload(net::InternetProtocolAddress src_ip_be, net::InternetProtocolAddress dst_ip_be, uint8_t* payload_data, uint32_t size) final;
			void request_echo_reply(uint32_t ip_be);
	};

}


#endif //MAXOS_NET_ICMP_H
