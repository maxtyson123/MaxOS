/**
 * @file ethernetframe.h
 * @brief Defines classes for handling Ethernet Frames and their payloads
 *
 * @date 11th May 2022
 * @author Max Tyson
 *
 * @todo Make big endian helper functions and clairty (ie be_uint16_t etc)z
 */

#ifndef MAXOS_NET_ETHERNETFRAME_H
#define MAXOS_NET_ETHERNETFRAME_H

#include <stdint.h>
#include <common/map.h>
#include <drivers/ethernet/ethernet.h>
#include <memory/memorymanagement.h>

namespace MaxOS {

	namespace net {

		/**
		 * @struct EthernetFrameHeader
		 * @brief Structure the raw data. This is the header of an Ethernet Frame
		 *
		 * @typedef ethernet_frame_header_t
		 * @brief Alias for EthernetFrameHeader struct
		 */
		typedef struct PACKED EthernetFrameHeader {

			uint64_t destinationMAC: 48;     ///< The mac address of the target (Big Endian)
			uint64_t sourceMAC: 48;          ///< The mac address of the sender (Big Endian)

			uint16_t type;                  ///< The type (Big Endian) 0x0800 = IPv4, 0x0806 = ARP, 0x86DD = IPv6

		} ethernet_frame_header_t;

		/**
		 * @struct EthernetFrameFooter
		 * @brief Structure the raw data. This is the footer of an Ethernet Frame
		 *
		 * @typedef ethernet_frame_footer_t
		 * @brief Alias for EthernetFrameFooter struct
		 */
		typedef struct PACKED EthernetFrameFooter {

			uint32_t checksum;          	///< Checksum of the payload

		} ethernet_frame_footer_t;

		class EthernetFrameHandler;

		/**
		 * @class EthernetFramePayloadHandler
		 * @brief Handles a specific type of Ethernet Frame payload
		 */
		class EthernetFramePayloadHandler {
			friend class EthernetFrameHandler;

			protected:
				EthernetFrameHandler* frameHandler; ///< The Ethernet frame handler this payload handler is connected to
				uint16_t handledType;               ///< The Ethernet frame type this handler handles

			public:
				EthernetFramePayloadHandler(EthernetFrameHandler* frameHandler, uint16_t handledType);
				~EthernetFramePayloadHandler();

				virtual bool handleEthernetframePayload(uint8_t* ethernetframePayload, uint32_t size);
				void Send(uint64_t destinationMAC, uint8_t* data, uint32_t size);
		};

		/**
		 * @class EthernetFrameHandler
		 * @brief Handles incoming Ethernet Frames and routes them to the appropriate payload handlers
		 */
		class EthernetFrameHandler : public drivers::ethernet::EthernetDriverEventHandler {
			protected:

				common::Map<uint16_t, EthernetFramePayloadHandler*> frameHandlers;  ///< The map of frame handlers by type

				drivers::ethernet::EthernetDriver* ethernetDriver;                  ///< The driver this frame handler is using
				common::OutputStream* errorMessages;                                ///< The output stream for error messages

			public:
				EthernetFrameHandler(drivers::ethernet::EthernetDriver* driver, common::OutputStream* errorMessages);
				~EthernetFrameHandler();

				drivers::ethernet::MediaAccessControlAddress getMAC();
				bool DataReceived(uint8_t* data, uint32_t size) override;
				void connectHandler(EthernetFramePayloadHandler* handler);
				void sendEthernetFrame(uint64_t destinationMAC, uint16_t frameType, uint8_t* data, uint32_t size);

		};
	}
}

#endif //MAXOS_NET_ETHERNETFRAME_H
