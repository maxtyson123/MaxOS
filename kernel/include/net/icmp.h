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
#include <stdint.h>
#include <common/outputStream.h>

namespace MaxOS {

	namespace net {


		/**
		 * @struct ICMPHeader
		 * @brief The header of an ICMP packet
		 */
		struct ICMPHeader {
			uint8_t type;           ///< The type of ICMP message
			uint8_t code;           ///< The code of the ICMP message

			uint16_t checksum;      ///< Checksum to verify integrity
			uint32_t data;          ///< Payload data (Varies by type and code)
		}__attribute__((packed));

		/**
		 * @class InternetControlMessageProtocol
		 * @brief Handles ICMP packets
		 */
		class InternetControlMessageProtocol : IPV4PayloadHandler {

				common::OutputStream* errorMessages;

			public:
				InternetControlMessageProtocol(InternetProtocolHandler* internetProtocolHandler, common::OutputStream* errorMessages);
				~InternetControlMessageProtocol();

				bool handleInternetProtocolPayload(InternetProtocolAddress sourceIP, InternetProtocolAddress destinationIP, uint8_t* payloadData, uint32_t size) final;
				void RequestEchoReply(InternetProtocolAddress ip_be);
		};

	}

}

#endif //MAXOS_NET_ICMP_H
