//
// Created by 98max on 11/11/2022.
//

#include <net/arp.h>

using namespace maxOS;
using namespace maxOS::common;
using namespace maxOS::net;
using namespace maxOS::drivers;
using namespace maxOS::drivers::ethernet;



net::AddressResolutionProtocol::AddressResolutionProtocol(EthernetFrameHandler* ethernetFrameHandler, InternetProtocolHandler* internetProtocolHandler, OutputStream* errorMessages)
: EthernetFramePayloadHandler(ethernetFrameHandler, 0x0806),
  InternetProtocolAddressResolver(internetProtocolHandler)
{
    this->internetProtocolHandler = internetProtocolHandler;
    this->errorMessages = errorMessages;
}

net::AddressResolutionProtocol::~AddressResolutionProtocol() {

}

/**
 * @brief Called when an ARP packet is received.
 *
 * @param etherframePayload The payload of the ARP packet.
 * @param size The size of the ARP packet.
 * @return True if the device should send a response, false otherwise.
 */
bool AddressResolutionProtocol::handleEthernetFramePayload(uint8_t* etherframePayload, uint32_t size) {

    //Check if the size is correct
    if(size < sizeof(AddressResolutionProtocolMessage))
        return false;

    //Convert the payload to an ARP message
    AddressResolutionProtocolMessage* arpMessage = (AddressResolutionProtocolMessage*)etherframePayload;

    //Check if the message hardware type is Ethernet (BigEndian)
    if(arpMessage -> hardwareType == 0x100){

        if(arpMessage -> protocol == 0x0008             //Check if the protocol is IPv4 (BigEndian)
        && arpMessage -> hardwareAddressSize == 6
        && arpMessage -> protocolAddressSize == 4
        && arpMessage -> dstIP == internetProtocolHandler -> GetInternetProtocolAddress())
        {

            switch (arpMessage -> command) {
                //Request
                case 0x0100:
                    arpMessage -> command = 0x0200;                                                                         //Set the command to reply
                    arpMessage -> dstMAC = arpMessage -> srcMAC;                                                            //Set the destination MAC to the source MAC
                    arpMessage -> dstIP = arpMessage -> srcIP;                                                              //Set the destination IP to the source IP
                    arpMessage -> srcMAC = internetProtocolHandler -> GetMediaAccessControlAddress();                      //Set the source MAC to this MAC
                    arpMessage -> srcIP = internetProtocolHandler -> GetInternetProtocolAddress();                         //Set the source IP to this IP
                    return true;
                    break;

                //Response
                case 0x0200:
                    addressCache.insert((InternetProtocolAddress)arpMessage -> srcIP, (MediaAccessControlAddress)arpMessage -> srcMAC);     //Insert the MAC address into the cache
                    break;

                default:
                    break;

            }

        }

    }

    //By default, don't send anything back
    return false;


}


/**
 * @brief Request the MAC address of a given IP address.
 *
 * @param IP_BE The IP address in BigEndian.
 */
void AddressResolutionProtocol::RequestMACAddress(InternetProtocolAddress address) {

    //When a MAC adress is requested, instantiate a new ARP message block on the stack
    AddressResolutionProtocolMessage arpMessage;

    //Set the message's values
    arpMessage.hardwareType = 0x0100;                                                   //Ethernet, encoded in BigEndian
    arpMessage.protocol = 0x0008;                                                       //IPv4, encoded in BigEndian
    arpMessage.hardwareAddressSize = 6;                                                 //MAC address size
    arpMessage.protocolAddressSize = 4;                                                 //IPv4 address size
    arpMessage.command = 0x0100;                                                        //Request, encoded in BigEndian

    //Set the message's source and destination
    arpMessage.srcMAC = frameHandler -> getMAC();                                       //Set the source MAC address to the backend's MAC address
    arpMessage.srcIP = internetProtocolHandler -> GetInternetProtocolAddress();        //Set the source IP address to the backend's IP address
    arpMessage.dstMAC = 0xFFFFFFFFFFFF;                                                 //Set the destination MAC address to broadcast
    arpMessage.dstIP = address;                                                           //Set the destination IP address to the requested IP address

    //Send the message
    this -> Send(arpMessage.dstMAC, (uint8_t*)&arpMessage, sizeof(AddressResolutionProtocolMessage));


}


/**
 * @brief Get the MAC address from an IP via ARP.
 *
 * @param IP_BE The IP address to get the MAC address from.
 * @return The MAC address of the IP address.
 */
MediaAccessControlAddress AddressResolutionProtocol::Resolve(InternetProtocolAddress address) {

    volatile Map<InternetProtocolAddress, MediaAccessControlAddress>::iterator cacheIterator = addressCache.find(address); //Check if the MAC address is in the cache

    //If not, request it
    if(addressCache.end() == cacheIterator){
        RequestMACAddress(address);
    }

    //TODO: Add clock to wait

    //This isnt safe because the MAC address might not be in the cache yet or the machine may not be connected to the network (possible infinite loop) //TODO: TIMEOUT
    while (cacheIterator == addressCache.end()) {                         //Wait until the MAC address is found
        cacheIterator = addressCache.find(address);
    }

    //Return the MAC address

    return cacheIterator -> second;

}

void AddressResolutionProtocol::Store(InternetProtocolAddress internetProtocolAddress, MediaAccessControlAddress mediaAccessControlAddress) {
    addressCache.insert(internetProtocolAddress, mediaAccessControlAddress);
}
