/**
 * @file ipv4.h
 * @brief Defines classes and structures for handling Internet Protocol version 4 (IPv4) packets
 *
 * @date 22nd November 2022
 * @author Max Tyson
 */

#ifndef MAXOS_NET_IPV4_H
#define MAXOS_NET_IPV4_H

#include <stdint.h>
#include <net/ethernetframe.h>

namespace MaxOS {

	namespace net {

		typedef uint32_t InternetProtocolAddress;   ///< An IPv4 address @todo: Make IPv4Address class and do ip_t
		typedef uint32_t SubnetMask;                ///< A subnet mask @todo: Make SubnetMask class and do subnetmask_t

		/**
		 * @struct IPV4Header
		 * @brief The header of an IPv4 packet
		 *
		 * @typedef ipv4_header_t
		 * @brief Alias for IPV4Header struct
		 */
		typedef struct PACKED IPV4Header {

			uint8_t headerLength: 4;        ///< The length of the header in 32-bit words (min 5, max 15)
			uint8_t version: 4;             ///< The version of the IP protocol (4 for IPv4)
			uint8_t typeOfService;          ///< The type of service (low delay, high throughput, high reliability, etc. @todo: enum
			uint16_t totalLength;           ///< The total length of the IP packet (header + data) in bytes

			uint16_t identifier;            ///< Unique identifier for the group of fragments of a single IP packet
			uint16_t flagsAndOffset;        ///< Flags (3 bits) and Fragment Offset (13 bits) @todo: bitfield struct?

			uint8_t timeToLive;             ///< The maximum hops between devices before the packet is discarded
			uint8_t protocol;               ///< The protocol of the payload (TCP, UDP, ICMP, etc. @todo: enum
			uint16_t checksum;              ///< The checksum of the header

			uint32_t sourceIP;              ///< The IP of the sender
			uint32_t destinationIP;         ///< The IP of the receiver

		} ipv4_header_t;

		class InternetProtocolHandler;

		/**
		 * @class IPV4AddressResolver
		 * @brief Resolves IP addresses to MAC addresses
		 */
		class IPV4AddressResolver {
			public:
				IPV4AddressResolver(InternetProtocolHandler* internetProtocolHandler);
				~IPV4AddressResolver();
				virtual drivers::ethernet::MediaAccessControlAddress Resolve(InternetProtocolAddress address);
				virtual void Store(InternetProtocolAddress internetProtocolAddress, drivers::ethernet::MediaAccessControlAddress mediaAccessControlAddress);
		};


		/**
		 * @class IPV4PayloadHandler
		 * @brief Handles the payload of a specific IP protocol
		 */
		class IPV4PayloadHandler {
				friend class InternetProtocolHandler;

			protected:
				InternetProtocolHandler* internetProtocolHandler;   ///< The Internet protocol handler this payload handler is connected to
				uint8_t ipProtocol;                                 ///< The IP protocol this handler handles

			public:
				IPV4PayloadHandler(InternetProtocolHandler* internetProtocolHandler, uint8_t protocol);
				~IPV4PayloadHandler();

				virtual bool handleInternetProtocolPayload(InternetProtocolAddress sourceIP, InternetProtocolAddress destinationIP, uint8_t* payloadData, uint32_t size);
				void Send(InternetProtocolAddress destinationIP, uint8_t* payloadData, uint32_t size);
		};

		/**
		 * @class InternetProtocolHandler
		 * @brief Handles IPv4 packets over Ethernet frames
		 */
		class InternetProtocolHandler : public EthernetFramePayloadHandler {

			friend class IPV4AddressResolver;

			protected:

				common::Map<uint8_t, IPV4PayloadHandler*> IPV4PayloadHandlers;  ///< Map of IP protocol numbers to their payload handlers

				IPV4AddressResolver* resolver = nullptr;                                    ///< The IP address resolver
				common::OutputStream* errorMessages;                                                    ///< Stream to output error messages to

				InternetProtocolAddress ownInternetProtocolAddress;                                     ///< The IP address of this device
				InternetProtocolAddress defaultGatewayInternetProtocolAddress;                          ///< The IP address of the default gateway
				SubnetMask subnetMask;                                                                  ///< The subnet mask

				void RegisterIPV4AddressResolver(IPV4AddressResolver* resolver);

			public:
				InternetProtocolHandler(EthernetFrameHandler* backend,
				                        InternetProtocolAddress ownInternetProtocolAddress,
				                        InternetProtocolAddress defaultGatewayInternetProtocolAddress,
				                        SubnetMask subnetMask,
				                        common::OutputStream* errorMessages);
				~InternetProtocolHandler();

				bool handleEthernetframePayload(uint8_t* ethernetframePayload, uint32_t size) override;
				void sendInternetProtocolPacket(uint32_t dstIP_BE, uint8_t protocol, const uint8_t* data, uint32_t size);

				static uint16_t Checksum(const uint16_t* data, uint32_t lengthInBytes);

				static InternetProtocolAddress CreateInternetProtocolAddress(uint8_t digit1, uint8_t digit2, uint8_t digit3, uint8_t digit4);
				static InternetProtocolAddress Parse(string address);
				static SubnetMask CreateSubnetMask(uint8_t digit1, uint8_t digit2, uint8_t digit3, uint8_t digit4);
				InternetProtocolAddress GetInternetProtocolAddress() const;
				drivers::ethernet::MediaAccessControlAddress GetMediaAccessControlAddress();

				void connectIPV4PayloadHandler(IPV4PayloadHandler* IPV4PayloadHandler);


		};
	}
}


#endif //MAXOS_NET_IPV4_H
