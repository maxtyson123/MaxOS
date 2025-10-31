//
// Created by 98max on 24/11/2022.
//

#ifndef MAXOS_NET_ICMP_H
#define MAXOS_NET_ICMP_H

#include <net/ipv4.h>
#include <stdint.h>
#include <common/outputStream.h>

namespace MaxOS {

	namespace net {


		/**
		 * @struct InternetControlMessageProtocolHeader
		 * @brief The header of an ICMP packet
		 */
		struct InternetControlMessageProtocolHeader {
			uint8_t type;           ///< The type of ICMP message
			uint8_t code;           ///< The code of the ICMP message

			uint16_t checksum;      ///< Checksum to verify integrity
			uint32_t data;          ///< Payload data (Varies by type and code)
		}__attribute__((packed));

		/**
		 * @class InternetControlMessageProtocol
		 * @brief Handles ICMP packets
		 */
		class InternetControlMessageProtocol : InternetProtocolPayloadHandler {

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
