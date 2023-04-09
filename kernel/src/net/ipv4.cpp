//
// Created by 98max on 22/11/2022.
//

#include <net/ipv4.h>

using namespace maxOS;
using namespace maxOS::common;
using namespace maxOS::net;
using namespace maxOS::memory;
using namespace maxOS::drivers;
using namespace maxOS::drivers::ethernet;


void printf(char*, bool=false);


///__RESOLVER__///

InternetProtocolAddressResolver::InternetProtocolAddressResolver(InternetProtocolProvider *handler)
{

    handler -> RegisterInternetProtocolAddressResolver(this);

}



InternetProtocolAddressResolver::~InternetProtocolAddressResolver() {

}

MediaAccessControlAddress InternetProtocolAddressResolver::Resolve(InternetProtocolAddress address) {
    return 0xFFFFFFFFFFFF; // the broadcast address
}

void InternetProtocolAddressResolver::Store(InternetProtocolAddress internetProtocolAddress, MediaAccessControlAddress mediaAccessControlAddress) {

}


///__Handler__///
InternetProtocolHandler::InternetProtocolHandler(InternetProtocolProvider *backend, uint8_t protocol) {

    // Store vars
    this -> backend = backend;
    this -> ip_protocol = protocol;

    //Register handler
    backend -> handlers[protocol] = this;

}

InternetProtocolHandler::~InternetProtocolHandler() {

    //Unregister handler
    if(backend -> handlers[ip_protocol] == this)            //Double check to make sure it is the same handler
        backend -> handlers[ip_protocol] = 0;               //Unregister handler

}

/**
 * @details Called when an IP packet is received. (Deafult, does nothing, overide for use)
 *
 * @param srcIP_BE The source IP address.
 * @param dstIP_BE The destination IP address.
 * @param internetprotocolPayload The payload of the IP packet.
 * @param size The size of the IP packet.
 * @return True if the packet was handled, false otherwise.
 */
bool InternetProtocolHandler::OnInternetProtocolReceived(uint32_t srcIP_BE, uint32_t dstIP_BE, uint8_t *internetprotocolPayload, uint32_t size) {
    return false;
}

/**
 * @details Sends an IP packet.
 *
 * @param dstIP_BE The destination IP address.
 * @param internetprotocolPayload The payload of the IP packet.
 * @param size The size of the IP packet.
 * @return True if the packet was sent, false otherwise.
 */
void InternetProtocolHandler::Send(uint32_t dstIP_BE, uint8_t *internetProtocolPayload, uint32_t size) {

    //Pass to backend
    backend -> Send(dstIP_BE, ip_protocol, internetProtocolPayload, size);

}



///__Provider__///


InternetProtocolProvider::InternetProtocolProvider(EtherFrameProvider *backend, InternetProtocolAddress ownInternetProtocolAddress, InternetProtocolAddress defaultGatewayInternetProtocolAddress, SubnetMask subnetMask)
        : EtherFrameHandler(backend, 0x0800)
{
    //Store vars
    this -> ownInternetProtocolAddress = ownInternetProtocolAddress;
    this -> defaultGatewayInternetProtocolAddress = defaultGatewayInternetProtocolAddress;
    this -> subnetMask = subnetMask;
    //Reset handlers
    for (int i = 0; i < 255; ++i) {
        handlers[i] = 0;
    }
}

InternetProtocolProvider::~InternetProtocolProvider() {

}

/**
 * @details Called when an IP packet is received.
 *
 * @param etherframePayload The payload of the IP packet.
 * @param size The size of the IP packet.
 * @return True if the packet is to be sent back, false otherwise.
 */
bool InternetProtocolProvider::OnEtherFrameReceived(uint8_t *etherframePayload, uint32_t size) {

    //Check if the size is big enough to contain an ethernet frame
    if(size < sizeof(InternetProtocolV4Message))
        return false;

    //Convert to struct for easier use
    InternetProtocolV4Message* ipMessage = (InternetProtocolV4Message*)etherframePayload;
    bool sendBack = false;

    //Only handle if it is for this device
    if(ipMessage -> dstIP == GetInternetProtocolAddress())
    {
        int length = ipMessage -> totalLength;                          //Get length of the message
        if(length > size)                                               //Check if the length is bigger than the size of the message
            length = size;                                              //If so, set length to size (this stops heartbleed attacks as it will not read past the end of the message, which the attacker could have filled with data)

        //Check if there is a handler for this frame type
        if(handlers[ipMessage -> protocol] != 0){

            sendBack = handlers[ipMessage -> protocol] -> OnInternetProtocolReceived(ipMessage -> srcIP,                                                     //Source IP
                                                                                     ipMessage -> dstIP,                                                     //Destination IP
                                                                                     etherframePayload + 4 * ipMessage -> headerLength,           //Payload is behind the header
                                                                                     length - 4*ipMessage -> headerLength                                       //Size of the payload
            );

        }

    }

    //If the data is to be sent back again
    if(sendBack){

        //Swap source and destination
        uint32_t temp = ipMessage -> dstIP;                                                                                     //Store destination IP
        ipMessage -> dstIP = ipMessage -> srcIP;                                                                                //Set destination IP to source IP
        ipMessage -> srcIP = temp;                                                                                              //Set source IP to destination IP

        ipMessage -> timeToLive = 0x40;                                                                                         //Reset TTL
        ipMessage -> checksum = Checksum((uint16_t*)ipMessage, 4 * ipMessage -> headerLength);                  //Reset checksum as the source and destination IP have changed so has the time to live and therefore there is a different header

    }

    return sendBack;
}


/**
 * @details Sends an IP packet.
 *
 * @param dstIP_BE The destination IP address.
 * @param protocol The protocol of the IP packet.
 * @param data The payload of the IP packet.
 * @param size The size of the IP packet.
 */
void InternetProtocolProvider::Send(uint32_t dstIP_BE, uint8_t protocol, uint8_t *data, uint32_t size) {

    uint8_t* buffer = (uint8_t*)MemoryManager::activeMemoryManager -> malloc(sizeof(InternetProtocolV4Message) + size);                           //Allocate memory for the message
    InternetProtocolV4Message *message = (InternetProtocolV4Message*)buffer;                                                                     //Convert to struct for easier use

    message -> version = 4;                                                                                                                                  //Set version
    message -> headerLength = sizeof(InternetProtocolV4Message)/4;                                                                                     //Set header length
    message -> tos = 0;                                                                                                                                      //Set type of service (not priv)

    message -> totalLength = size + sizeof(InternetProtocolV4Message);                                                                                 //Set total length
    message -> totalLength = ((message -> totalLength & 0xFF00) >> 8)                                                                                        // Convert to big endian (Swap bytes)
                             | ((message -> totalLength & 0x00FF) << 8);                                                                                       // Convert to big endian (Swap bytes)

    message -> ident = 0x100;                                                                                                                       //Set identification
    message -> flagsAndOffset = 0x0040;                                                                                                                      //Set flags/offset, 0x40 because we are not fragmenting

    message -> timeToLive = 0x40;                                                                                                                            //Set time to live
    message -> protocol = protocol;                                                                                                                          //Set protocol

    message -> dstIP = dstIP_BE;                                                                                                                             //Set destination IP
    message -> srcIP = GetInternetProtocolAddress();                                                                                                            //Set source IP

    message -> checksum = 0;                                                                                                                                 //Set checksum to 0, init with 0 as checksum funct will also add this value
    message -> checksum = Checksum((uint16_t*)message, sizeof(InternetProtocolV4Message));                                             //Calculate checksum

    //Copy data
    uint8_t* databuffer = buffer + sizeof(InternetProtocolV4Message);                                                                                  //Get pointer to the data
    for(int i = 0; i < size; i++)                                                                                                                            //Loop through data
        databuffer[i] = data[i];                                                                                                                             //Copy data

    //Check if the destination is on the same subnet, The if condition determines if the destination device is on the same Local network as the source device . and if they are not on the same local network then we resolve the ip address of the gateway .
    uint32_t route = dstIP_BE;                                                                                                                               //Set route to destination IP by default
    if((dstIP_BE & subnetMask) != (message->srcIP & subnetMask))                                                                                             //Check if the destination is on the same subnet
        route = defaultGatewayInternetProtocolAddress;                                                                                                                                   //If not, set route to gateway IP

    printf("\nRequesting ARP\n");                                                                                                                            //Print debug info
    uint32_t MAC = resolver ->Resolve(route);

    //Send message
    printf("\nSending IP packet\n");
    backend -> Send(MAC, this -> etherType_BE, buffer, size + sizeof(InternetProtocolV4Message));      //Send message
    MemoryManager::activeMemoryManager->free(buffer);                                                                                                 //Free memory
    printf("\nSent IP packet\n");
}

/**
 * @details Creates a checksum for the given data.
 *
 * @param data The data to create a checksum for.
 * @param lengthInBytes The length of the data in bytes.
 * @return The checksum.
 */
uint16_t InternetProtocolProvider::Checksum(uint16_t *data, uint32_t lengthInBytes) {

    uint32_t temp = 0;                                                                             //Init sum

    for(int i = 0; i < lengthInBytes/2; i++)                                                       //Loop through data (/2 bc bytes)
        temp += ((data[i] & 0xFF00) >> 8) | ((data[i] & 0x00FF) << 8);                             //Add data to sum in big endian

    if(lengthInBytes % 2)                                                                          //If there is an odd number of bytes
        temp += ((uint16_t)((char*)data)[lengthInBytes-1]) << 8;                                   //Add the last byte to the sum

    while(temp & 0xFFFF0000)                                                                       //While there is a carry
        temp = (temp & 0xFFFF) + (temp >> 16);                                                     //Add the carry to the sum

    return ((~temp & 0xFF00) >> 8) | ((~temp & 0x00FF) << 8);
}

void InternetProtocolProvider::RegisterInternetProtocolAddressResolver(InternetProtocolAddressResolver *resolver) {

    this -> resolver = resolver;

}

InternetProtocolAddress InternetProtocolProvider::CreateInternetProtocolAddress(uint8_t digit1, uint8_t digit2, uint8_t digit3, uint8_t digit4) {
    InternetProtocolAddress result = digit4;
    result = (result << 8) | digit3;
    result = (result << 8) | digit2;
    result = (result << 8) | digit1;
    return result;
}

InternetProtocolAddress InternetProtocolProvider::Parse(string address) {
    uint8_t digits[4];

    uint8_t currentDigit = 0;
    for(int i = 0; i < 4; i++)
        digits[i] = 0;
    for(char* i = (char*)address; *i != '\0'; ++i)
    {
        if('0' <= *i && *i <= '9')
        {
            digits[currentDigit] = digits[currentDigit] * 10 + *i - '0';
        }
        else
        {
            if(currentDigit >= 3)
                break;
            currentDigit++;
        }

    }

    return CreateInternetProtocolAddress(digits[0], digits[1], digits[2], digits[3]);
}

SubnetMask InternetProtocolProvider::CreateSubnetMask(uint8_t digit1, uint8_t digit2, uint8_t digit3, uint8_t digit4) {
    return (SubnetMask)CreateInternetProtocolAddress(digit1, digit2, digit3, digit4);
}

InternetProtocolAddress InternetProtocolProvider::GetInternetProtocolAddress() {
    return ownInternetProtocolAddress;
}

MediaAccessControlAddress InternetProtocolProvider::GetMediaAccessControlAddress() {
    return backend -> GetMACAddress();
}

