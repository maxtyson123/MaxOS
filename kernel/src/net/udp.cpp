/**
 * @file udp.cpp
 * @brief Implementation of the User Datagram Protocol (UDP) handler and socket classes
 *
 * @date 24 November 2022
 * @author Max Tyson
 */

#include <net/udp.h>

using namespace MaxOS;
using namespace MaxOS::net;
using namespace MaxOS::common;
using namespace MaxOS::memory;

///__Handler__

UDPPayloadHandler::UDPPayloadHandler() = default;

UDPPayloadHandler::~UDPPayloadHandler() = default;

/**
 * @brief Handle the recivement of a UDP message
 *
 * @param socket The socket that received the message
 * @param data The data received
 * @param size The size of the data received
 */
void UDPPayloadHandler::handle_user_datagram_protocol_message(UDPSocket* socket, uint8_t* data, uint16_t size) {

}

/**
 * @brief Event handler for UDP payload events
 *
 * @param event The event that was raised
 * @return The event that was raised
 */
Event<UDPEvents>* UDPPayloadHandler::on_event(Event<UDPEvents> *event) {

    switch (event -> type) {
        case UDPEvents::DATA_RECEIVED:
	        handle_user_datagram_protocol_message(((UDPDataReceivedEvent*) event)->socket,
	                                              ((UDPDataReceivedEvent*) event)->data,
	                                              ((UDPDataReceivedEvent*) event)->size);
            break;
        default:
            break;
    }

    return event;
}

///__Socket__


UDPSocket::UDPSocket() {

    //Set the instance variables
    listening = false;

}

UDPSocket::~UDPSocket() = default;

/**
 * @brief Handle the recivement of a UDP payload
 *
 * @param data The data received
 * @param size The size of the data received
 */
void UDPSocket::handle_user_datagram_protocol_payload(uint8_t *data, uint16_t size) {

    // Create the event
    auto* event = new UDPDataReceivedEvent(this, data, size);
    raise_event(event);
    MemoryManager::kfree(event);

}

/**
 * @brief send data through the UDP socket
 *
 * @param data The data to send
 * @param size The size of the data
 */
void UDPSocket::send(uint8_t *data, uint16_t size) {

	user_datagram_protocol_handler->send(this, data, size);

}

/**
 * @brief disconnect the UDP socket
 */
void UDPSocket::disconnect() {

	user_datagram_protocol_handler->disconnect(this);

}

///__Provider__

UserDatagramProtocolPort UserDatagramProtocolHandler::free_ports = 0x8000;

/**
 * @brief Construct a new User Datagram Protocol Handler object
 *
 * @param internet_protocol_handler The Internet protocol handler
 * @param error_messages Where to write error messages
 */
UserDatagramProtocolHandler::UserDatagramProtocolHandler(InternetProtocolHandler* internet_protocol_handler, OutputStream* error_messages)
: IPV4PayloadHandler(internet_protocol_handler, 0x11)    //0x11 is the UDP protocol number
{
    this -> errorMessages = error_messages;
}

UserDatagramProtocolHandler::~UserDatagramProtocolHandler() = default;

/**
 * @brief Handle the recivement of an UDP packet
 *
 * @param source_ip The source IP address in big endian
 * @param destination_ip  The destination IP address in big endian
 * @param payload_data The UDP payload
 * @param size The size of the UDP payload
 * @return True if the packet is to be sent back to the sender
 */
bool UserDatagramProtocolHandler::handle_internet_protocol_payload(net::InternetProtocolAddress source_ip, net::InternetProtocolAddress destination_ip, uint8_t* payload_data, uint32_t size) {

    //Check the size
    if(size < sizeof(UDPHeader)) {
        return false;
    }

    //Get the header
    auto* header = (UDPHeader*)payload_data;

    //Set the local and remote ports
    uint16_t local_port = header -> destination_port;
    uint16_t remote_port = header -> source_port;

    UDPSocket* socket = nullptr;                     //The socket that will be used
    for(auto & current_socket : sockets) {
        if(current_socket->local_port == local_port                  //If the local port (header dst, our port) is the same as the local port of the socket
        && current_socket->local_ip == destination_ip                     //If the local IP (packet dst, our IP) is the same as the local IP of the socket
        && current_socket->listening)                              //If the socket is listening
        {

            socket = current_socket;                               //Set the socket to the socket that is being checked
            socket->listening = false;                         //Set the socket to not listening, as it is now in use
            socket->remote_port = remote_port;                   //Set the remote port of the socket to the remote port of the packet
            socket->remote_ip = source_ip;                       //Set the remote IP of the socket to the remote IP of the packet

        }else if(current_socket->local_port == local_port            //If the local port (header dst, our port) is the same as the local port of the socket
              && current_socket->local_ip == destination_ip              //If the local IP (packet dst, our IP) is the same as the local IP of the socket
              && current_socket->remote_port == remote_port         //If the remote port (header src, their port) is the same as the remote port of the socket
              && current_socket->remote_ip == source_ip)            //If the remote IP (packet src, their IP) is the same as the remote IP of the socket
        {
            socket = current_socket;                               //Set the socket to the current socket
        }

    }

    if(socket != nullptr) {                                          //If the socket is not null then pass the data to the socket
	    socket->handle_user_datagram_protocol_payload(payload_data + sizeof(UDPHeader),
	                                                  size - sizeof(UDPHeader));
    }

    //UDP doesn't send back packets, so always return false
    return false;

}

/**
 * Connects the socket to the remote IP and port
 * @param ip The remote IP address in big endian
 * @param port The remote port
 * @return The socket that was connected
 */
UDPSocket *UserDatagramProtocolHandler::connect(uint32_t ip, uint16_t port) {


    auto* socket = (UDPSocket*)MemoryManager::kmalloc(sizeof(UDPSocket));   //Allocate memory for the socket

    if(socket != nullptr) //If the socket was created
    {
        new (socket) UDPSocket();    //Create the socket

        //Configure the socket
        socket -> remote_port = port;                                    //Port to that application wants to connect to
        socket -> remote_ip = ip;                                        //IP to that application wants to connect to
        socket -> local_port = free_ports++;                               //Port that we will use to connect to the remote application  (note, local port doesnt have to be the same as remote)
        socket -> local_ip = internet_protocol_handler->get_internet_protocol_address();    //IP that we will use to connect to the remote application
        socket -> user_datagram_protocol_handler = this;                    //Set the UDP handler

        sockets.push_back(socket);                                       //Add the socket to the list of sockets
    }

    return socket;                                        //Return the socket
}

/**
 * @brief Connects to a remote host through the UDP protocol
 *
 * @param address The address to connect to in the form "IP:PORT"
 * @return The socket that is connected to the remote host, nullptr if it failed
 *
 * @todo Implement string parsing to extract IP and port
 */
UDPSocket *UserDatagramProtocolHandler::connect(const string& address) {

  return nullptr;
}

/**
 * @brief Listens for incoming packets on the port
 *
 * @param port The port to listen on
 * @return The socket that is listening
 */
UDPSocket *UserDatagramProtocolHandler::listen(uint16_t port) {

    auto* socket = (UDPSocket*)MemoryManager::kmalloc(sizeof(UDPSocket));   //Allocate memory for the socket

    if(socket != nullptr) //If the socket was created
    {
        new (socket) UDPSocket();    //Create the socket

        //Configure the socket
        socket -> listening = true;                                     //Set the socket to listening
        socket -> local_port = port;                                     //Port that we will use to connect to the remote application  (note, local port doesnt have to be the same as remote)
        socket -> local_ip = internet_protocol_handler->get_internet_protocol_address();    //IP that we will use to connect to the remote application
        socket -> user_datagram_protocol_handler = this;                    //Set the UDP handler

        sockets.push_back(socket);                                       //Add the socket to the list of sockets
    }

    return socket;                                        //Return the socket

}

/**
 * @brief Disconnects the socket from the remote IP and port
 *
 * @param socket The socket to disconnect
 */
void UserDatagramProtocolHandler::disconnect(UDPSocket *socket) {


    for(Vector<UDPSocket*>::iterator current_socket = sockets.begin(); current_socket != sockets.end(); current_socket++) {
        if((*current_socket) == socket)                               //If the socket is the same as the socket that is being checked
        {
            sockets.erase(current_socket);                            //Remove the socket from the list of sockets
            MemoryManager::kfree(socket);      //Free the socket
            break;                                                   //Break out of the loop
        }
    }

}

/**
 * @brief Sends a packet to the remote IP and port
 *
 * @param socket The socket to send the packet from
 * @param data The data to send
 * @param size The size of the data
 */
void UserDatagramProtocolHandler::send(UDPSocket *socket, const uint8_t *data, uint16_t size) {

    uint16_t total_size = sizeof(UDPHeader) + size;                                 //Get the total size of the packet
    auto* buffer = (uint8_t*)MemoryManager::kmalloc(total_size);          //Allocate memory for the packet
    uint8_t* buffer2 = buffer + sizeof(UDPHeader);                                 //Get the buffer that will be used to store the data

    auto* header = (UDPHeader*)buffer;                       //Create the header of the packet

    //Set the header
    header -> source_port = socket -> local_port;                                                    //Set the source port to the local port of the socket    (this is the port that the packet will be sent from)
    header -> destination_port = socket -> remote_port;                                              //Set the destination port to the remote port of the socket (this is the port that the packet will be sent to)
    header -> length = ((total_size & 0x00FF) << 8) | ((total_size & 0xFF00) >> 8);                  //Set the length of the packet

    // Convert the ports into big endian
    header -> source_port = ((header -> source_port & 0x00FF) << 8) | ((header -> source_port & 0xFF00) >> 8);
    header -> destination_port = ((header -> destination_port & 0x00FF) << 8) | ((header -> destination_port & 0xFF00) >> 8);

    //Copy the data to the buffer
    for (int i = 0; i < size; ++i) {                                                               //Loop through the data
        buffer2[i] = data[i];                                                                      //Copy the data to the buffer
    }

    //Set the checksum
    header -> checksum = 0;                                                                        //Set the checksum to 0, this is becuase UDP doesnt have to have a checksum

    //Send the packet
	IPV4PayloadHandler::send(socket->remote_ip, buffer, total_size);

    //Free the buffer
    MemoryManager::kfree(buffer);

}

/**
 * @brief Binds a handler to the socket
 *
 * @param socket The socket to bind the handler to
 * @param udp_payload_handler The handler to bind
 */
void UserDatagramProtocolHandler::bind(UDPSocket *socket, UDPPayloadHandler *udp_payload_handler) {

  socket->m_handlers.push_back(udp_payload_handler);                                                                //Set the handler of the socket to the handler that was passed in


}

/**
 * @brief Construct a new UDP Data Received Event object
 *
 * @param socket The socket that received the data
 * @param data The data received
 * @param size The size of the data received
 */
UDPDataReceivedEvent::UDPDataReceivedEvent(UDPSocket *socket, uint8_t *data, uint16_t size)
: Event(UDPEvents::DATA_RECEIVED)
{
    this -> socket = socket;    //Set the socket
    this -> data = data;        //Set the data
    this -> size = size;        //Set the size

}

UDPDataReceivedEvent::~UDPDataReceivedEvent() = default;
