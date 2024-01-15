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
{
}

TransmissionControlProtocolPayloadHandler::~TransmissionControlProtocolPayloadHandler()
{
}

void TransmissionControlProtocolPayloadHandler::handleTransmissionControlProtocolPayload(TransmissionControlProtocolSocket*, uint8_t*, uint16_t) {

}

void TransmissionControlProtocolPayloadHandler::Connected(TransmissionControlProtocolSocket*) {

}

void TransmissionControlProtocolPayloadHandler::Disconnected(TransmissionControlProtocolSocket*) {

}

Event<TransmissionControlProtocolPayloadHandlerEvents>* TransmissionControlProtocolPayloadHandler::on_event(Event<TransmissionControlProtocolPayloadHandlerEvents> *event) {

    switch (event -> type)
    {
        case CONNECTED:
            Connected(((ConnectedEvent*)event) -> socket);
            break;
        case DISCONNECTED:
            Disconnected(((DisconnectedEvent*)event) -> socket);
            break;
        case DATA_RECEIVED:
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
    state = CLOSED;
}

TransmissionControlProtocolSocket::~TransmissionControlProtocolSocket()
{
}
/**
 * @brief Handle the TCP message (socket end)
 *
 * @param data The datah
 * @param size The size of the data
 * @return True if the connection is to be terminated after hadnling or false if not
 */
bool TransmissionControlProtocolSocket::handleTransmissionControlProtocolPayload(uint8_t* data, uint16_t size)
{
    DataReceivedEvent* event = new DataReceivedEvent(this, data, size);
    raise_event(event);
    MemoryManager::s_active_memory_manager->free(event);
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
    while(state != ESTABLISHED);

    //Pass the data to the backend
    transmissionControlProtocolHandler -> sendTransmissionControlProtocolPacket(this, data, size, PSH|ACK);
}

/**
 * @brief Disconnect the socket
 */
void TransmissionControlProtocolSocket::Disconnect()
{
    transmissionControlProtocolHandler -> Disconnect(this);
}

void TransmissionControlProtocolSocket::Disconnected() {
    DisconnectedEvent* event = new DisconnectedEvent(this);
    raise_event(event);
    MemoryManager::s_active_memory_manager->free(event);

}

void TransmissionControlProtocolSocket::Connected() {
    ConnectedEvent* event = new ConnectedEvent(this);
    raise_event(event);
    MemoryManager::s_active_memory_manager->free(event);

}

///__Handler__///

TransmissionControlProtocolPort TransmissionControlProtocolHandler::freePorts = 0x8000;

TransmissionControlProtocolHandler::TransmissionControlProtocolHandler(
        MaxOS::net::InternetProtocolHandler *internetProtocolHandler, OutputStream *errorMessages)
: InternetProtocolPayloadHandler(internetProtocolHandler, 0x06)
{
    this -> errorMessages = errorMessages;
    
}

TransmissionControlProtocolHandler::~TransmissionControlProtocolHandler()
{
}

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
    TransmissionControlProtocolHeader* msg = (TransmissionControlProtocolHeader*)payloadData;

    //Get the connection values (convert to host endian)
    uint16_t localPort = bigEndian16(msg -> dstPort);
    uint16_t remotePort = bigEndian16(msg -> srcPort);

    //Create the socket
    TransmissionControlProtocolSocket* socket = 0;

    for(Vector<TransmissionControlProtocolSocket*>::iterator currentSocket = sockets.begin(); currentSocket != sockets.end(); currentSocket++)
    {
        if( (*currentSocket) -> localPort == localPort                               //Check if the local port is the same as the destination port
        &&  (*currentSocket) -> localIP == destinationIP                                  //Check if the local IP is the same as the destination IP
        &&  (*currentSocket) -> state == LISTEN                                           //Check if the socket is in the LISTEN state
        && (((msg -> flags) & (SYN | ACK)) == SYN))                                       //Check if the SYN flag is set (allow for acknoweldgement)
        {
            socket = *currentSocket;
        }
        else if( (*currentSocket) -> localPort == localPort                          //Check if the local port is the same as the destination port
             &&  (*currentSocket) -> localIP == destinationIP                             //Check if the local IP is the same as the destination IP
             &&  (*currentSocket) -> remotePort == remotePort                         //Check if the remote port is the same as the source port
             &&  (*currentSocket) -> remoteIP == destinationIP)                           //Check if the remote IP is the same as the source IP
        {
            socket = *currentSocket;
        }
    }


    bool reset = false;

    //Check if the socket is found and if the socket wants to reset
    if(socket != 0 && msg -> flags & RST)
    {
        socket -> state = CLOSED;
        socket -> Disconnected();
    }

    //Check if the socket is found and if the socket is not closed
    if(socket != 0 && socket -> state != CLOSED)
    {
        switch((msg -> flags) & (SYN | ACK | FIN))
        {
            /*
             * Example for explanation:
             * socket -> state = SYN_RECEIVED;                                                  //The state of the socket, e.g. recieved, or established. This is used to know how to handle the socket
             * socket -> remotePort = msg -> srcPort;                                           //The remote port, e.g. the port of the server
             * socket -> remoteIP = srcIP_BE;                                                   //The remote IP, e.g. the IP of the server
             * socket -> acknowledgementNumber = bigEndian32( msg -> sequenceNumber ) + 1;      //The acknowledgement number, the number used to keep track of what has been received, this is just incremented by 1 each time
             * socket -> sequenceNumber = 0xbeefcafe;                                           //The sequence number, the number of the next set that is to be sent but in this case sequence isnt enabled so just set it to anything
             * Send(socket, 0,0, SYN|ACK);                                                      //The response command, genneraly has to have the acknoledgement flag set
             * socket -> sequenceNumber++;                                                      //Increment the sequence number
             *
             */

            case SYN:
                if(socket -> state == LISTEN)
                {
                    socket -> state = SYN_RECEIVED;
                    socket -> remotePort = msg -> srcPort;
                    socket -> remoteIP = sourceIP;
                    socket -> acknowledgementNumber = bigEndian32( msg -> sequenceNumber ) + 1;
                    socket -> sequenceNumber = 0xbeefcafe;
                    sendTransmissionControlProtocolPacket(socket, 0,0, SYN|ACK);
                    socket -> sequenceNumber++;
                }
                else
                    reset = true;
                break;


            case SYN | ACK:
                if(socket -> state == SYN_SENT)
                {
                    socket -> state = ESTABLISHED;
                    socket -> acknowledgementNumber = bigEndian32( msg -> sequenceNumber ) + 1;
                    socket -> sequenceNumber++;
                    sendTransmissionControlProtocolPacket(socket, 0,0, ACK);
                }
                else
                    reset = true;
                break;


            case SYN | FIN:
            case SYN | FIN | ACK:
                reset = true;
                break;


            case FIN:
            case FIN|ACK:
                if(socket -> state == ESTABLISHED)
                {
                    socket -> state = CLOSE_WAIT;
                    socket -> acknowledgementNumber++;
                    sendTransmissionControlProtocolPacket(socket, 0,0, ACK);
                    sendTransmissionControlProtocolPacket(socket, 0,0, FIN|ACK);
                    socket -> Disconnected();
                }
                else if(socket -> state == CLOSE_WAIT)
                {
                    socket -> state = CLOSED;
                }
                else if(socket -> state == FIN_WAIT1 || socket -> state == FIN_WAIT2)
                {
                    socket -> state = CLOSED;
                    socket -> acknowledgementNumber++;
                    sendTransmissionControlProtocolPacket(socket, 0,0, ACK);
                    socket -> Disconnected();
                }
                else
                    reset = true;
                break;


            case ACK:
                if(socket -> state == SYN_RECEIVED)
                {
                    socket -> state = ESTABLISHED;
                    socket -> Connected();
                    return false;
                }
                else if(socket -> state == FIN_WAIT1)
                {
                    socket -> state = FIN_WAIT2;
                    return false;
                }
                else if(socket -> state == CLOSE_WAIT)
                {
                    socket -> state = CLOSED;
                    break;
                }

                if(msg -> flags == ACK)
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
                        int x = 0;                                                                      //The number of bytes to send back
                        for(uint32_t i = msg -> headerSize32*4; i < size; i++)                          //Loop through the data
                            if(payloadData[i] != 0)                                                     //Check if the data is not 0
                                x = i;                                                                  //Set the number of bytes to send back to the current index
                        socket -> acknowledgementNumber += x - msg -> headerSize32*4 + 1;               //Increment the acknowledgement number by the number of bytes to send back
                        sendTransmissionControlProtocolPacket(socket, 0,0, ACK);                                          //Send the acknowledgement
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
        if(socket != 0)                                                             //If the socket exists then send a reset flag
        {
            sendTransmissionControlProtocolPacket(socket, 0,0, RST);
        }
        else                                                                        //If it doesnt exist then create a new socket and send a reset flag
        {
            TransmissionControlProtocolSocket socket(this);                     //Create a new socket
            socket.remotePort = msg -> srcPort;                                         //Set the remote port
            socket.remoteIP = sourceIP;                                                 //Set the remote IP
            socket.localPort = msg -> dstPort;                                                  //Set the local port
            socket.localIP = destinationIP;                                                     //Set the local IP
            socket.sequenceNumber = bigEndian32(msg -> acknowledgementNumber);              //Set the sequence number
            socket.acknowledgementNumber = bigEndian32(msg -> sequenceNumber) + 1;          //Set the acknowledgement number
            sendTransmissionControlProtocolPacket(&socket, 0,0, RST);          //Send the reset flag
        }
    }


    errorMessages ->write("TCP: Handled packet\n");

    if(socket != 0 && socket -> state == CLOSED)                                        //If the socket is closed then remove it from the list
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
void TransmissionControlProtocolHandler::sendTransmissionControlProtocolPacket(TransmissionControlProtocolSocket* socket, uint8_t* data, uint16_t size, uint16_t flags)
{
    //Get the total size of the packet and the packet with the pseudo header
    uint16_t totalLength = size + sizeof(TransmissionControlProtocolHeader);
    uint16_t lengthInclPHdr = totalLength + sizeof(TransmissionControlProtocolPseudoHeader);

    //Create a buffer for the packet
    uint8_t* buffer = (uint8_t*)MemoryManager::s_active_memory_manager-> malloc(lengthInclPHdr);
    uint8_t* buffer2 = buffer + sizeof(TransmissionControlProtocolHeader) + sizeof(TransmissionControlProtocolPseudoHeader);

    //Create the headers
    TransmissionControlProtocolPseudoHeader* phdr = (TransmissionControlProtocolPseudoHeader*)buffer;
    TransmissionControlProtocolHeader* msg = (TransmissionControlProtocolHeader*)(buffer + sizeof(TransmissionControlProtocolPseudoHeader));

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
    msg -> options = ((flags & SYN) != 0) ? 0xB4050402 : 0;

    //Increase the sequence number
    socket -> sequenceNumber += size;

    // Check if the data is not null
    if(data != 0)
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
    MemoryManager::s_active_memory_manager-> free(buffer);
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
    TransmissionControlProtocolSocket* socket = (TransmissionControlProtocolSocket*)MemoryManager::s_active_memory_manager-> malloc(sizeof(TransmissionControlProtocolSocket));

    //If there is space for the socket
    if(socket != 0)
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
        socket -> state = SYN_SENT;

        //Dummy sequence number
        socket -> sequenceNumber = 0xbeefcafe;

        //Send a sync packet
        sendTransmissionControlProtocolPacket(socket, 0,0, SYN);
    }

    return socket;
}


TransmissionControlProtocolSocket *TransmissionControlProtocolHandler::Connect(string) {

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

    socket -> state = FIN_WAIT1;                            //Begin fin wait sequence
    sendTransmissionControlProtocolPacket(socket, 0,0, FIN + ACK);            //Send FIN|ACK packet
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
    TransmissionControlProtocolSocket* socket = (TransmissionControlProtocolSocket*)MemoryManager::s_active_memory_manager-> malloc(sizeof(TransmissionControlProtocolSocket));

    //If there is space for the socket
    if(socket != 0)
    {
        //Set the socket
        new (socket) TransmissionControlProtocolSocket(this);

        //Configure the socket
        socket -> state = LISTEN;
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
: Event(DATA_RECEIVED)
{
    this -> socket = socket;
    this -> data = data;
    this -> size = size;
}

DataReceivedEvent::~DataReceivedEvent()
{
}

ConnectedEvent::ConnectedEvent(TransmissionControlProtocolSocket *socket)
: Event(CONNECTED)
{
    this -> socket = socket;
}
ConnectedEvent::~ConnectedEvent()
{
}

DisconnectedEvent::DisconnectedEvent(TransmissionControlProtocolSocket *socket)
: Event(DISCONNECTED)
{
    this -> socket = socket;
}
DisconnectedEvent::~DisconnectedEvent()
{
}