//
// Created by 98max on 12/9/2022.
//

#include <net/tcp.h>
#include "net/udp.h"


using namespace MaxOS;
using namespace MaxOS::net;
using namespace MaxOS::common;
using namespace MaxOS::memory;

///__Handler__///

TransmissionControlProtocolPayloadHandler::TransmissionControlProtocolPayloadHandler()
= default;

TransmissionControlProtocolPayloadHandler::~TransmissionControlProtocolPayloadHandler()
= default;

void TransmissionControlProtocolPayloadHandler::handleTransmissionControlProtocolPayload(TransmissionControlProtocolSocket*, uint8_t*, uint16_t) {

}

void TransmissionControlProtocolPayloadHandler::Connected(TransmissionControlProtocolSocket*) {

}

void TransmissionControlProtocolPayloadHandler::Disconnected(TransmissionControlProtocolSocket*) {

}

Event<TransmissionControlProtocolPayloadHandlerEvents>* TransmissionControlProtocolPayloadHandler::on_event(Event<TransmissionControlProtocolPayloadHandlerEvents> *event) {

    switch (event -> type)
    {
      case TransmissionControlProtocolPayloadHandlerEvents::CONNECTED:
            Connected(((ConnectedEvent*)event) -> socket);
            break;
        case TransmissionControlProtocolPayloadHandlerEvents::DISCONNECTED:
            Disconnected(((DisconnectedEvent*)event) -> socket);
            break;
        case TransmissionControlProtocolPayloadHandlerEvents::DATA_RECEIVED:
            handleTransmissionControlProtocolPayload(((DataReceivedEvent*)event) -> socket, ((DataReceivedEvent*)event) -> data, ((DataReceivedEvent*)event) -> size);
            break;
    }

    return event;
}



///__Socket__///

TransmissionControlProtocolSocket::TransmissionControlProtocolSocket(TransmissionControlProtocolHandler *transmissionControlProtocolHandler)
{
    //Set the default values
    this -> transmissionControlProtocolHandler = transmissionControlProtocolHandler;

    //Closed as default
    state = TCPSocketState::CLOSED;
}

TransmissionControlProtocolSocket::~TransmissionControlProtocolSocket() = default;

/**
 * @brief Handle the TCP message (socket end)
 *
 * @param data The datah
 * @param size The size of the data
 * @return True if the connection is to be terminated after hadnling or false if not
 */
bool TransmissionControlProtocolSocket::handleTransmissionControlProtocolPayload(uint8_t* data, uint16_t size)
{
    auto* event = new DataReceivedEvent(this, data, size);
    raise_event(event);
    MemoryManager::kfree(event);
    return true;
}

/**
 * @brief Send data over the socket
 *
 * @param data The data to send
 * @param size The size of the data
 */
void TransmissionControlProtocolSocket::Send(uint8_t* data, uint16_t size)
{
    //Wait for the socket to be connected
    while(state != TCPSocketState::ESTABLISHED);

    //Pass the data to the backend
    transmissionControlProtocolHandler -> sendTransmissionControlProtocolPacket(this, data, size, (uint16_t)TCPFlag::PSH | (uint16_t)TCPFlag::ACK);
}

/**
 * @brief Disconnect the socket
 */
void TransmissionControlProtocolSocket::Disconnect()
{
    transmissionControlProtocolHandler -> Disconnect(this);
}

void TransmissionControlProtocolSocket::Disconnected() {
    auto* event = new DisconnectedEvent(this);
    raise_event(event);
    MemoryManager::kfree(event);

}

void TransmissionControlProtocolSocket::Connected() {
    auto* event = new ConnectedEvent(this);
    raise_event(event);
    MemoryManager::kfree(event);

}

///__Handler__///

TransmissionControlProtocolPort TransmissionControlProtocolHandler::freePorts = 0x8000;

TransmissionControlProtocolHandler::TransmissionControlProtocolHandler(
        MaxOS::net::InternetProtocolHandler *internetProtocolHandler, OutputStream *errorMessages)
: InternetProtocolPayloadHandler(internetProtocolHandler, 0x06)
{
    this -> errorMessages = errorMessages;
    
}

TransmissionControlProtocolHandler::~TransmissionControlProtocolHandler() = default;

//Shorthand for BE
uint32_t bigEndian32(uint32_t x)
{
    return ((x & 0xFF000000) >> 24)
           | ((x & 0x00FF0000) >> 8)
           | ((x & 0x0000FF00) << 8)
           | ((x & 0x000000FF) << 24);
}

uint32_t bigEndian16(uint16_t x)
{
    return ((x & 0xFF00) >> 8)
           | ((x & 0x00FF) << 8);
}

/**
 * @brief Handle the TCP message (provider end)
 *
 * @param srcIP_BE The source IP address
 * @param dstIP_BE The destination IP address
 * @param internetprotocolPayload The payload
 * @param size The size of the payload
 * @return True if data is to be sent back or false if not
 */
bool TransmissionControlProtocolHandler::handleInternetProtocolPayload(InternetProtocolAddress sourceIP, InternetProtocolAddress destinationIP, uint8_t* payloadData, uint32_t size)
{

    errorMessages -> write("TCP: Handling TCP message\n");

    //Check if the size is too small
    if(size < 13)
    {
        return false;
    }

    // If it's smaller than the header, return
    if(size < 4*payloadData[12]/16)             // The lower 4 bits of the 13th byte is the header length
    {
        return false;
    }

    //Get the header
    auto* msg = (TransmissionControlProtocolHeader*)payloadData;

    //Get the connection values (convert to host endian)
    uint16_t localPort = bigEndian16(msg -> dstPort);
    uint16_t remotePort = bigEndian16(msg -> srcPort);

    //Create the socket
    TransmissionControlProtocolSocket* socket = nullptr;

    for(auto & currentSocket : sockets)
    {
        if( currentSocket -> localPort == localPort                               //Check if the local port is the same as the destination port
        &&  currentSocket -> localIP == destinationIP                                  //Check if the local IP is the same as the destination IP
        &&  currentSocket -> state == TCPSocketState::LISTEN                                           //Check if the socket is in the LISTEN state
        && (((msg -> flags) & ((uint16_t)TCPFlag::SYN | (uint16_t)TCPFlag::ACK)) == (uint16_t)TCPFlag::SYN))                                       //Check if the SYN flag is set (allow for acknoweldgement)
        {
            socket = currentSocket;
        }
        else if( currentSocket -> localPort == localPort                          //Check if the local port is the same as the destination port
             &&  currentSocket -> localIP == destinationIP                             //Check if the local IP is the same as the destination IP
             &&  currentSocket -> remotePort == remotePort                         //Check if the remote port is the same as the source port
             &&  currentSocket -> remoteIP == destinationIP)                           //Check if the remote IP is the same as the source IP
        {
            socket = currentSocket;
        }
    }


    bool reset = false;

    //Check if the socket is found and if the socket wants to reset
    if(socket != nullptr && msg -> flags & (uint16_t)TCPFlag::RST)
    {
        socket -> state = TCPSocketState::CLOSED;
        socket -> Disconnected();
    }

    //Check if the socket is found and if the socket is not closed
    if(socket != nullptr && socket -> state != TCPSocketState::CLOSED)
    {
        switch((msg -> flags) & ((uint16_t)TCPFlag::SYN | (uint16_t)TCPFlag::ACK | (uint16_t)TCPFlag::FIN))
        {
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

            case (uint16_t)TCPFlag::SYN:
                if(socket -> state == TCPSocketState::LISTEN)
                {
                    socket -> state = TCPSocketState::SYN_RECEIVED;
                    socket -> remotePort = msg -> srcPort;
                    socket -> remoteIP = sourceIP;
                    socket -> acknowledgementNumber = bigEndian32( msg -> sequenceNumber ) + 1;
                    socket -> sequenceNumber = 0xbeefcafe;
                    sendTransmissionControlProtocolPacket(socket, nullptr,0, (uint16_t)TCPFlag::SYN | (uint16_t)TCPFlag::ACK);
                    socket -> sequenceNumber++;
                }
                else
                    reset = true;
                break;


            case (uint16_t)TCPFlag::SYN | (uint16_t)TCPFlag::ACK:
                if(socket -> state == TCPSocketState::SYN_SENT)
                {
                    socket -> state = TCPSocketState::ESTABLISHED;
                    socket -> acknowledgementNumber = bigEndian32( msg -> sequenceNumber ) + 1;
                    socket -> sequenceNumber++;
                    sendTransmissionControlProtocolPacket(socket, nullptr,0, (uint16_t)TCPFlag::ACK);
                }
                else
                    reset = true;
                break;


            case (uint16_t)TCPFlag::SYN | (uint16_t)TCPFlag::FIN:
            case (uint16_t)TCPFlag::SYN | (uint16_t)TCPFlag::FIN | (uint16_t)TCPFlag::ACK:
                reset = true;
                break;


            case (uint16_t)TCPFlag::FIN:
            case (uint16_t)TCPFlag::FIN | (uint16_t)TCPFlag::ACK:
                if(socket -> state == TCPSocketState::ESTABLISHED)
                {
                    socket -> state = TCPSocketState::CLOSE_WAIT;
                    socket -> acknowledgementNumber++;
                    sendTransmissionControlProtocolPacket(socket, nullptr,0, (uint16_t)TCPFlag::ACK);
                    sendTransmissionControlProtocolPacket(socket, nullptr,0, (uint16_t)TCPFlag::FIN | (uint16_t)TCPFlag::ACK);
                    socket -> Disconnected();
                }
                else if(socket -> state == TCPSocketState::CLOSE_WAIT)
                {
                    socket -> state = TCPSocketState::CLOSED;
                }
                else if(socket -> state == TCPSocketState::FIN_WAIT1 || socket -> state == TCPSocketState::FIN_WAIT2)
                {
                    socket -> state = TCPSocketState::CLOSED;
                    socket -> acknowledgementNumber++;
                    sendTransmissionControlProtocolPacket(socket, nullptr,0, (uint16_t)TCPFlag::ACK);
                    socket -> Disconnected();
                }
                else
                    reset = true;
                break;


            case (uint16_t)TCPFlag::ACK:
                if(socket -> state == TCPSocketState::SYN_RECEIVED)
                {
                    socket -> state = TCPSocketState::ESTABLISHED;
                    socket -> Connected();
                    return false;
                }
                else if(socket -> state == TCPSocketState::FIN_WAIT1)
                {
                    socket -> state = TCPSocketState::FIN_WAIT2;
                    return false;
                }
                else if(socket -> state == TCPSocketState::CLOSE_WAIT)
                {
                    socket -> state = TCPSocketState::CLOSED;
                    break;
                }

                if(msg -> flags == (uint16_t)TCPFlag::ACK)
                    break;

                // no break, because of piggybacking
                [[fallthrough]];

            default:

                //By default handle the data

                if(bigEndian32(msg -> sequenceNumber) == socket -> acknowledgementNumber)
                {

                    reset = !(socket -> handleTransmissionControlProtocolPayload(payloadData + msg -> headerSize32*4,size - msg -> headerSize32*4));
                    if(!reset)
                    {
                        uint32_t x = 0;                                                                      //The number of bytes to send back
                        for(uint32_t i = msg -> headerSize32*4; i < size; i++)                          //Loop through the data
                            if(payloadData[i] != 0)                                                     //Check if the data is not 0
                                x = i;                                                                  //Set the number of bytes to send back to the current index
                        socket -> acknowledgementNumber += x - msg -> headerSize32*4 + 1;               //Increment the acknowledgement number by the number of bytes to send back
                        sendTransmissionControlProtocolPacket(socket, nullptr,0, (uint16_t)TCPFlag::ACK);                                          //Send the acknowledgement
                    }
                }
                else
                {
                    // data in wrong order
                    reset = true;
                }

        }
    }



    if(reset)                                                                       //If the socket is to be reset
    {
        if(socket != nullptr)                                                             //If the socket exists then send a reset flag
        {
            sendTransmissionControlProtocolPacket(socket, nullptr,0, (uint16_t)TCPFlag::RST);
        }
        else                                                                        //If it doesn't exist then create a new socket and send a reset flag
        {
            TransmissionControlProtocolSocket new_socket(this);                     //Create a new socket
            new_socket.remotePort = msg -> srcPort;                                         //Set the remote port
            new_socket.remoteIP = sourceIP;                                                 //Set the remote IP
            new_socket.localPort = msg -> dstPort;                                                  //Set the local port
            new_socket.localIP = destinationIP;                                                     //Set the local IP
            new_socket.sequenceNumber = bigEndian32(msg -> acknowledgementNumber);              //Set the sequence number
            new_socket.acknowledgementNumber = bigEndian32(msg -> sequenceNumber) + 1;          //Set the acknowledgement number
            sendTransmissionControlProtocolPacket(&new_socket, nullptr,0, (uint16_t)TCPFlag::RST);          //Send the reset flag
        }
    }


    errorMessages ->write("TCP: Handled packet\n");

    if(socket != nullptr && socket -> state == TCPSocketState::CLOSED)                                        //If the socket is closed then remove it from the list
    {
        sockets.erase(socket);
        return true;
    }



    return false;
}

/**
 * @brief Send a packet (Throught the provider)
 *
 * @param socket    The socket to send the packet from
 * @param data    The data to send
 * @param size   The size of the data
 * @param flags  The flags to send
 */
void TransmissionControlProtocolHandler::sendTransmissionControlProtocolPacket(TransmissionControlProtocolSocket* socket, const uint8_t* data, uint16_t size, uint16_t flags)
{
    //Get the total size of the packet and the packet with the pseudo header
    uint16_t totalLength = size + sizeof(TransmissionControlProtocolHeader);
    uint16_t lengthInclPHdr = totalLength + sizeof(TransmissionControlProtocolPseudoHeader);

    //Create a buffer for the packet
    auto* buffer = (uint8_t*)MemoryManager::kmalloc(lengthInclPHdr);
    uint8_t* buffer2 = buffer + sizeof(TransmissionControlProtocolHeader) + sizeof(TransmissionControlProtocolPseudoHeader);

    //Create the headers
    auto* phdr = (TransmissionControlProtocolPseudoHeader*)buffer;
    auto* msg = (TransmissionControlProtocolHeader*)(buffer + sizeof(TransmissionControlProtocolPseudoHeader));

    //Size is translated into 32bit
    msg -> headerSize32 = sizeof(TransmissionControlProtocolHeader)/4;

    //Set the ports
    msg -> srcPort = bigEndian16(socket -> localPort);
    msg -> dstPort = bigEndian16(socket -> remotePort);

    //Set TCP related data
    msg -> acknowledgementNumber = bigEndian32( socket -> acknowledgementNumber );
    msg -> sequenceNumber = bigEndian32( socket -> sequenceNumber );
    msg -> reserved = 0;
    msg -> flags = flags;
    msg -> windowSize = 0xFFFF;
    msg -> urgentPtr = 0;

    //Through the options allow for the MSS to be set
    msg -> options = ((flags & (uint16_t)TCPFlag::SYN) != 0) ? 0xB4050402 : 0;

    //Increase the sequence number
    socket -> sequenceNumber += size;

    // Check if the data is not null
    if(data != nullptr)
    {
            //Copy the data into the buffer
            for(int i = 0; i < size; i++)
            buffer2[i] = data[i];
    }

    //Set the pseudo header
    phdr -> srcIP = socket -> localIP;
    phdr -> dstIP = socket -> remoteIP;
    phdr -> protocol = 0x0600;
    phdr -> totalLength = ((totalLength & 0x00FF) << 8) | ((totalLength & 0xFF00) >> 8);

    //Calculate the checksum
    msg -> checksum = 0;
    msg -> checksum = InternetProtocolHandler::Checksum((uint16_t*)buffer, lengthInclPHdr);


    //Send and then free the data
    Send(socket -> remoteIP, (uint8_t*)msg, totalLength);
    MemoryManager::kfree(buffer);
}

/**
 * @brief Connect to a remote host through the TCP protocol
 * @param ip The IP of the remote host
 * @param port The port of the remote host
 * @return The socket that is connected to the remote host, 0 if it failed
 */
TransmissionControlProtocolSocket* TransmissionControlProtocolHandler::Connect(InternetProtocolAddress ip, TransmissionControlProtocolPort port)
{
    //Create a new socket
    auto* socket = (TransmissionControlProtocolSocket*)MemoryManager::kmalloc(sizeof(TransmissionControlProtocolSocket));

    //If there is space for the socket
    if(socket != nullptr)
    {
        //Set the socket
        new (socket) TransmissionControlProtocolSocket(this);

        //Set local and remote addresses
        socket -> remotePort = port;
        socket -> remoteIP = ip;
        socket -> localPort = freePorts++;
        socket -> localIP = internetProtocolHandler -> GetInternetProtocolAddress();

        //Convert into big endian
        socket -> remotePort = ((socket -> remotePort & 0xFF00)>>8) | ((socket -> remotePort & 0x00FF) << 8);
        socket -> localPort = ((socket -> localPort & 0xFF00)>>8) | ((socket -> localPort & 0x00FF) << 8);

        //Set the socket into the socket array and then set its state
        sockets.push_back(socket);
        socket -> state = TCPSocketState::SYN_SENT;

        //Dummy sequence number
        socket -> sequenceNumber = 0xbeefcafe;

        //Send a sync packet
        sendTransmissionControlProtocolPacket(socket, nullptr,0, (uint16_t)TCPFlag::SYN);
    }

    return socket;
}


TransmissionControlProtocolSocket *TransmissionControlProtocolHandler::Connect(const string&) {

  //TODO NEW STRING PARSEING

  return nullptr;
}

/**
 * @brief Begin the disconnect process
 *
 * @param socket The socket to disconnect
 */
void TransmissionControlProtocolHandler::Disconnect(TransmissionControlProtocolSocket* socket)
{

    socket -> state = TCPSocketState::FIN_WAIT1;                            //Begin fin wait sequence
    sendTransmissionControlProtocolPacket(socket, nullptr,0, (uint16_t)TCPFlag::FIN + (uint16_t)TCPFlag::ACK);            //Send FIN|ACK packet
    socket -> sequenceNumber++;                             //Increase the sequence number
}

/**
 * @brief Begin listening on a port
 *
 * @param port The port to listen on
 * @return The socket that will handle the connection
 */
TransmissionControlProtocolSocket* TransmissionControlProtocolHandler::Listen(uint16_t port)
{
    //Create a new socket
    auto* socket = (TransmissionControlProtocolSocket*)MemoryManager::kmalloc(sizeof(TransmissionControlProtocolSocket));

    //If there is space for the socket
    if(socket != nullptr)
    {
        //Set the socket
        new (socket) TransmissionControlProtocolSocket(this);

        //Configure the socket
        socket -> state = TCPSocketState::LISTEN;
        socket -> localIP = internetProtocolHandler -> GetInternetProtocolAddress();
        socket -> localPort = ((port & 0xFF00)>>8) | ((port & 0x00FF) << 8);

        //Add the socket to the socket array
        sockets.push_back(socket);
    }

    //Return the socket
    return socket;
}


/**
 * @brief Bind a data handler to this socket
 *
 * @param socket The socket to bind the handler to
 * @param transmissionControlProtocolHandler The handler to bind
 */
void TransmissionControlProtocolHandler::Bind(TransmissionControlProtocolSocket* socket, TransmissionControlProtocolPayloadHandler* handler)
{
  socket->connect_event_handler(handler);
}


/// ___ EVENTS ___ ///

DataReceivedEvent::DataReceivedEvent(TransmissionControlProtocolSocket *socket, uint8_t *data, uint16_t size)
: Event(TransmissionControlProtocolPayloadHandlerEvents::DATA_RECEIVED)
{
    this -> socket = socket;
    this -> data = data;
    this -> size = size;
}

DataReceivedEvent::~DataReceivedEvent()
= default;

ConnectedEvent::ConnectedEvent(TransmissionControlProtocolSocket *socket)
: Event(TransmissionControlProtocolPayloadHandlerEvents::CONNECTED)
{
    this -> socket = socket;
}
ConnectedEvent::~ConnectedEvent()
= default;

DisconnectedEvent::DisconnectedEvent(TransmissionControlProtocolSocket *socket)
: Event(TransmissionControlProtocolPayloadHandlerEvents::DISCONNECTED)
{
    this -> socket = socket;
}
DisconnectedEvent::~DisconnectedEvent()
= default;