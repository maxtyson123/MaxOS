//
// Created by 98max on 11/11/2022.
//

#ifndef MAXOS_NET_ARP_H
#define MAXOS_NET_ARP_H

#include <stdint.h>

#include <net/ipv4.h>

namespace MaxOS {

	namespace net {

		/**
		 * @struct AddressResolutionProtocolMessage
		 * @brief An ARP message
		 */
		struct AddressResolutionProtocolMessage {

			uint16_t hardwareType;          ///< The type of device that sent the ARP message (1 = Ethernet)
			uint16_t protocol;              ///< The protocol being used (0x0800 = IPv4)
			uint8_t hardwareAddressSize;    ///< The size of the hardware address (6 for MAC)
			uint8_t protocolAddressSize;    ///< The size of the protocol address (4 for IPv4)

			uint16_t command;               ///< The ARP command (1 = request, 2 = reply)

			uint64_t srcMAC: 48;            ///< The MAC address of the sender
			uint32_t srcIP;                 ///< The IP address of the sender
			uint64_t dstMAC: 48;            ///< The MAC address of the target
			uint32_t dstIP;                 ///< The IP address of the target


		}__attribute__((packed));

		/**
		 * @class AddressResolutionProtocol
		 * @brief Handles ARP requests and replies
		 */
		class AddressResolutionProtocol : public EthernetFramePayloadHandler, public InternetProtocolAddressResolver {

			private:
				common::Map<InternetProtocolAddress, drivers::ethernet::MediaAccessControlAddress> addressCache;
				InternetProtocolHandler* internetProtocolHandler;
				common::OutputStream* errorMessages;

			public:
				AddressResolutionProtocol(EthernetFrameHandler* ethernetFrameHandler, InternetProtocolHandler* internetProtocolHandler, common::OutputStream* errorMessages);
				~AddressResolutionProtocol();

				bool handleEthernetframePayload(uint8_t* data, uint32_t size);

				void RequestMACAddress(InternetProtocolAddress address);
				drivers::ethernet::MediaAccessControlAddress Resolve(InternetProtocolAddress address) final;
				void Store(InternetProtocolAddress internetProtocolAddress, drivers::ethernet::MediaAccessControlAddress mediaAccessControlAddress) final;
		};

	}

}


#endif //MAXOS_NET_ARP_H
