/**
 * @file udp.h
 * @brief Defines the User Datagram Protocol (UDP) for network communication.
 *
 * @date 24th November 2022
 * @author Max Tyson
 */

#ifndef MAXOS_NET_UDP_H
#define MAXOS_NET_UDP_H


#include <stdint.h>
#include <common/eventHandler.h>
#include <net/ipv4.h>
#include <memory/memorymanagement.h>

namespace MaxOS {
	namespace net {

		/**
		 * @struct UDPHeader
		 * @brief The header of a UDP packet
		 */
		struct UDPHeader {

			uint16_t sourcePort;            ///< The port of the sender
			uint16_t destinationPort;       ///< The port of the receiver
			uint16_t length;                ///< The length of the UDP header and data
			uint16_t checksum;              ///< The checksum of the header and data

		} __attribute__((packed));

		/**
		 * @enum UDPEvents
		 * @brief The events that can be fired by the UDP protocol
		 */
		enum class UDPEvents {
			DATA_RECEIVED,
		};

		//Predefine
		class UDPSocket;
		class UserDatagramProtocolHandler;

		typedef uint16_t UserDatagramProtocolPort;

		/**
		 * @class UDPDataReceivedEvent
		 * @brief Event fired when data is received on a UDP socket
		 */
		class UDPDataReceivedEvent : public common::Event<UDPEvents> {
			public:
				UDPSocket* socket;     ///< The socket that received the data
				uint8_t* data;                          ///< The data received
				uint16_t size;                          ///< The size of the data received

				UDPDataReceivedEvent(UDPSocket* socket, uint8_t* data, uint16_t size);
				~UDPDataReceivedEvent();
		};

		/**
		 * @class UDPPayloadHandler
		 * @brief Handles the payload of a UDP packet
		 */
		class UDPPayloadHandler : public common::EventHandler<UDPEvents> {
			public:
				UDPPayloadHandler();
				~UDPPayloadHandler();

				common::Event<UDPEvents>* on_event(common::Event<UDPEvents>* event) override;

				virtual void handleUserDatagramProtocolMessage(UDPSocket* socket, uint8_t* data, uint16_t size);

		};

		/**
		 * @class UDPSocket
		 * @brief A UDP socket
		 */
		class UDPSocket : public common::EventManager<UDPEvents> {
				friend class UserDatagramProtocolHandler;

			protected:
				bool listening;             	///< Wether the port is waiting for incoming connections

				uint16_t localPort = 0;         ///< The port on this device
				uint16_t remotePort = 0;        ///< The port on the remote device

				uint32_t localIP = 0;           ///< The IP of this device
				uint32_t remoteIP = 0;          ///< The IP of the remote device

				UserDatagramProtocolHandler* userDatagramProtocolHandler;   ///< The UDP handler this socket is connected to

			public:
				UDPSocket();
				~UDPSocket();

				virtual void handleUserDatagramProtocolPayload(uint8_t* data, uint16_t size);
				virtual void Send(uint8_t* data, uint16_t size);
				virtual void Disconnect();

		};

		/**
		 * @class UserDatagramProtocolHandler
		 * @brief Handles the UDP protocol
		 */
		class UserDatagramProtocolHandler : IPV4PayloadHandler {
			protected:
				common::Vector<UDPSocket*> sockets;    ///< The list of UDP sockets
				static UserDatagramProtocolPort freePorts;              ///< The next free port number
				common::OutputStream* errorMessages;                    ///< Where to write error messages

			public:
				UserDatagramProtocolHandler(InternetProtocolHandler* internetProtocolHandler, common::OutputStream* errorMessages);
				~UserDatagramProtocolHandler();

				bool handleInternetProtocolPayload(InternetProtocolAddress sourceIP, InternetProtocolAddress destinationIP, uint8_t* payloadData, uint32_t size) override;

				UDPSocket* Connect(uint32_t ip, uint16_t port);
				static UDPSocket* Connect(const string &address);

				UDPSocket* Listen(uint16_t port);

				void Disconnect(UDPSocket* socket);
				void Send(UDPSocket* socket, const uint8_t* data, uint16_t size);

				static void Bind(UDPSocket* socket, UDPPayloadHandler* UDPPayloadHandler);
		};

	}
}

#endif //MAXOS_NET_UDP_H
