/**
 * @file ipv4.h
 * @brief Defines classes and structures for handling Internet Protocol version 4 (IPv4) packets
 *
 * @date 22nd November 2022
 * @author Max Tyson
 */

#ifndef MAXOS_NET_IPV4_H
#define MAXOS_NET_IPV4_H

#include <cstdint>
#include <net/ethernetframe.h>


namespace MaxOS::net {

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

		uint8_t header_length : 4;        ///< The length of the header in 32-bit words (min 5, max 15)
		uint8_t version : 4;             ///< The version of the IP protocol (4 for IPv4)
		uint8_t type_of_service;          ///< The type of service (low delay, high throughput, high reliability, etc. @todo: enum
		uint16_t total_length;           ///< The total length of the IP packet (header + data) in bytes

		uint16_t identifier;            ///< Unique identifier for the group of fragments of a single IP packet
		uint16_t flags_and_offset;        ///< Flags (3 bits) and Fragment Offset (13 bits) @todo: bitfield struct?

		uint8_t time_to_live;             ///< The maximum hops between devices before the packet is discarded
		uint8_t protocol;               ///< The protocol of the payload (TCP, UDP, ICMP, etc. @todo: enum
		uint16_t checksum;              ///< The checksum of the header

		uint32_t source_ip;              ///< The IP of the sender
		uint32_t destination_ip;         ///< The IP of the receiver

	} ipv4_header_t;

	class InternetProtocolHandler;

	/**
	 * @class IPV4AddressResolver
	 * @brief Resolves IP addresses to MAC addresses
	 */
	class IPV4AddressResolver {
		public:
			explicit IPV4AddressResolver(InternetProtocolHandler* internet_protocol_handler);
			~IPV4AddressResolver();
			virtual drivers::ethernet::MediaAccessControlAddress resolve(InternetProtocolAddress address);
			virtual void store(InternetProtocolAddress internet_protocol_address, drivers::ethernet::MediaAccessControlAddress media_access_control_address);
	};


	/**
	 * @class IPV4PayloadHandler
	 * @brief Handles the payload of a specific IP protocol
	 */
	class IPV4PayloadHandler {
			friend class InternetProtocolHandler;

		protected:
			InternetProtocolHandler* internet_protocol_handler;   ///< The Internet protocol handler this payload handler is connected to
			uint8_t ip_protocol;                                 ///< The IP protocol this handler handles

		public:
			IPV4PayloadHandler(InternetProtocolHandler* internet_protocol_handler, uint8_t protocol);
			~IPV4PayloadHandler();

			virtual bool handle_internet_protocol_payload(net::InternetProtocolAddress src_ip_be, net::InternetProtocolAddress dst_ip_be, uint8_t* internetprotocol_payload, uint32_t size);
			void send(InternetProtocolAddress destination_ip, uint8_t* payload_data, uint32_t size);
	};

	/**
	 * @class InternetProtocolHandler
	 * @brief Handles IPv4 packets over Ethernet frames
	 */
	class InternetProtocolHandler : public EthernetFramePayloadHandler {

			friend class IPV4AddressResolver;

		protected:

			common::Map<uint8_t, IPV4PayloadHandler*> ipv_4_payload_handlers;  ///< Map of IP protocol numbers to their payload handlers

			IPV4AddressResolver* resolver = nullptr;                                    ///< The IP address resolver
			common::OutputStream* error_messages;                                                    ///< Stream to output error messages to

			InternetProtocolAddress own_internet_protocol_address;                                     ///< The IP address of this device
			InternetProtocolAddress default_gateway_internet_protocol_address;                          ///< The IP address of the default gateway
			SubnetMask subnet_mask;                                                                  ///< The subnet mask

			void register_ipv_4_address_resolver(IPV4AddressResolver* ipv4_resolver);

		public:
			InternetProtocolHandler(EthernetFrameHandler* backend,
			                        InternetProtocolAddress own_internet_protocol_address,
			                        InternetProtocolAddress default_gateway_internet_protocol_address,
			                        SubnetMask subnet_mask,
			                        common::OutputStream* error_messages);
			~InternetProtocolHandler();

			bool handle_ethernetframe_payload(uint8_t* ethernetframe_payload, uint32_t size) override;
			void send_internet_protocol_packet(uint32_t dst_ip_be, uint8_t protocol, const uint8_t* data, uint32_t size);

			static uint16_t checksum(const uint16_t* data, uint32_t length_in_bytes);

			static InternetProtocolAddress create_internet_protocol_address(uint8_t digit1, uint8_t digit2, uint8_t digit3, uint8_t digit4);
			static InternetProtocolAddress parse(string address);
			static SubnetMask create_subnet_mask(uint8_t digit1, uint8_t digit2, uint8_t digit3, uint8_t digit4);
			[[nodiscard]] InternetProtocolAddress get_internet_protocol_address() const;
			drivers::ethernet::MediaAccessControlAddress get_media_access_control_address();

			void connect_ipv_4_payload_handler(IPV4PayloadHandler* ipv_4_payload_handler);


	};
}


#endif //MAXOS_NET_IPV4_H
