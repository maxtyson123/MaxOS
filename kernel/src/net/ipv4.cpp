/**
 * @file ipv4.cpp
 * @brief Implementation of IPv4 protocol handlers
 *
 * @date 22nd November 2022
 * @author Max Tyson
 */

#include <net/ipv4.h>

using namespace MaxOS;
using namespace MaxOS::common;
using namespace MaxOS::net;
using namespace MaxOS::memory;
using namespace MaxOS::drivers;
using namespace MaxOS::drivers::ethernet;



///__RESOLVER__///

IPV4AddressResolver::IPV4AddressResolver(InternetProtocolHandler *internetProtocolHandler)
{

    internetProtocolHandler -> RegisterIPV4AddressResolver(this);

}



IPV4AddressResolver::~IPV4AddressResolver() = default;

/**
 * @brief Resolves an IP address to a MAC address. (Default, returns broadcast address, override for use)
 *
 * @return The MAC address.
 */
MediaAccessControlAddress IPV4AddressResolver::Resolve(InternetProtocolAddress) {
    return 0xFFFFFFFFFFFF;
}

/**
 * @brief Stores an IP address to MAC address mapping. (Default, does nothing, override for use)
 *
 * @param internetProtocolAddress The IP address.
 * @param mediaAccessControlAddress The MAC address.
 */
void IPV4AddressResolver::Store(InternetProtocolAddress, MediaAccessControlAddress) {

}


///__Payload Handler__///
IPV4PayloadHandler::IPV4PayloadHandler(InternetProtocolHandler* internetProtocolHandler, uint8_t protocol) {

    // Store vars
    this -> internetProtocolHandler = internetProtocolHandler;
    this -> ipProtocol = protocol;

    //Register handler
    internetProtocolHandler ->connectIPV4PayloadHandler(this);

}

IPV4PayloadHandler::~IPV4PayloadHandler() = default;

/**
 * @brief Called when an IP packet is received. (Deafult, does nothing, overide for use)
 *
 * @param srcIP_BE The source IP address.
 * @param dstIP_BE The destination IP address.
 * @param internetprotocolPayload The payload of the IP packet.
 * @param size The size of the IP packet.
 * @return True if the packet was handled, false otherwise.
 */
bool IPV4PayloadHandler::handleInternetProtocolPayload(net::InternetProtocolAddress srcIP_BE,
                                                                   net::InternetProtocolAddress dstIP_BE,
                                                                   uint8_t * internetprotocolPayload,
                                                                   uint32_t size) {
    return false;
}

/**
 * @brief Sends an IP packet.
 *
 * @param destinationIP The destination IP address.
 * @param payloadData The payload of the IP packet.
 * @param size The size of the IP packet.
 */
void IPV4PayloadHandler::Send(InternetProtocolAddress destinationIP, uint8_t *payloadData, uint32_t size) {


    //Pass to backend
    internetProtocolHandler -> sendInternetProtocolPacket(destinationIP, ipProtocol, payloadData, size);

}

/**
 * @brief Construct a new Internet Protocol Handler object
 *
 * @param backend The backend Ethernet frame handler.
 * @param ownInternetProtocolAddress The IP address of this device.
 * @param defaultGatewayInternetProtocolAddress The IP address of the default gateway.
 * @param subnetMask The subnet mask.
 * @param errorMessages Where to write error messages.
 */
InternetProtocolHandler::InternetProtocolHandler(EthernetFrameHandler *backend, InternetProtocolAddress ownInternetProtocolAddress, InternetProtocolAddress defaultGatewayInternetProtocolAddress, SubnetMask subnetMask, OutputStream* errorMessages)
        : EthernetFramePayloadHandler(backend, 0x0800)
{
    //Store vars
    this -> ownInternetProtocolAddress = ownInternetProtocolAddress;
    this -> defaultGatewayInternetProtocolAddress = defaultGatewayInternetProtocolAddress;
    this -> subnetMask = subnetMask;
    this -> errorMessages = errorMessages;
}

InternetProtocolHandler::~InternetProtocolHandler() = default;

/**
 * @brief Called when an IP packet is received.
 *
 * @param ethernetframePayload The payload of the IP packet.
 * @param size The size of the IP packet.
 * @return True if the packet is to be sent back, false otherwise.
 *
 * @todo Set the identifier when sending packets back
 */
bool InternetProtocolHandler::handleEthernetframePayload(uint8_t* ethernetframePayload, uint32_t size){

    errorMessages ->write("IP: Handling packet\n");

    //Check if the size is big enough to contain an ethernet frame
    if(size < sizeof(IPV4Header))
        return false;

    //Convert to struct for easier use
    auto* ipMessage = (IPV4Header*)ethernetframePayload;
    bool sendBack = false;

    //Only handle if it is for this device
    if(ipMessage -> destinationIP == GetInternetProtocolAddress())
    {
        uint32_t length = ipMessage -> totalLength;                          //Get length of the message
        if(length > size)                                               //Check if the length is bigger than the size of the message
            length = size;                                              //If so, set length to size (this stops heartbleed attacks as it will not read past the end of the message, which the attacker could have filled with data)

        // Get the handler for the protocol
        Map<uint8_t, IPV4PayloadHandler*>::iterator handlerIterator = IPV4PayloadHandlers.find(ipMessage -> protocol);
        if(handlerIterator != IPV4PayloadHandlers.end()) {
            IPV4PayloadHandler* handler = handlerIterator -> second;
            if(handler != nullptr) {
                sendBack = handler -> handleInternetProtocolPayload(ipMessage -> sourceIP, ipMessage -> destinationIP, ethernetframePayload + sizeof(IPV4Header), length - sizeof(IPV4Header));
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
void InternetProtocolHandler::sendInternetProtocolPacket(uint32_t dstIP_BE, uint8_t protocol, const uint8_t *data, uint32_t size) {

    auto* buffer = (uint8_t*)MemoryManager::kmalloc(sizeof(IPV4Header) + size);                           //Allocate memory for the message
    auto *message = (IPV4Header*)buffer;                                                                            //Convert to struct for easier use

    message -> version = 4;                                                                                                                           //Set version
    message -> headerLength = sizeof(IPV4Header)/4;                                                                                     //Set header length
    message -> typeOfService = 0;                                                                                                                     //Set type of service (not private)

    message -> totalLength = size + sizeof(IPV4Header);                                                                                 //Set total length
    message -> totalLength = ((message -> totalLength & 0xFF00) >> 8)                                                                                 // Convert to big endian (Swap bytes)
                             | ((message -> totalLength & 0x00FF) << 8);                                                                              // Convert to big endian (Swap bytes)

    message -> identifier = 0x100;                                                                                                                    //Set identification TODO: do properly
    message -> flagsAndOffset = 0x0040;                                                                                                               //Set flags/offset, 0x40 because we are not fragmenting (TODO: doesnt work for packets bigger than 1500 bytes)

    message -> timeToLive = 0x40;                                                                                                                     //Set time to live
    message -> protocol = protocol;                                                                                                                   //Set protocol

    message -> destinationIP = dstIP_BE;                                                                                                                      //Set destination IP
    message -> sourceIP = GetInternetProtocolAddress();                                                                                                  //Set source IP

    message -> checksum = 0;                                                                                                                          //Set checksum to 0, init with 0 as checksum funct will also add this value
    message -> checksum = Checksum((uint16_t*)message, sizeof(IPV4Header));                                             //Calculate checksum

    //Copy data
    uint8_t* data_buffer = buffer + sizeof(IPV4Header);                                                                                  //Get pointer to the data
    for(uint32_t i = 0; i < size; i++)                                                                                                                            //Loop through data
      data_buffer[i] = data[i];                                                                                                                             //Copy data

    //Check if the destination is on the same subnet, The if condition determines if the destination device is on the same Local network as the source device . and if they are not on the same local network then we resolve the ip address of the gateway .
    InternetProtocolAddress route = dstIP_BE;                                                                                                                               //Set route to destination IP by default
    if((dstIP_BE & subnetMask) != (ownInternetProtocolAddress & subnetMask))                                                                                             //Check if the destination is on the same subnet
        route = defaultGatewayInternetProtocolAddress;                                                                                                                                   //If not, set route to gateway IP
                                                                                                                    //Print debug info
    uint32_t MAC = resolver ->Resolve(route);

    //Send message
    frameHandler -> sendEthernetFrame(MAC, this -> handledType, buffer, size + sizeof(IPV4Header));      //Send message
    MemoryManager::kfree(buffer);                                                                                                 //Free memory
}

/**
 * @brief Creates a checksum for the given data.
 *
 * @param data The data to create a checksum for.
 * @param lengthInBytes The length of the data in bytes.
 * @return The checksum.
 */
uint16_t InternetProtocolHandler::Checksum(const uint16_t *data, uint32_t lengthInBytes) {

    uint32_t temp = 0;                                                                             //Init sum

    for(uint32_t i = 0; i < lengthInBytes/2; i++)                                                       //Loop through data (/2 bc bytes)
        temp += ((data[i] & 0xFF00) >> 8) | ((data[i] & 0x00FF) << 8);                             //Add data to sum in big endian

    if(lengthInBytes % 2)                                                                          //If there is an odd number of bytes
        temp += ((uint16_t)((char*)data)[lengthInBytes-1]) << 8;                                   //Add the last byte to the sum

    while(temp & 0xFFFF0000)                                                                       //While there is a carry
        temp = (temp & 0xFFFF) + (temp >> 16);                                                     //Add the carry to the sum

    return ((~temp & 0xFF00) >> 8) | ((~temp & 0x00FF) << 8);
}

/**
 * @brief Registers an IP address resolver.
 *
 * @param resolver The resolver to register.
 */
void InternetProtocolHandler::RegisterIPV4AddressResolver(IPV4AddressResolver *resolver) {

    this -> resolver = resolver;

}

/**
 * @brief Creates an IP address from four digits.
 *
 * @param digit1 The first digit.
 * @param digit2 The second digit.
 * @param digit3 The third digit.
 * @param digit4 The fourth digit.
 *
 * @return The created IP address.
 */
InternetProtocolAddress InternetProtocolHandler::CreateInternetProtocolAddress(uint8_t digit1, uint8_t digit2, uint8_t digit3, uint8_t digit4) {
    InternetProtocolAddress result = digit4;
    result = (result << 8) | digit3;
    result = (result << 8) | digit2;
    result = (result << 8) | digit1;
    return result;
}

/**
 * @brief Parses a string representation of an IP address.
 *
 * @param address The string representation of the IP address.
 * @return The parsed IP address.
 */
InternetProtocolAddress InternetProtocolHandler::Parse(string address) {
    uint8_t digits[4];

    uint8_t currentDigit = 0;
    for(unsigned char & digit : digits)
        digit = 0;

    for(int i = 0; i < address.length(); i++) {
        if(address[i] == '.') {
            currentDigit++;
            continue;
        }

        digits[currentDigit] *= 10;
        digits[currentDigit] += address[i] - '0';
    }

    return CreateInternetProtocolAddress(digits[0], digits[1], digits[2], digits[3]);
}

/**
 * @brief Creates a subnet mask from four digits.
 *
 * @param digit1 The first digit.
 * @param digit2 The second digit.
 * @param digit3 The third digit.
 * @param digit4 The fourth digit.
 *
 * @return The created subnet mask.
 */
SubnetMask InternetProtocolHandler::CreateSubnetMask(uint8_t digit1, uint8_t digit2, uint8_t digit3, uint8_t digit4) {
    return (SubnetMask)CreateInternetProtocolAddress(digit1, digit2, digit3, digit4);
}

/**
 * @brief Gets the IP address of this device.
 *
 * @return The IP address.
 */
InternetProtocolAddress InternetProtocolHandler::GetInternetProtocolAddress() const {
    return ownInternetProtocolAddress;
}

/**
 * @brief Gets the MAC address of this device.
 *
 * @return The MAC address.
 */
MediaAccessControlAddress InternetProtocolHandler::GetMediaAccessControlAddress() {
    return frameHandler -> getMAC();
}

/**
 * @brief Connects an IP protocol payload handler.
 *
 * @param IPV4PayloadHandler The payload handler to connect.
 */
void InternetProtocolHandler::connectIPV4PayloadHandler( IPV4PayloadHandler *IPV4PayloadHandler) {
    IPV4PayloadHandlers.insert(IPV4PayloadHandler -> ipProtocol, IPV4PayloadHandler);
}
