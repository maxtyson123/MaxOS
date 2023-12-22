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



///__RESOLVER__///

InternetProtocolAddressResolver::InternetProtocolAddressResolver(InternetProtocolHandler *internetProtocolHandler)
{

    internetProtocolHandler -> RegisterInternetProtocolAddressResolver(this);

}



InternetProtocolAddressResolver::~InternetProtocolAddressResolver() {

}

MediaAccessControlAddress InternetProtocolAddressResolver::Resolve(InternetProtocolAddress address) {
    return 0xFFFFFFFFFFFF; // the broadcast address
}

void InternetProtocolAddressResolver::Store(InternetProtocolAddress internetProtocolAddress, MediaAccessControlAddress mediaAccessControlAddress) {

}


///__Payload Handler__///
InternetProtocolPayloadHandler::InternetProtocolPayloadHandler(InternetProtocolHandler* internetProtocolHandler, uint8_t protocol) {

    // Store vars
    this -> internetProtocolHandler = internetProtocolHandler;
    this -> ipProtocol = protocol;

    //Register handler
    internetProtocolHandler ->connectInternetProtocolPayloadHandler(this);

}

InternetProtocolPayloadHandler::~InternetProtocolPayloadHandler() {

}

/**
 * @brief Called when an IP packet is received. (Deafult, does nothing, overide for use)
 *
 * @param srcIP_BE The source IP address.
 * @param dstIP_BE The destination IP address.
 * @param internetprotocolPayload The payload of the IP packet.
 * @param size The size of the IP packet.
 * @return True if the packet was handled, false otherwise.
 */
bool InternetProtocolPayloadHandler::handleInternetProtocolPayload(maxOS::net::InternetProtocolAddress sourceIP,
                                                                   maxOS::net::InternetProtocolAddress destinationIP,
                                                                   uint8_t *payloadData,
                                                                   uint32_t size) {
    return false;
}

/**
 * @brief Sends an IP packet.
 *
 * @param dstIP_BE The destination IP address.
 * @param internetprotocolPayload The payload of the IP packet.
 * @param size The size of the IP packet.
 * @return True if the packet was sent, false otherwise.
 */
void InternetProtocolPayloadHandler::Send(InternetProtocolAddress destinationIP, uint8_t *payloadData, uint32_t size) {


    //Pass to backend
    internetProtocolHandler -> sendInternetProtocolPacket(destinationIP, ipProtocol, payloadData, size);

}





InternetProtocolHandler::InternetProtocolHandler(EthernetFrameHandler *backend, InternetProtocolAddress ownInternetProtocolAddress, InternetProtocolAddress defaultGatewayInternetProtocolAddress, SubnetMask subnetMask, OutputStream* errorMessages)
        : EthernetFramePayloadHandler(backend, 0x0800)
{
    //Store vars
    this -> ownInternetProtocolAddress = ownInternetProtocolAddress;
    this -> defaultGatewayInternetProtocolAddress = defaultGatewayInternetProtocolAddress;
    this -> subnetMask = subnetMask;
    this -> errorMessages = errorMessages;
}

InternetProtocolHandler::~InternetProtocolHandler() {

}

/**
 * @brief Called when an IP packet is received.
 *
 * @param etherframePayload The payload of the IP packet.
 * @param size The size of the IP packet.
 * @return True if the packet is to be sent back, false otherwise.
 */
bool InternetProtocolHandler::handleEthernetframePayload(uint8_t* ethernetframePayload, uint32_t size){

    errorMessages ->write("IP: Handling packet\n");

    //Check if the size is big enough to contain an ethernet frame
    if(size < sizeof(InternetProtocolV4Header))
        return false;

    //Convert to struct for easier use
    InternetProtocolV4Header* ipMessage = (InternetProtocolV4Header*)ethernetframePayload;
    bool sendBack = false;

    //Only handle if it is for this device
    if(ipMessage -> destinationIP == GetInternetProtocolAddress())
    {
        int length = ipMessage -> totalLength;                          //Get length of the message
        if(length > size)                                               //Check if the length is bigger than the size of the message
            length = size;                                              //If so, set length to size (this stops heartbleed attacks as it will not read past the end of the message, which the attacker could have filled with data)

        // Get the handler for the protocol
        Map<uint8_t, InternetProtocolPayloadHandler*>::iterator handlerIterator = internetProtocolPayloadHandlers.find(ipMessage -> protocol);
        if(handlerIterator != internetProtocolPayloadHandlers.end()) {
            InternetProtocolPayloadHandler* handler = handlerIterator -> second;
            if(handler != 0) {
                sendBack = handler -> handleInternetProtocolPayload(ipMessage -> sourceIP, ipMessage -> destinationIP, ethernetframePayload + sizeof(InternetProtocolV4Header), length - sizeof(InternetProtocolV4Header));
            }
        }


    }

    //If the data is to be sent back again
    if(sendBack){

        //Swap source and destination
        uint32_t temp = ipMessage -> destinationIP;                                                                                     //Store destination IP
        ipMessage -> destinationIP = ipMessage -> sourceIP;                                                                                //Set destination IP to source IP
        ipMessage -> sourceIP = temp;                                                                                              //Set source IP to destination IP

        ipMessage -> timeToLive = 0x40;                                                                                         //Reset TTL
        ipMessage -> checksum = Checksum((uint16_t*)ipMessage, 4 * ipMessage -> headerLength);                  //Reset checksum as the source and destination IP have changed so has the time to live and therefore there is a different header


        // TODO: Set the identifier
    }

    errorMessages ->write("IP: Handled packet\n");
    return sendBack;
}


/**
 * @brief Sends an IP packet.
 *
 * @param dstIP_BE The destination IP address.
 * @param protocol The protocol of the IP packet.
 * @param data The payload of the IP packet.
 * @param size The size of the IP packet.
 */
void InternetProtocolHandler::sendInternetProtocolPacket(uint32_t dstIP_BE, uint8_t protocol, uint8_t *data, uint32_t size) {

    uint8_t* buffer = (uint8_t*)MemoryManager::s_active_memory_manager-> malloc(sizeof(InternetProtocolV4Header) + size);                           //Allocate memory for the message
    InternetProtocolV4Header *message = (InternetProtocolV4Header*)buffer;                                                                            //Convert to struct for easier use

    message -> version = 4;                                                                                                                           //Set version
    message -> headerLength = sizeof(InternetProtocolV4Header)/4;                                                                                     //Set header length
    message -> typeOfService = 0;                                                                                                                     //Set type of service (not priv)

    message -> totalLength = size + sizeof(InternetProtocolV4Header);                                                                                 //Set total length
    message -> totalLength = ((message -> totalLength & 0xFF00) >> 8)                                                                                 // Convert to big endian (Swap bytes)
                             | ((message -> totalLength & 0x00FF) << 8);                                                                              // Convert to big endian (Swap bytes)

    message -> identifier = 0x100;                                                                                                                    //Set identification TODO: do properly
    message -> flagsAndOffset = 0x0040;                                                                                                               //Set flags/offset, 0x40 because we are not fragmenting (TODO: doesnt work for packets bigger than 1500 bytes)

    message -> timeToLive = 0x40;                                                                                                                     //Set time to live
    message -> protocol = protocol;                                                                                                                   //Set protocol

    message -> destinationIP = dstIP_BE;                                                                                                                      //Set destination IP
    message -> sourceIP = GetInternetProtocolAddress();                                                                                                  //Set source IP

    message -> checksum = 0;                                                                                                                          //Set checksum to 0, init with 0 as checksum funct will also add this value
    message -> checksum = Checksum((uint16_t*)message, sizeof(InternetProtocolV4Header));                                             //Calculate checksum

    //Copy data
    uint8_t* databuffer = buffer + sizeof(InternetProtocolV4Header);                                                                                  //Get pointer to the data
    for(int i = 0; i < size; i++)                                                                                                                            //Loop through data
        databuffer[i] = data[i];                                                                                                                             //Copy data

    //Check if the destination is on the same subnet, The if condition determines if the destination device is on the same Local network as the source device . and if they are not on the same local network then we resolve the ip address of the gateway .
    InternetProtocolAddress route = dstIP_BE;                                                                                                                               //Set route to destination IP by default
    if((dstIP_BE & subnetMask) != (ownInternetProtocolAddress & subnetMask))                                                                                             //Check if the destination is on the same subnet
        route = defaultGatewayInternetProtocolAddress;                                                                                                                                   //If not, set route to gateway IP
                                                                                                                    //Print debug info
    uint32_t MAC = resolver ->Resolve(route);

    //Send message
    frameHandler -> sendEthernetFrame(MAC, this -> handledType, buffer, size + sizeof(InternetProtocolV4Header));      //Send message
    MemoryManager::s_active_memory_manager->free(buffer);                                                                                                 //Free memory
}

/**
 * @brief Creates a checksum for the given data.
 *
 * @param data The data to create a checksum for.
 * @param lengthInBytes The length of the data in bytes.
 * @return The checksum.
 */
uint16_t InternetProtocolHandler::Checksum(uint16_t *data, uint32_t lengthInBytes) {

    uint32_t temp = 0;                                                                             //Init sum

    for(int i = 0; i < lengthInBytes/2; i++)                                                       //Loop through data (/2 bc bytes)
        temp += ((data[i] & 0xFF00) >> 8) | ((data[i] & 0x00FF) << 8);                             //Add data to sum in big endian

    if(lengthInBytes % 2)                                                                          //If there is an odd number of bytes
        temp += ((uint16_t)((string)data)[lengthInBytes-1]) << 8;                                   //Add the last byte to the sum

    while(temp & 0xFFFF0000)                                                                       //While there is a carry
        temp = (temp & 0xFFFF) + (temp >> 16);                                                     //Add the carry to the sum

    return ((~temp & 0xFF00) >> 8) | ((~temp & 0x00FF) << 8);
}

void InternetProtocolHandler::RegisterInternetProtocolAddressResolver(InternetProtocolAddressResolver *resolver) {

    this -> resolver = resolver;

}

InternetProtocolAddress InternetProtocolHandler::CreateInternetProtocolAddress(uint8_t digit1, uint8_t digit2, uint8_t digit3, uint8_t digit4) {
    InternetProtocolAddress result = digit4;
    result = (result << 8) | digit3;
    result = (result << 8) | digit2;
    result = (result << 8) | digit1;
    return result;
}

InternetProtocolAddress InternetProtocolHandler::Parse(string address) {
    uint8_t digits[4];

    uint8_t currentDigit = 0;
    for(int i = 0; i < 4; i++)
        digits[i] = 0;
    for(string i = (string)address; *i != '\0'; ++i)
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

SubnetMask InternetProtocolHandler::CreateSubnetMask(uint8_t digit1, uint8_t digit2, uint8_t digit3, uint8_t digit4) {
    return (SubnetMask)CreateInternetProtocolAddress(digit1, digit2, digit3, digit4);
}

InternetProtocolAddress InternetProtocolHandler::GetInternetProtocolAddress() {
    return ownInternetProtocolAddress;
}

MediaAccessControlAddress InternetProtocolHandler::GetMediaAccessControlAddress() {
    return frameHandler -> getMAC();
}

void InternetProtocolHandler::connectInternetProtocolPayloadHandler( InternetProtocolPayloadHandler *internetProtocolPayloadHandler) {
    internetProtocolPayloadHandlers.insert(internetProtocolPayloadHandler -> ipProtocol, internetProtocolPayloadHandler);
}
