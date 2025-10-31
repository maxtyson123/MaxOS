//
// Created by 98max on 12/9/2022.
//

#ifndef MAXOS_NET_TCP_H
#define MAXOS_NET_TCP_H

#include <stdint.h>
#include <net/ipv4.h>
#include <memory/memorymanagement.h>

namespace MaxOS {

	namespace net {

		typedef uint16_t TransmissionControlProtocolPort;

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
		 * @struct TransmissionControlProtocolHeader
		 * @brief The header of a TCP packet
		 */
		struct TransmissionControlProtocolHeader {
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

			uint32_t options;                   ///< The options for the TCP packet (MSS, Window Scale, SACK Permitted, todo: make this a struct)
		} __attribute__((packed));

		/**
		 * @struct TransmissionControlProtocolPseudoHeader
		 * @brief The pseudo header used for TCP checksum calculation
		 *
		 * @details The pseudo header is used to calculate the checksum of the TCP header. It is a copy of the IP
		 * header, but with the protocol field set to 6 (TCP) and the length field set to the length of the TCP header.
		 */
		struct TransmissionControlProtocolPseudoHeader {
			uint32_t srcIP;         ///< The IP address of the sender
			uint32_t dstIP;         ///< The IP address of the receiver
			uint16_t protocol;      ///< The protocol (set to 6 for TCP)
			uint16_t totalLength;   ///< The total length of the TCP header and data
		} __attribute__((packed));


		// Forward declarations
		class TransmissionControlProtocolSocket;
		class TransmissionControlProtocolHandler;

		/**
		 * @enum TransmissionControlProtocolPayloadHandlerEvents
		 * @brief Events for the TransmissionControlProtocolPayloadHandler
		 */
		enum class TransmissionControlProtocolPayloadHandlerEvents {
			CONNECTED,
			DISCONNECTED,
			DATA_RECEIVED
		};

		/**
		 * @class DataReceivedEvent
		 * @brief Event for when data is received on a TCP socket
		 */
		class DataReceivedEvent : public common::Event<TransmissionControlProtocolPayloadHandlerEvents> {
			public:
				TransmissionControlProtocolSocket* socket;       ///< The socket that received the data
				uint8_t* data;                                   ///< The data received
				uint16_t size;                                    ///< The size of the data received
				DataReceivedEvent(TransmissionControlProtocolSocket* socket, uint8_t* data, uint16_t size);
				~DataReceivedEvent();
		};

		/**
		 * @class ConnectedEvent
		 * @brief Event for when a TCP socket is connected
		 */
		class ConnectedEvent : public common::Event<TransmissionControlProtocolPayloadHandlerEvents> {
			public:
				TransmissionControlProtocolSocket* socket;                      ///< The socket that is connected
				ConnectedEvent(TransmissionControlProtocolSocket* socket);
				~ConnectedEvent();
		};

		/**
		 * @class DisconnectedEvent
		 * @brief Event for when a TCP socket is disconnected
		 */
		class DisconnectedEvent : public common::Event<TransmissionControlProtocolPayloadHandlerEvents> {
			public:
				TransmissionControlProtocolSocket* socket;                      ///< The socket that is disconnected
				DisconnectedEvent(TransmissionControlProtocolSocket* socket);
				~DisconnectedEvent();
		};

		/**
		 * @class TransmissionControlProtocolPayloadHandler
		 * @brief Handler for TCP payloads
		 */
		class TransmissionControlProtocolPayloadHandler : public common::EventHandler<TransmissionControlProtocolPayloadHandlerEvents> {
			public:
				TransmissionControlProtocolPayloadHandler();
				~TransmissionControlProtocolPayloadHandler();

				common::Event<TransmissionControlProtocolPayloadHandlerEvents>* on_event(common::Event<TransmissionControlProtocolPayloadHandlerEvents>* event) override;

				virtual void handleTransmissionControlProtocolPayload(TransmissionControlProtocolSocket* socket, uint8_t* data, uint16_t size);
				virtual void Connected(TransmissionControlProtocolSocket* socket);
				virtual void Disconnected(TransmissionControlProtocolSocket* socket);
		};

		/**
		 * @class TransmissionControlProtocolSocket
		 * @brief A TCP socket. Allows for sending and receiving data over TCP at a port
		 */
		class TransmissionControlProtocolSocket: public common::EventManager<TransmissionControlProtocolPayloadHandlerEvents> {
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
				TransmissionControlProtocolSocket(TransmissionControlProtocolHandler* transmissionControlProtocolHandler);
				~TransmissionControlProtocolSocket();

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
		class TransmissionControlProtocolHandler : InternetProtocolPayloadHandler {
			friend class TransmissionControlProtocolSocket;

			protected:
				common::OutputStream* errorMessages;                                ///< Where to write error messages
				common::Vector<TransmissionControlProtocolSocket*> sockets;         ///< The list of connected sockets

				static TransmissionControlProtocolPort freePorts;                   ///< The next free port to use for new sockets
				void sendTransmissionControlProtocolPacket(TransmissionControlProtocolSocket* socket, const uint8_t* data, uint16_t size, uint16_t flags = 0);

			public:
				TransmissionControlProtocolHandler(InternetProtocolHandler* internetProtocolHandler, common::OutputStream* errorMessages);
				~TransmissionControlProtocolHandler();

				bool handleInternetProtocolPayload(InternetProtocolAddress sourceIP, InternetProtocolAddress destinationIP, uint8_t* payloadData, uint32_t size) override;

				TransmissionControlProtocolSocket* Connect(InternetProtocolAddress ip, TransmissionControlProtocolPort port);
				static TransmissionControlProtocolSocket* Connect(const string &address);

				void Disconnect(TransmissionControlProtocolSocket* socket);

				virtual TransmissionControlProtocolSocket* Listen(uint16_t port);
				virtual void Bind(TransmissionControlProtocolSocket* socket, TransmissionControlProtocolPayloadHandler* handler);
		};


	}

}

#endif //MAXOS_NET_TCP_H
