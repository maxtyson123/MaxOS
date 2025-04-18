#include <net/udp.h>

using namespace MaxOS;
using namespace MaxOS::net;
using namespace MaxOS::common;
using namespace MaxOS::memory;

///__Handler__

UserDatagramProtocolPayloadHandler::UserDatagramProtocolPayloadHandler() = default;

UserDatagramProtocolPayloadHandler::~UserDatagramProtocolPayloadHandler() = default;

void UserDatagramProtocolPayloadHandler::handleUserDatagramProtocolMessage(UserDatagramProtocolSocket*, uint8_t *, uint16_t) {

}

Event<UserDatagramProtocolEvents>* UserDatagramProtocolPayloadHandler::on_event(Event<UserDatagramProtocolEvents> *event) {

    switch (event -> type) {
        case UserDatagramProtocolEvents::DATA_RECEIVED:
            handleUserDatagramProtocolMessage(((UDPDataReceivedEvent*)event) -> socket, ((UDPDataReceivedEvent*)event) -> data, ((UDPDataReceivedEvent*)event) -> size);
            break;
        default:
            break;
    }

    return event;
}

///__Socket__


UserDatagramProtocolSocket::UserDatagramProtocolSocket() {

    //Set the instance variables
    listening = false;

}

UserDatagramProtocolSocket::~UserDatagramProtocolSocket() = default;

void UserDatagramProtocolSocket::handleUserDatagramProtocolPayload(uint8_t *data, uint16_t size) {

    // Create the event
    auto* event = new UDPDataReceivedEvent(this, data, size);
    raise_event(event);
    MemoryManager::kfree(event);

}

void UserDatagramProtocolSocket::Send(uint8_t *data, uint16_t size) {

    userDatagramProtocolHandler -> Send(this, data, size);

}

void UserDatagramProtocolSocket::Disconnect() {

    userDatagramProtocolHandler ->Disconnect(this);

}

///__Provider__

UserDatagramProtocolPort UserDatagramProtocolHandler::freePorts = 0x8000;

UserDatagramProtocolHandler::UserDatagramProtocolHandler(InternetProtocolHandler* internetProtocolHandler, OutputStream* errorMessages)
: InternetProtocolPayloadHandler(internetProtocolHandler, 0x11)    //0x11 is the UDP protocol number
{
    this -> errorMessages = errorMessages;
}

UserDatagramProtocolHandler::~UserDatagramProtocolHandler() = default;
/**
 * @brief Handle the recivement of an UDP packet
 *
 * @param srcIP_BE The source IP address in big endian
 * @param dstIP_BE  The destination IP address in big endian
 * @param internetprotocolPayload The UDP payload
 * @param size The size of the UDP payload
 * @return True if the packet is to be sent back to the sender
 */
bool UserDatagramProtocolHandler::handleInternetProtocolPayload(InternetProtocolAddress sourceIP, InternetProtocolAddress destinationIP, uint8_t* payloadData, uint32_t size) {

    //Check the size
    if(size < sizeof(UserDatagramProtocolHeader)) {
        return false;
    }

    //Get the header
    auto* header = (UserDatagramProtocolHeader*)payloadData;

    //Set the local and remote ports
    uint16_t localPort = header -> destinationPort;
    uint16_t remotePort = header -> sourcePort;

    UserDatagramProtocolSocket* socket = nullptr;                     //The socket that will be used
    for(auto & currentSocket : sockets) {
        if(currentSocket->localPort == localPort                  //If the local port (header dst, our port) is the same as the local port of the socket
        && currentSocket->localIP == destinationIP                     //If the local IP (packet dst, our IP) is the same as the local IP of the socket
        && currentSocket->listening)                              //If the socket is listening
        {

            socket = currentSocket;                               //Set the socket to the socket that is being checked
            socket->listening = false;                         //Set the socket to not listening, as it is now in use
            socket->remotePort = remotePort;                   //Set the remote port of the socket to the remote port of the packet
            socket->remoteIP = sourceIP;                       //Set the remote IP of the socket to the remote IP of the packet

        }else if(currentSocket->localPort == localPort            //If the local port (header dst, our port) is the same as the local port of the socket
              &&  currentSocket->localIP == destinationIP              //If the local IP (packet dst, our IP) is the same as the local IP of the socket
              &&  currentSocket->remotePort == remotePort         //If the remote port (header src, their port) is the same as the remote port of the socket
              &&  currentSocket->remoteIP == sourceIP)            //If the remote IP (packet src, their IP) is the same as the remote IP of the socket
        {
            socket = currentSocket;                               //Set the socket to the current socket
        }

    }

    if(socket != nullptr) {                                          //If the socket is not null then pass the data to the socket
        socket->handleUserDatagramProtocolPayload(payloadData + sizeof(UserDatagramProtocolHeader), size - sizeof(UserDatagramProtocolHeader));
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
UserDatagramProtocolSocket *UserDatagramProtocolHandler::Connect(uint32_t ip, uint16_t port) {


    auto* socket = (UserDatagramProtocolSocket*)MemoryManager::kmalloc(sizeof(UserDatagramProtocolSocket));   //Allocate memory for the socket

    if(socket != nullptr) //If the socket was created
    {
        new (socket) UserDatagramProtocolSocket();    //Create the socket

        //Configure the socket
        socket -> remotePort = port;                                    //Port to that application wants to connect to
        socket -> remoteIP = ip;                                        //IP to that application wants to connect to
        socket -> localPort = freePorts++;                               //Port that we will use to connect to the remote application  (note, local port doesnt have to be the same as remote)
        socket -> localIP = internetProtocolHandler -> GetInternetProtocolAddress();    //IP that we will use to connect to the remote application
        socket -> userDatagramProtocolHandler = this;                    //Set the UDP handler

        sockets.push_back(socket);                                       //Add the socket to the list of sockets
    }

    return socket;                                        //Return the socket
}

UserDatagramProtocolSocket *UserDatagramProtocolHandler::Connect(const string&) {

  //TODO NEW STRING PARSEING

  return nullptr;
}

/**
 * @brief Listens for incoming packets on the port
 *
 * @param port The port to listen on
 * @return The socket that is listening
 */
UserDatagramProtocolSocket *UserDatagramProtocolHandler::Listen(uint16_t port) {

    auto* socket = (UserDatagramProtocolSocket*)MemoryManager::kmalloc(sizeof(UserDatagramProtocolSocket));   //Allocate memory for the socket

    if(socket != nullptr) //If the socket was created
    {
        new (socket) UserDatagramProtocolSocket();    //Create the socket

        //Configure the socket
        socket -> listening = true;                                     //Set the socket to listening
        socket -> localPort = port;                                     //Port that we will use to connect to the remote application  (note, local port doesnt have to be the same as remote)
        socket -> localIP = internetProtocolHandler -> GetInternetProtocolAddress();    //IP that we will use to connect to the remote application
        socket -> userDatagramProtocolHandler = this;                    //Set the UDP handler

        sockets.push_back(socket);                                       //Add the socket to the list of sockets
    }

    return socket;                                        //Return the socket

}

/**
 * @brief Disconnects the socket from the remote IP and port
 *
 * @param socket The socket to disconnect
 */
void UserDatagramProtocolHandler::Disconnect(UserDatagramProtocolSocket *socket) {


    for(Vector<UserDatagramProtocolSocket*>::iterator currentSocket = sockets.begin(); currentSocket != sockets.end(); currentSocket++) {
        if((*currentSocket) == socket)                               //If the socket is the same as the socket that is being checked
        {
            sockets.erase(currentSocket);                            //Remove the socket from the list of sockets
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
void UserDatagramProtocolHandler::Send(UserDatagramProtocolSocket *socket, const uint8_t *data, uint16_t size) {

    uint16_t totalSize = sizeof(UserDatagramProtocolHeader) + size;                                 //Get the total size of the packet
    auto* buffer = (uint8_t*)MemoryManager::kmalloc(totalSize);          //Allocate memory for the packet
    uint8_t* buffer2 = buffer + sizeof(UserDatagramProtocolHeader);                                 //Get the buffer that will be used to store the data

    auto* header = (UserDatagramProtocolHeader*)buffer;                       //Create the header of the packet

    //Set the header
    header -> sourcePort = socket -> localPort;                                                    //Set the source port to the local port of the socket    (this is the port that the packet will be sent from)
    header -> destinationPort = socket -> remotePort;                                              //Set the destination port to the remote port of the socket (this is the port that the packet will be sent to)
    header -> length = ((totalSize & 0x00FF) << 8) | ((totalSize & 0xFF00) >> 8);                  //Set the length of the packet

    // Convert the ports into big endian
    header -> sourcePort = ((header -> sourcePort & 0x00FF) << 8) | ((header -> sourcePort & 0xFF00) >> 8);
    header -> destinationPort = ((header -> destinationPort & 0x00FF) << 8) | ((header -> destinationPort & 0xFF00) >> 8);

    //Copy the data to the buffer
    for (int i = 0; i < size; ++i) {                                                               //Loop through the data
        buffer2[i] = data[i];                                                                      //Copy the data to the buffer
    }

    //Set the checksum
    header -> checksum = 0;                                                                        //Set the checksum to 0, this is becuase UDP doesnt have to have a checksum

    //Send the packet
    InternetProtocolPayloadHandler::Send(socket->remoteIP, buffer, totalSize);

    //Free the buffer
    MemoryManager::kfree(buffer);

}

/**
 * @brief Binds a handler to the socket
 *
 * @param socket The socket to bind the handler to
 * @param userDatagramProtocolHandler The handler to bind
 */
void UserDatagramProtocolHandler::Bind(UserDatagramProtocolSocket *socket, UserDatagramProtocolPayloadHandler *userDatagramProtocolPayloadHandler) {

  socket->m_handlers.push_back(userDatagramProtocolPayloadHandler);                                                                //Set the handler of the socket to the handler that was passed in


}

/// ___ Events ___ ///
UDPDataReceivedEvent::UDPDataReceivedEvent(UserDatagramProtocolSocket *socket, uint8_t *data, uint16_t size)
: Event(UserDatagramProtocolEvents::DATA_RECEIVED)
{
    this -> socket = socket;    //Set the socket
    this -> data = data;        //Set the data
    this -> size = size;        //Set the size

}

UDPDataReceivedEvent::~UDPDataReceivedEvent() = default;
