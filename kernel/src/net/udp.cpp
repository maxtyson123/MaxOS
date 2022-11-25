#include <net/udp.h>

using namespace maxOS;
using namespace maxOS::net;
using namespace maxOS::common;
using namespace maxOS::system;

///__Handler__

UserDatagramProtocolHandler::UserDatagramProtocolHandler() {

}

UserDatagramProtocolHandler::~UserDatagramProtocolHandler() {

}

void UserDatagramProtocolHandler::HandleUserDatagramProtocolMessage(UserDatagramProtocolSocket *socket, uint8_t *data, uint16_t size) {

}

///__Socket__


UserDatagramProtocolSocket::UserDatagramProtocolSocket(UserDatagramProtocolProvider *backend) {

    //Set the instance variables
    this -> backend = backend;
    handler = 0;
    listening = false;

}

UserDatagramProtocolSocket::~UserDatagramProtocolSocket() {

}

void UserDatagramProtocolSocket::HandleUserDatagramProtocolMessage(uint8_t *data, uint16_t size) {

    if(handler != 0) {
        handler -> HandleUserDatagramProtocolMessage(this, data, size);
    }

}

void UserDatagramProtocolSocket::Send(uint8_t *data, uint16_t size) {

    backend -> Send(this, data, size);

}

void UserDatagramProtocolSocket::Disconnect() {

    backend ->Disconnect(this);

}

///__Provider__
UserDatagramProtocolProvider::UserDatagramProtocolProvider(InternetProtocolProvider *backend)
: InternetProtocolHandler(backend, 0x11)    //0x11 is the UDP protocol number
{

    //Clear the sockets
    for (int i = 0; i < 65535; ++i) {
        sockets[i] = 0;
    }

    //Set the instance variables
    numSockets = 0;
    freePort = 1024;                    //1024 is the first port that can be used as the rest are reserved
    
}

UserDatagramProtocolProvider::~UserDatagramProtocolProvider() {

}

bool UserDatagramProtocolProvider::OnInternetProtocolReceived(uint32_t srcIP_BE, uint32_t dstIP_BE, uint8_t *internetprotocolPayload,uint32_t size) {

    //Check the size
    if(size < sizeof(UserDatagramProtocolHeader)) {
        return false;
    }

    //Get the header
    UserDatagramProtocolHeader* header = (UserDatagramProtocolHeader*)internetprotocolPayload;

    //Set the local and remote ports
    uint16_t localPort = header -> destinationPort;
    uint16_t remotePort = header -> sourcePort;

    UserDatagramProtocolSocket* socket = 0;                    //The socket that will be used
    for (int i = 0; i < numSockets && socket == 0; ++i)        //Loop through the sockets that are in use while the socket is not found
    {

        if(sockets[i]->localPort == localPort                  //If the local port (header dst, our port) is the same as the local port of the socket
        && sockets[i]->localIP == dstIP_BE                     //If the local IP (packet dst, our IP) is the same as the local IP of the socket
        && sockets[i]->listening)                              //If the socket is listening
        {

            socket = sockets[i];                               //Set the socket to the socket that is being checked
            socket->listening = false;                         //Set the socket to not listening
            socket->remotePort = remotePort;                   //Set the remote port of the socket to the remote port of the packet
            socket->remoteIP = srcIP_BE;                       //Set the remote IP of the socket to the remote IP of the packet

        }else if(sockets[i]->localPort == localPort            //If the local port (header dst, our port) is the same as the local port of the socket
              &&  sockets[i]->localIP == dstIP_BE              //If the local IP (packet dst, our IP) is the same as the local IP of the socket
              &&  sockets[i]->remotePort == remotePort         //If the remote port (header src, their port) is the same as the remote port of the socket
              &&  sockets[i]->remoteIP == srcIP_BE)            //If the remote IP (packet src, their IP) is the same as the remote IP of the socket
        {
            socket = sockets[i];                               //Set the socket to the current socket
        }

    }

    if(socket != 0) {                                          //If the socket is not null then pass the data to the socket
        socket->HandleUserDatagramProtocolMessage(internetprotocolPayload + sizeof(UserDatagramProtocolHeader), size - sizeof(UserDatagramProtocolHeader));
    }

    //UDP doesn't send back packets, so always return false
    return false;

}

UserDatagramProtocolSocket *UserDatagramProtocolProvider::Connect(uint32_t ip, uint16_t port) {


    UserDatagramProtocolSocket* socket = (UserDatagramProtocolSocket*)MemoryManager::activeMemoryManager -> malloc(sizeof(UserDatagramProtocolSocket));   //Allocate memory for the socket

    if(socket != 0) //If the socket was created
    {
        new (socket) UserDatagramProtocolSocket(this);    //Create the socket

        //Configure the socket
        socket -> remotePort = port;                      //Port to that application wants to connect to
        socket -> remoteIP = ip;                          //IP to that application wants to connect to
        socket -> localPort = freePort++;                 //Port that we will use to connect to the remote application  (note, local port doesnt have to be the same as remote)
        socket -> localIP = backend -> GetIPAddress();    //IP that we will use to connect to the remote application

        //Convet the ports to big endian
        socket -> remotePort = ((socket -> remotePort & 0xFF00)>>8) | ((socket -> remotePort & 0x00FF) << 8);
        socket -> localPort = ((socket -> localPort & 0xFF00)>>8) | ((socket -> localPort & 0x00FF) << 8);


        sockets[numSockets++] = socket;                   //Add the socket to the list of sockets
    }

    return socket;                                        //Return the socket
}

UserDatagramProtocolSocket *UserDatagramProtocolProvider::Listen(uint16_t port) {
    return nullptr;
}

void UserDatagramProtocolProvider::Disconnect(UserDatagramProtocolSocket *socket) {

    for(uint16_t i = 0; i < numSockets && socket == 0; i++){                    //Loop through the sockets that are in use while the socket is not found

        if(sockets[i] == socket)                                                //If the socket is the same as the socket that is being checked
        {
            sockets[i] = sockets[--numSockets];                                 //Replace the socket with the last socket in the list
            MemoryManager::activeMemoryManager -> free(socket);          //Free the socket
            break;                                                              //Break out of the loop
        }


    }

}

void UserDatagramProtocolProvider::Send(UserDatagramProtocolSocket *socket, uint8_t *data, uint16_t size) {

    uint16_t totalSize = sizeof(UserDatagramProtocolHeader) + size;                                 //Get the total size of the packet
    uint8_t* buffer = (uint8_t*)MemoryManager::activeMemoryManager->malloc(totalSize);          //Allocate memory for the packet
    uint8_t* buffer2 = buffer + sizeof(UserDatagramProtocolHeader);                                 //Get the buffer that will be used to store the data

    UserDatagramProtocolHeader* header = (UserDatagramProtocolHeader*)buffer;                       //Create the header of the packet

    //Set the header
    header -> sourcePort = socket -> localPort;                                                    //Set the source port to the local port of the socket    (this is the port that the packet will be sent from)
    header -> destinationPort = socket -> remotePort;                                              //Set the destination port to the remote port of the socket (this is the port that the packet will be sent to)
    header -> length = ((totalSize & 0x00FF) << 8) | ((totalSize & 0xFF00) >> 8);                  //Set the length of the packet

    //Copy the data to the buffer
    for (int i = 0; i < size; ++i) {                                                               //Loop through the data
        buffer2[i] = data[i];                                                                      //Copy the data to the buffer
    }

    //Set the checksum
    header -> checksum = 0;                                                                        //Set the checksum to 0, this is becuase UDP doesnt have to have a checksum

    //Send the packet
    InternetProtocolHandler::Send(socket->remoteIP, buffer, totalSize);

    //Free the buffer
    MemoryManager::activeMemoryManager->free(buffer);

}

void UserDatagramProtocolProvider::Bind(UserDatagramProtocolSocket *socket, UserDatagramProtocolHandler *handler) {

}
