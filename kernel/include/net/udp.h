//
// Created by 98max on 24/11/2022.
//

#ifndef MAXOS_NET_UDP_H
#define MAXOS_NET_UDP_H


#include <stdint.h>
#include <common/eventHandler.h>
#include <net/ipv4.h>
#include <memory/memorymanagement.h>

namespace MaxOS {
	namespace net {

		/**
		 * @struct UserDatagramProtocolHeader
		 * @brief The header of a UDP packet
		 */
		struct UserDatagramProtocolHeader {

			uint16_t sourcePort;            ///< The port of the sender
			uint16_t destinationPort;       ///< The port of the receiver
			uint16_t length;                ///< The length of the UDP header and data
			uint16_t checksum;              ///< The checksum of the header and data

		} __attribute__((packed));

		/**
		 * @enum UserDatagramProtocolEvents
		 * @brief The events that can be fired by the UDP protocol
		 */
		enum class UserDatagramProtocolEvents {
			DATA_RECEIVED,
		};

		//Predefine
		class UserDatagramProtocolSocket;
		class UserDatagramProtocolHandler;

		typedef uint16_t UserDatagramProtocolPort;

		/**
		 * @class UDPDataReceivedEvent
		 * @brief Event fired when data is received on a UDP socket
		 */
		class UDPDataReceivedEvent : public common::Event<UserDatagramProtocolEvents> {
			public:
				UserDatagramProtocolSocket* socket;     ///< The socket that received the data
				uint8_t* data;                          ///< The data received
				uint16_t size;                          ///< The size of the data received

				UDPDataReceivedEvent(UserDatagramProtocolSocket* socket, uint8_t* data, uint16_t size);
				~UDPDataReceivedEvent();
		};

		/**
		 * @class UserDatagramProtocolPayloadHandler
		 * @brief Handles the payload of a UDP packet
		 */
		class UserDatagramProtocolPayloadHandler : public common::EventHandler<UserDatagramProtocolEvents> {
			public:
				UserDatagramProtocolPayloadHandler();
				~UserDatagramProtocolPayloadHandler();

				common::Event<UserDatagramProtocolEvents>* on_event(common::Event<UserDatagramProtocolEvents>* event) override;

				virtual void handleUserDatagramProtocolMessage(UserDatagramProtocolSocket* socket, uint8_t* data, uint16_t size);

		};

		/**
		 * @class UserDatagramProtocolSocket
		 * @brief A UDP socket
		 */
		class UserDatagramProtocolSocket : public common::EventManager<UserDatagramProtocolEvents> {
				friend class UserDatagramProtocolHandler;

			protected:
				bool listening;             	///< Wether the port is waiting for incoming connections

				uint16_t localPort = 0;         ///< The port on this device
				uint16_t remotePort = 0;        ///< The port on the remote device

				uint32_t localIP = 0;           ///< The IP of this device
				uint32_t remoteIP = 0;          ///< The IP of the remote device

				UserDatagramProtocolHandler* userDatagramProtocolHandler;   ///< The UDP handler this socket is connected to

			public:
				UserDatagramProtocolSocket();
				~UserDatagramProtocolSocket();

				virtual void handleUserDatagramProtocolPayload(uint8_t* data, uint16_t size);
				virtual void Send(uint8_t* data, uint16_t size);
				virtual void Disconnect();

		};

		/**
		 * @class UserDatagramProtocolHandler
		 * @brief Handles the UDP protocol
		 */
		class UserDatagramProtocolHandler : InternetProtocolPayloadHandler {
			protected:
				common::Vector<UserDatagramProtocolSocket*> sockets;    ///< The list of UDP sockets
				static UserDatagramProtocolPort freePorts;              ///< The next free port number
				common::OutputStream* errorMessages;                    ///< Where to write error messages

			public:
				UserDatagramProtocolHandler(InternetProtocolHandler* internetProtocolHandler, common::OutputStream* errorMessages);
				~UserDatagramProtocolHandler();

				bool handleInternetProtocolPayload(InternetProtocolAddress sourceIP, InternetProtocolAddress destinationIP, uint8_t* payloadData, uint32_t size) override;

				UserDatagramProtocolSocket* Connect(uint32_t ip, uint16_t port);
				static UserDatagramProtocolSocket* Connect(const string &address);

				UserDatagramProtocolSocket* Listen(uint16_t port);

				void Disconnect(UserDatagramProtocolSocket* socket);
				void Send(UserDatagramProtocolSocket* socket, const uint8_t* data, uint16_t size);

				static void Bind(UserDatagramProtocolSocket* socket, UserDatagramProtocolPayloadHandler* userDatagramProtocolPayloadHandler);
		};

	}
}

#endif //MAXOS_NET_UDP_H
