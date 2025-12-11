/**
 * @file udp.h
 * @brief Defines the User Datagram Protocol (UDP) for network communication.
 *
 * @date 24th November 2022
 * @author Max Tyson
 */

#ifndef MAXOS_NET_UDP_H
#define MAXOS_NET_UDP_H


#include <cstdint>
#include <eventHandler.h>
#include <net/ipv4.h>
#include <memory/memorymanagement.h>
#include <mem.h>


namespace MaxOS::net {

	/**
	 * @struct UDPHeader
	 * @brief The header of a UDP packet
	 *
	 * @typedef udp_header_t
	 * @brief Alias for UDPHeader struct
	 */
	typedef struct PACKED UDPHeader {

		uint16_t source_port;            ///< The port of the sender
		uint16_t destination_port;       ///< The port of the receiver
		uint16_t length;                ///< The length of the UDP header and data
		uint16_t checksum;              ///< The checksum of the header and data

	} udp_header_t;

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

	typedef uint16_t UserDatagramProtocolPort;      ///< UDP port @todo: Make UDPPort class and do udp_port_t (or generic port that can be used for TCP and UDP)

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

			virtual void handle_user_datagram_protocol_message(UDPSocket* socket, uint8_t* data, uint16_t size);

	};

	/**
	 * @class UDPSocket
	 * @brief A UDP socket
	 */
	class UDPSocket : public common::EventManager<UDPEvents> {
			friend class UserDatagramProtocolHandler;

		protected:
			bool listening;                ///< Wether the port is waiting for incoming connections

			uint16_t local_port = 0;         ///< The port on this device
			uint16_t remote_port = 0;        ///< The port on the remote device

			uint32_t local_ip = 0;           ///< The IP of this device
			uint32_t remote_ip = 0;          ///< The IP of the remote device

			UserDatagramProtocolHandler* user_datagram_protocol_handler;   ///< The UDP handler this socket is connected to

		public:
			UDPSocket();
			~UDPSocket();

			virtual void handle_user_datagram_protocol_payload(uint8_t* data, uint16_t size);
			virtual void send(uint8_t* data, uint16_t size);
			virtual void disconnect();

	};

	/**
	 * @class UserDatagramProtocolHandler
	 * @brief Handles the UDP protocol
	 */
	class UserDatagramProtocolHandler : IPV4PayloadHandler {
		protected:
			common::Vector<UDPSocket*> sockets;    ///< The list of UDP sockets
			static UserDatagramProtocolPort free_ports;              ///< The next free port number
			common::OutputStream* errorMessages;                    ///< Where to write error messages

		public:
			UserDatagramProtocolHandler(InternetProtocolHandler* internet_protocol_handler, common::OutputStream* error_messages);
			~UserDatagramProtocolHandler();

			bool handle_internet_protocol_payload(net::InternetProtocolAddress source_ip, net::InternetProtocolAddress destination_ip, uint8_t* payload_data, uint32_t size) override;

			UDPSocket* connect(uint32_t ip, uint16_t port);
			static UDPSocket* connect(const string& address);

			UDPSocket* listen(uint16_t port);

			void disconnect(UDPSocket* socket);
			void send(UDPSocket* socket, const uint8_t* data, uint16_t size);

			static void bind(UDPSocket* socket, UDPPayloadHandler* udp_payload_handler);
	};

}


#endif //MAXOS_NET_UDP_H
