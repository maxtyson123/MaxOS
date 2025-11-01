/**
 * @file tcp.h
 * @brief Defines the Transmission Control Protocol (TCP) structures and classes for handling TCP sockets and payloads.
 *
 * @date 12th September 2022
 * @author Max Tyson
 */

#ifndef MAXOS_NET_TCP_H
#define MAXOS_NET_TCP_H

#include <stdint.h>
#include <net/ipv4.h>
#include <memory/memorymanagement.h>

namespace MaxOS {

	namespace net {

		typedef uint16_t TransmissionControlProtocolPort;   ///< TCP port @todo: Make TCPPort class and do tcp_port_t

		/**
		 * @enum TCPSocketState
		 * @brief The state of a TCP socket
		 */
		enum class TCPSocketState {
			CLOSED,
			LISTEN,
			SYN_SENT,
			SYN_RECEIVED,

			ESTABLISHED,

			FIN_WAIT1,
			FIN_WAIT2,
			CLOSING,
			TIME_WAIT,

			CLOSE_WAIT,
			LAST_ACK
		};

		/**
		 * @enum TCPFlag
		 * @brief The flags in a TCP header
		 */
		enum class TCPFlag {
			FIN = 1,
			SYN = 2,
			RST = 4,
			PSH = 8,
			ACK = 16,
			URG = 32,
			ECE = 64,
			CWR = 128,
			NS = 256
		};

		/**
		 * @struct TCPHeader
		 * @brief The header of a TCP packet
		 *
		 * @typedef tcp_header_t
		 * @brief Alias for TCPHeader struct
		 */
		typedef struct PACKED TCPHeader {

			uint16_t srcPort;                   ///< The port of the sender
			uint16_t dstPort;                   ///< The port of the receiver
			uint32_t sequenceNumber;            ///< Where this packet's data fits in the overall order of the data stream
			uint32_t acknowledgementNumber;     ///< The next expected sequence number from the sender (used by the receiver to request the next data)

			uint8_t reserved: 4;                ///< Unused, must be 0
			uint8_t headerSize32: 4;            ///< The size of the header in 32-bit words
			uint8_t flags;                      ///< The flags of the TCP packet (see TCPFlag enum)

			uint16_t windowSize;                ///< How many bytes the sender is willing to receive
			uint16_t checksum;                  ///< The checksum of the header
			uint16_t urgentPtr;                 ///< Where in the data the urgent data *ends* (if the URG flag is set)

			uint32_t options;                   ///< The options for the TCP packet (MSS, Window Scale, SACK Permitted) @todo: make this a struct

		} tcp_header_t;

		/**
		 * @struct TCPPseudoHeader
		 * @brief The pseudo header used for TCP checksum calculation
		 *
		 * @details The pseudo header is used to calculate the checksum of the TCP header. It is a copy of the IP
		 * header, but with the protocol field set to 6 (TCP) and the length field set to the length of the TCP header.
		 *
		 * @typedef tcp_pseudo_header_t
		 * @brief Alias for TCPPseudoHeader struct
		 */
		typedef struct PACKED TCPPseudoHeader {

			uint32_t srcIP;         ///< The IP address of the sender
			uint32_t dstIP;         ///< The IP address of the receiver
			uint16_t protocol;      ///< The protocol (set to 6 for TCP)
			uint16_t totalLength;   ///< The total length of the TCP header and data

		} tcp_pseudo_header_t;


		// Forward declarations
		class TCPSocket;
		class TransmissionControlProtocolHandler;

		/**
		 * @enum TCPPayloadHandlerEvents
		 * @brief Events for the TCPPayloadHandler
		 */
		enum class TCPPayloadHandlerEvents {
			CONNECTED,
			DISCONNECTED,
			DATA_RECEIVED
		};

		/**
		 * @class DataReceivedEvent
		 * @brief Event for when data is received on a TCP socket
		 */
		class DataReceivedEvent : public common::Event<TCPPayloadHandlerEvents> {
			public:
				TCPSocket* socket;       ///< The socket that received the data
				uint8_t* data;                                   ///< The data received
				uint16_t size;                                    ///< The size of the data received
				DataReceivedEvent(TCPSocket* socket, uint8_t* data, uint16_t size);
				~DataReceivedEvent();
		};

		/**
		 * @class ConnectedEvent
		 * @brief Event for when a TCP socket is connected
		 */
		class ConnectedEvent : public common::Event<TCPPayloadHandlerEvents> {
			public:
				TCPSocket* socket;                      ///< The socket that is connected
				ConnectedEvent(TCPSocket* socket);
				~ConnectedEvent();
		};

		/**
		 * @class DisconnectedEvent
		 * @brief Event for when a TCP socket is disconnected
		 */
		class DisconnectedEvent : public common::Event<TCPPayloadHandlerEvents> {
			public:
				TCPSocket* socket;                      ///< The socket that is disconnected
				DisconnectedEvent(TCPSocket* socket);
				~DisconnectedEvent();
		};

		/**
		 * @class TCPPayloadHandler
		 * @brief Handler for TCP payloads
		 */
		class TCPPayloadHandler : public common::EventHandler<TCPPayloadHandlerEvents> {
			public:
				TCPPayloadHandler();
				~TCPPayloadHandler();

				common::Event<TCPPayloadHandlerEvents>* on_event(common::Event<TCPPayloadHandlerEvents>* event) override;

				virtual void handleTransmissionControlProtocolPayload(TCPSocket* socket, uint8_t* data, uint16_t size);
				virtual void Connected(TCPSocket* socket);
				virtual void Disconnected(TCPSocket* socket);
		};

		/**
		 * @class TCPSocket
		 * @brief A TCP socket. Allows for sending and receiving data over TCP at a port
		 */
		class TCPSocket: public common::EventManager<TCPPayloadHandlerEvents> {
			friend class TransmissionControlProtocolHandler;
			friend class TransmissionControlProtocolPortListener;

			protected:
				uint16_t remotePort = 0;                    ///< The port on the external device
				uint32_t remoteIP = 0;                      ///< The IP address of the external device
				uint16_t localPort = 0;                     ///< The port on this device
				uint32_t localIP = 0;                       ///< The IP address of this device
				uint32_t sequenceNumber = 0;                ///< The current order number of the data being sent
				uint32_t acknowledgementNumber = 0;         ///< The number used to keep track of what has been received, incremented by 1 each time

				TransmissionControlProtocolHandler* transmissionControlProtocolHandler;  ///< The TCP handler this socket is using
				TCPSocketState state;                                                    ///< The state of the socket
			public:
				TCPSocket(TransmissionControlProtocolHandler* transmissionControlProtocolHandler);
				~TCPSocket();

				virtual void Send(uint8_t* data, uint16_t size);
				virtual void Disconnect();

				void Disconnected();
				void Connected();

				bool handleTransmissionControlProtocolPayload(uint8_t* data, uint16_t size);
		};

		/**
		 * @class TransmissionControlProtocolHandler
		 * @brief Handles TCP packets and manages TCP sockets
		 */
		class TransmissionControlProtocolHandler : IPV4PayloadHandler {
			friend class TCPSocket;

			protected:
				common::OutputStream* errorMessages;                                ///< Where to write error messages
				common::Vector<TCPSocket*> sockets;         ///< The list of connected sockets

				static TransmissionControlProtocolPort freePorts;                   ///< The next free port to use for new sockets
				void sendTransmissionControlProtocolPacket(TCPSocket* socket, const uint8_t* data, uint16_t size, uint16_t flags = 0);

			public:
				TransmissionControlProtocolHandler(InternetProtocolHandler* internetProtocolHandler, common::OutputStream* errorMessages);
				~TransmissionControlProtocolHandler();

				bool handleInternetProtocolPayload(InternetProtocolAddress sourceIP, InternetProtocolAddress destinationIP, uint8_t* payloadData, uint32_t size) override;

				TCPSocket* Connect(InternetProtocolAddress ip, TransmissionControlProtocolPort port);
				static TCPSocket* Connect(const string &address);

				void Disconnect(TCPSocket* socket);

				virtual TCPSocket* Listen(uint16_t port);
				virtual void Bind(TCPSocket* socket, TCPPayloadHandler* handler);
		};


	}

}

#endif //MAXOS_NET_TCP_H
