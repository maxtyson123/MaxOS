/**
 * @file tcp.cpp
 * @brief Implementation of Transmission Control Protocol (TCP) classes and handlers
 *
 * @date 26th March 2024
 * @author Max Tyson
 */

#include <net/tcp.h>
#include "net/udp.h"


using namespace MaxOS;
using namespace MaxOS::net;
using namespace MaxOS::common;
using namespace MaxOS::memory;

///__Handler__///

TCPPayloadHandler::TCPPayloadHandler() = default;

TCPPayloadHandler::~TCPPayloadHandler() = default;

/**
 * @brief Handle TCP data received on the socket
 *
 * @param socket The socket the data was received on
 * @param data The data received
 * @param size The size of the data in bytes
 */
void TCPPayloadHandler::handle_transmission_control_protocol_payload(TCPSocket* socket, uint8_t* data, uint16_t size) {

}

/**
 * @brief Handle a new TCP connection on the socket
 *
 * @param socket The socket that was connected
 */
void TCPPayloadHandler::connected(TCPSocket* socket) {

}

/**
 * @brief Handle a TCP disconnection on the socket
 *
 * @param socket The socket that was disconnected
 */
void TCPPayloadHandler::disconnected(TCPSocket* socket) {

}

/**
 * @brief Handle an event occurring on the TCP payload handler
 *
 * @param event The event to handle
 * @return The handled event
 */
Event<TCPPayloadHandlerEvents>* TCPPayloadHandler::on_event(Event<TCPPayloadHandlerEvents>* event) {

	switch(event->type) {
		case TCPPayloadHandlerEvents::CONNECTED:
			connected(((ConnectedEvent*) event)->socket);
			break;
		case TCPPayloadHandlerEvents::DISCONNECTED:
			disconnected(((DisconnectedEvent*) event)->socket);
			break;
		case TCPPayloadHandlerEvents::DATA_RECEIVED:
			handle_transmission_control_protocol_payload(((DataReceivedEvent*) event)->socket,
			                                             ((DataReceivedEvent*) event)->data,
			                                             ((DataReceivedEvent*) event)->size);
			break;
	}

	return event;
}

/**
 * @brief Construct a new TCP Socket object
 *
 * @param transmission_control_protocol_handler the TCP handler the socket is associated with
 */
TCPSocket::TCPSocket(TransmissionControlProtocolHandler* transmission_control_protocol_handler) {
	//Set the default values
	this->transmission_control_protocol_handler = transmission_control_protocol_handler;

	//Closed as default
	state = TCPSocketState::CLOSED;
}

TCPSocket::~TCPSocket() = default;

/**
 * @brief Handle the TCP message (socket end)
 *
 * @param data The datah
 * @param size The size of the data
 * @return True if the connection is to be terminated after hadnling or false if not
 */
bool TCPSocket::handle_transmission_control_protocol_payload(uint8_t* data, uint16_t size) {
	auto* event = new DataReceivedEvent(this, data, size);
	raise_event(event);
	MemoryManager::kfree(event);
	return true;
}

/**
 * @brief send data over the socket
 *
 * @param data The data to send
 * @param size The size of the data
 */
void TCPSocket::send(uint8_t* data, uint16_t size) {
	//Wait for the socket to be connected
	while(state != TCPSocketState::ESTABLISHED);

	//Pass the data to the backend
	transmission_control_protocol_handler->send_transmission_control_protocol_packet(this, data, size,
	                                                                                 (uint16_t) TCPFlag::PSH |
	                                                                                 (uint16_t) TCPFlag::ACK);
}

/**
 * @brief disconnect the socket
 */
void TCPSocket::disconnect() {
	transmission_control_protocol_handler->disconnect(this);
}

/**
 * @brief Raise the disconnected event
 */
void TCPSocket::disconnected() {
	auto* event = new DisconnectedEvent(this);
	raise_event(event);
	MemoryManager::kfree(event);

}

/**
 * @brief Raise the connected event
 */
void TCPSocket::connected() {
	auto* event = new ConnectedEvent(this);
	raise_event(event);
	MemoryManager::kfree(event);

}

///__Handler__///

TransmissionControlProtocolPort TransmissionControlProtocolHandler::free_ports = 0x8000;

/**
 * @brief Construct a new Transmission Control Protocol Handler object
 *
 * @param internet_protocol_handler The Internet protocol handler
 * @param error_messages Where to write error messages
 */
TransmissionControlProtocolHandler::TransmissionControlProtocolHandler(MaxOS::net::InternetProtocolHandler* internet_protocol_handler, OutputStream* error_messages)
		: IPV4PayloadHandler(internet_protocol_handler, 0x06) {
	this->error_messages = error_messages;

}

TransmissionControlProtocolHandler::~TransmissionControlProtocolHandler() = default;

/**
 * @brief Convert a 32-bit integer to big-endian format
 *
 * @param x The 32-bit integer to convert
 * @return The big-endian formatted integer
 */
uint32_t big_endian_32(uint32_t x) {
	return ((x & 0xFF000000) >> 24)
	       | ((x & 0x00FF0000) >> 8)
	       | ((x & 0x0000FF00) << 8)
	       | ((x & 0x000000FF) << 24);
}

/**
 * @brief Convert a 16-bit integer to big-endian format
 *
 * @param x The 16-bit integer to convert
 * @return The big-endian formatted integer
 */
uint32_t big_endian_16(uint16_t x) {
	return ((x & 0xFF00) >> 8)
	       | ((x & 0x00FF) << 8);
}

/**
 * @brief Handle the TCP message (provider end)
 *
 * @param source_ip The source IP address
 * @param destination_ip The destination IP address
 * @param payload_data The payload
 * @param size The size of the payload
 * @return True if data is to be sent back or false if not
 */
bool TransmissionControlProtocolHandler::handle_internet_protocol_payload(net::InternetProtocolAddress source_ip, net::InternetProtocolAddress destination_ip, uint8_t* payload_data, uint32_t size) {

	error_messages->write("TCP: Handling TCP message\n");

	//Check if the size is too small
	if(size < 13) {
		return false;
	}

	// If it's smaller than the header, return
	if(size < 4 * payload_data[12] / 16)             // The lower 4 bits of the 13th byte is the header length
	{
		return false;
	}

	//Get the header
	auto* msg = (TCPHeader*) payload_data;

	//Get the connection values (convert to host endian)
	uint16_t local_port = big_endian_16(msg->dst_port);
	uint16_t remote_port = big_endian_16(msg->src_port);

	//Create the socket
	TCPSocket* socket = nullptr;

	for(auto& current_socket : sockets) {
		if(current_socket->local_port ==
		   local_port                               //Check if the local port is the same as the destination port
		   && current_socket->local_ip ==
		      destination_ip                                  //Check if the local IP is the same as the destination IP
		   && current_socket->state ==
		      TCPSocketState::LISTEN                                           //Check if the socket is in the LISTEN state
		   && (((msg->flags) & ((uint16_t) TCPFlag::SYN | (uint16_t) TCPFlag::ACK)) ==
		       (uint16_t) TCPFlag::SYN))                                       //Check if the SYN flag is set (allow for acknoweldgement)
		{
			socket = current_socket;
		} else if(current_socket->local_port ==
		          local_port                          //Check if the local port is the same as the destination port
		          && current_socket->local_ip ==
		             destination_ip                             //Check if the local IP is the same as the destination IP
		          && current_socket->remotePort ==
		             remote_port                         //Check if the remote port is the same as the source port
		          && current_socket->remote_ip ==
		             destination_ip)                           //Check if the remote IP is the same as the source IP
		{
			socket = current_socket;
		}
	}


	bool reset = false;

	//Check if the socket is found and if the socket wants to reset
	if(socket != nullptr && msg->flags & (uint16_t) TCPFlag::RST) {
		socket->state = TCPSocketState::CLOSED;
		socket->disconnected();
	}

	//Check if the socket is found and if the socket is not closed
	if(socket != nullptr && socket->state != TCPSocketState::CLOSED) {
		switch((msg->flags) & ((uint16_t) TCPFlag::SYN | (uint16_t) TCPFlag::ACK | (uint16_t) TCPFlag::FIN)) {
			/*
			 * Example for explanation:
			 * socket -> state = SYN_RECEIVED;                                                  //The state of the socket, e.g. recieved, or established. This is used to know how to handle the socket
			 * socket -> remotePort = msg -> srcPort;                                           //The remote port, e.g. the port of the server
			 * socket -> remoteIP = srcIP_BE;                                                   //The remote IP, e.g. the IP of the server
			 * socket -> acknowledgementNumber = bigEndian32( msg -> sequenceNumber ) + 1;      //The acknowledgement number, the number used to keep track of what has been received, this is just incremented by 1 each time
			 * socket -> sequenceNumber = 0xbeefcafe;                                           //The sequence number, the number of the next set that is to be sent but in this case sequence isn't enabled so just set it to anything
			 * Send(socket, 0,0, SYN|ACK);                                                      //The response command, genneraly has to have the acknoledgement flag set
			 * socket -> sequenceNumber++;                                                      //Increment the sequence number
			 *
			 */

			case (uint16_t) TCPFlag::SYN:
				if(socket->state == TCPSocketState::LISTEN) {
					socket->state = TCPSocketState::SYN_RECEIVED;
					socket->remotePort = msg->src_port;
					socket->remote_ip = source_ip;
					socket->acknowledgement_number = big_endian_32(msg->sequence_number) + 1;
					socket->sequence_number = 0xbeefcafe;
					send_transmission_control_protocol_packet(socket, nullptr, 0,
					                                          (uint16_t) TCPFlag::SYN | (uint16_t) TCPFlag::ACK);
					socket->sequence_number++;
				} else
					reset = true;
				break;


			case (uint16_t) TCPFlag::SYN | (uint16_t) TCPFlag::ACK:
				if(socket->state == TCPSocketState::SYN_SENT) {
					socket->state = TCPSocketState::ESTABLISHED;
					socket->acknowledgement_number = big_endian_32(msg->sequence_number) + 1;
					socket->sequence_number++;
					send_transmission_control_protocol_packet(socket, nullptr, 0, (uint16_t) TCPFlag::ACK);
				} else
					reset = true;
				break;


			case (uint16_t) TCPFlag::SYN | (uint16_t) TCPFlag::FIN:
			case (uint16_t) TCPFlag::SYN | (uint16_t) TCPFlag::FIN | (uint16_t) TCPFlag::ACK:
				reset = true;
				break;


			case (uint16_t) TCPFlag::FIN:
			case (uint16_t) TCPFlag::FIN | (uint16_t) TCPFlag::ACK:
				if(socket->state == TCPSocketState::ESTABLISHED) {
					socket->state = TCPSocketState::CLOSE_WAIT;
					socket->acknowledgement_number++;
					send_transmission_control_protocol_packet(socket, nullptr, 0, (uint16_t) TCPFlag::ACK);
					send_transmission_control_protocol_packet(socket, nullptr, 0,
					                                          (uint16_t) TCPFlag::FIN | (uint16_t) TCPFlag::ACK);
					socket->disconnected();
				} else if(socket->state == TCPSocketState::CLOSE_WAIT) {
					socket->state = TCPSocketState::CLOSED;
				} else if(socket->state == TCPSocketState::FIN_WAIT1 || socket->state == TCPSocketState::FIN_WAIT2) {
					socket->state = TCPSocketState::CLOSED;
					socket->acknowledgement_number++;
					send_transmission_control_protocol_packet(socket, nullptr, 0, (uint16_t) TCPFlag::ACK);
					socket->disconnected();
				} else
					reset = true;
				break;


			case (uint16_t) TCPFlag::ACK:
				if(socket->state == TCPSocketState::SYN_RECEIVED) {
					socket->state = TCPSocketState::ESTABLISHED;
					socket->connected();
					return false;
				} else if(socket->state == TCPSocketState::FIN_WAIT1) {
					socket->state = TCPSocketState::FIN_WAIT2;
					return false;
				} else if(socket->state == TCPSocketState::CLOSE_WAIT) {
					socket->state = TCPSocketState::CLOSED;
					break;
				}

				if(msg->flags == (uint16_t) TCPFlag::ACK)
					break;

				// no break, because of piggybacking
				[[fallthrough]];

			default:

				// By default, handle the data

				if(big_endian_32(msg->sequence_number) == socket->acknowledgement_number) {

					reset = !(socket->handle_transmission_control_protocol_payload(
							payload_data + msg->header_size_32 * 4,
							size - msg->header_size_32 * 4));
					if(!reset) {
						uint32_t x = 0;                                                                      //The number of bytes to send back
						for(uint32_t i = msg->header_size_32 * 4;
						    i < size; i++)                          //Loop through the data
							if(payload_data[i] !=
							   0)                                                     //Check if the data is not 0
								x = i;                                                                  //Set the number of bytes to send back to the current index
						socket->acknowledgement_number += x - msg->header_size_32 * 4 +
						                                  1;               //Increment the acknowledgement number by the number of bytes to send back
						send_transmission_control_protocol_packet(socket, nullptr, 0,
						                                          (uint16_t) TCPFlag::ACK);                                          //Send the acknowledgement
					}
				} else {
					// data in wrong order
					reset = true;
				}

		}
	}


	if(reset)                                                                       //If the socket is to be reset
	{
		if(socket !=
		   nullptr)                                                             //If the socket exists then send a reset flag
		{
			send_transmission_control_protocol_packet(socket, nullptr, 0, (uint16_t) TCPFlag::RST);
		} else                                                                        //If it doesn't exist then create a new socket and send a reset flag
		{
			TCPSocket new_socket(this);                     //Create a new socket
			new_socket.remotePort = msg->src_port;                                         //Set the remote port
			new_socket.remote_ip = source_ip;                                                 //Set the remote IP
			new_socket.local_port = msg->dst_port;                                                  //Set the local port
			new_socket.local_ip = destination_ip;                                                     //Set the local IP
			new_socket.sequence_number = big_endian_32(
					msg->acknowledgement_number);              //Set the sequence number
			new_socket.acknowledgement_number =
					big_endian_32(msg->sequence_number) + 1;          //Set the acknowledgement number
			send_transmission_control_protocol_packet(&new_socket, nullptr, 0,
			                                          (uint16_t) TCPFlag::RST);          //Send the reset flag
		}
	}


	error_messages->write("TCP: Handled packet\n");

	if(socket != nullptr && socket->state ==
	                        TCPSocketState::CLOSED)                                        //If the socket is closed then remove it from the list
	{
		sockets.erase(socket);
		return true;
	}


	return false;
}

/**
 * @brief send a packet (Throught the provider)
 *
 * @param socket    The socket to send the packet from
 * @param data    The data to send
 * @param size   The size of the data
 * @param flags  The flags to send
 */
void TransmissionControlProtocolHandler::send_transmission_control_protocol_packet(TCPSocket* socket, const uint8_t* data, uint16_t size, uint16_t flags) {
	//Get the total size of the packet and the packet with the pseudo header
	uint16_t total_length = size + sizeof(TCPHeader);
	uint16_t length_incl_p_hdr = total_length + sizeof(TCPPseudoHeader);

	//Create a buffer for the packet
	auto* buffer = (uint8_t*) MemoryManager::kmalloc(length_incl_p_hdr);
	uint8_t* buffer2 =
			buffer + sizeof(TCPHeader) + sizeof(TCPPseudoHeader);

	//Create the headers
	auto* phdr = (TCPPseudoHeader*) buffer;
	auto* msg = (TCPHeader*) (buffer + sizeof(TCPPseudoHeader));

	//Size is translated into 32bit
	msg->header_size_32 = sizeof(TCPHeader) / 4;

	//Set the ports
	msg->src_port = big_endian_16(socket->local_port);
	msg->dst_port = big_endian_16(socket->remotePort);

	//Set TCP related data
	msg->acknowledgement_number = big_endian_32(socket->acknowledgement_number);
	msg->sequence_number = big_endian_32(socket->sequence_number);
	msg->reserved = 0;
	msg->flags = flags;
	msg->window_size = 0xFFFF;
	msg->urgent_ptr = 0;

	//Through the options allow for the MSS to be set
	msg->options = ((flags & (uint16_t) TCPFlag::SYN) != 0) ? 0xB4050402 : 0;

	//Increase the sequence number
	socket->sequence_number += size;

	// Check if the data is not null
	if(data != nullptr) {
		//Copy the data into the buffer
		for(int i = 0; i < size; i++)
			buffer2[i] = data[i];
	}

	//Set the pseudo header
	phdr->src_ip = socket->local_ip;
	phdr->dst_ip = socket->remote_ip;
	phdr->protocol = 0x0600;
	phdr->total_length = ((total_length & 0x00FF) << 8) | ((total_length & 0xFF00) >> 8);

	//Calculate the checksum
	msg->checksum = 0;
	msg->checksum = InternetProtocolHandler::checksum((uint16_t*) buffer, length_incl_p_hdr);


	//Send and then free the data
	send(socket->remote_ip, (uint8_t*) msg, total_length);
	MemoryManager::kfree(buffer);
}

/**
 * @brief connect to a remote host through the TCP protocol
 * @param ip The IP of the remote host
 * @param port The port of the remote host
 * @return The socket that is connected to the remote host, 0 if it failed
 */
TCPSocket* TransmissionControlProtocolHandler::connect(InternetProtocolAddress ip, TransmissionControlProtocolPort port) {
	//Create a new socket
	auto* socket = (TCPSocket*) MemoryManager::kmalloc(
			sizeof(TCPSocket));

	//If there is space for the socket
	if(socket != nullptr) {
		//Set the socket
		new(socket) TCPSocket(this);

		//Set local and remote addresses
		socket->remotePort = port;
		socket->remote_ip = ip;
		socket->local_port = free_ports++;
		socket->local_ip = internet_protocol_handler->get_internet_protocol_address();

		//Convert into big endian
		socket->remotePort = ((socket->remotePort & 0xFF00) >> 8) | ((socket->remotePort & 0x00FF) << 8);
		socket->local_port = ((socket->local_port & 0xFF00) >> 8) | ((socket->local_port & 0x00FF) << 8);

		//Set the socket into the socket array and then set its state
		sockets.push_back(socket);
		socket->state = TCPSocketState::SYN_SENT;

		//Dummy sequence number
		socket->sequence_number = 0xbeefcafe;

		//Send a sync packet
		send_transmission_control_protocol_packet(socket, nullptr, 0, (uint16_t) TCPFlag::SYN);
	}

	return socket;
}

/**
 * @brief connect to a remote host through the TCP protocol
 *
 * @param address The address to connect to in the form "IP:PORT"
 *
 * @return The socket that is connected to the remote host, nullptr if it failed
 *
 * @todo Implement string parsing for address
 */
TCPSocket* TransmissionControlProtocolHandler::connect(const string& address) {

	return nullptr;
}

/**
 * @brief Begin the disconnect process
 *
 * @param socket The socket to disconnect
 */
void TransmissionControlProtocolHandler::disconnect(TCPSocket* socket) {

	socket->state = TCPSocketState::FIN_WAIT1;                            //Begin fin wait sequence
	send_transmission_control_protocol_packet(socket, nullptr, 0, (uint16_t) TCPFlag::FIN +
	                                                              (uint16_t) TCPFlag::ACK);            //Send FIN|ACK packet
	socket->sequence_number++;                             //Increase the sequence number
}

/**
 * @brief Begin listening on a port
 *
 * @param port The port to listen on
 * @return The socket that will handle the connection
 */
TCPSocket* TransmissionControlProtocolHandler::listen(uint16_t port) {
	//Create a new socket
	auto* socket = (TCPSocket*) MemoryManager::kmalloc(
			sizeof(TCPSocket));

	//If there is space for the socket
	if(socket != nullptr) {
		//Set the socket
		new(socket) TCPSocket(this);

		//Configure the socket
		socket->state = TCPSocketState::LISTEN;
		socket->local_ip = internet_protocol_handler->get_internet_protocol_address();
		socket->local_port = ((port & 0xFF00) >> 8) | ((port & 0x00FF) << 8);

		//Add the socket to the socket array
		sockets.push_back(socket);
	}

	//Return the socket
	return socket;
}


/**
 * @brief bind a data handler to this socket
 *
 * @param socket The socket to bind the handler to
 * @param handler The handler to bind
 */
void TransmissionControlProtocolHandler::bind(TCPSocket* socket, TCPPayloadHandler* handler) {
	socket->connect_event_handler(handler);
}


/**
 * @brief Construct a new Data Received Event object
 *
 * @param socket The socket that received the data
 * @param data The data that was received
 * @param size The size of the data

 */
DataReceivedEvent::DataReceivedEvent(TCPSocket* socket, uint8_t* data, uint16_t size)
		: Event(TCPPayloadHandlerEvents::DATA_RECEIVED) {
	this->socket = socket;
	this->data = data;
	this->size = size;
}

DataReceivedEvent::~DataReceivedEvent() = default;

/**
 * @brief Construct a new connected Event object
 *
 * @param socket The socket that is connected
 */
ConnectedEvent::ConnectedEvent(TCPSocket* socket)
		: Event(TCPPayloadHandlerEvents::CONNECTED) {
	this->socket = socket;
}

ConnectedEvent::~ConnectedEvent() = default;

/**
 * @brief Construct a new disconnected Event object
 *
 * @param socket The socket that is disconnected
 */
DisconnectedEvent::DisconnectedEvent(TCPSocket* socket)
		: Event(TCPPayloadHandlerEvents::DISCONNECTED) {
	this->socket = socket;
}

DisconnectedEvent::~DisconnectedEvent() = default;