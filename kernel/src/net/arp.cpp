//
// Created by 98max on 11/11/2022.
//

#include <net/arp.h>

using namespace maxOS;
using namespace maxOS::common;
using namespace maxOS::net;
using namespace maxOS::drivers;



net::AddressResolutionProtocol::AddressResolutionProtocol(net::EtherFrameProvider* backend)
: EtherFrameHandler(backend, 0x0806)
{
    numCacheEntries = 0;
}

net::AddressResolutionProtocol::~AddressResolutionProtocol() {

}

/**
 * Called when an ARP packet is received.
 * @param etherframePayload The payload of the ARP packet.
 * @param size The size of the ARP packet.
 * @return True if the device should send a response, false otherwise.
 */
bool AddressResolutionProtocol::OnEtherFrameReceived(common::uint8_t* etherframePayload, common::uint32_t size) {

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
        && arpMessage -> dstIP == backend -> GetIPAddress())
        {

            switch (arpMessage -> command) {
                //Request
                case 0x0100:
                    arpMessage -> command = 0x0200;                                         //Set the command to reply
                    arpMessage -> dstMAC = arpMessage -> srcMAC;                            //Set the destination MAC to the source MAC
                    arpMessage -> dstIP = arpMessage -> srcIP;                              //Set the destination IP to the source IP
                    arpMessage -> srcMAC = backend -> GetMACAddress();                      //Set the source MAC to this MAC
                    arpMessage -> srcIP = backend -> GetIPAddress();                        //Set the source IP to this IP
                    return true;
                    break;

                //Response
                case 0x0200:
                    if(numCacheEntries < 128){                                              //Check if the cache is full
                        cacheIPAddress[numCacheEntries] = arpMessage -> srcIP;              //Save the IP address
                        cacheMACAddress[numCacheEntries] = arpMessage -> srcMAC;            //Save the MAC address
                        numCacheEntries++;                                                  //Increase the number of entries
                    }
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
 * Request the MAC address of a given IP address.
 * @param IP_BE The IP address in BigEndian.
 */
void AddressResolutionProtocol::RequestMACAddress(common::uint32_t IP_BE) {

    //When a MAC adress is requested, instantiate a new ARP message block on the stack
    AddressResolutionProtocolMessage arpMessage;

    //Set the message's values
    arpMessage.hardwareType = 0x0100;                   //Ethernet, encoded in BigEndian
    arpMessage.protocol = 0x0008;                       //IPv4, encoded in BigEndian
    arpMessage.hardwareAddressSize = 6;                 //MAC address size
    arpMessage.protocolAddressSize = 4;                 //IPv4 address size
    arpMessage.command = 0x0100;                        //Request, encoded in BigEndian

    //Set the message's source and destination
    arpMessage.srcMAC = backend -> GetMACAddress();     //Set the source MAC address to the backend's MAC address
    arpMessage.srcIP = backend -> GetIPAddress();       //Set the source IP address to the backend's IP address
    arpMessage.dstMAC = 0xFFFFFFFFFFFF;                 //Set the destination MAC address to broadcast
    arpMessage.dstIP = IP_BE;                           //Set the destination IP address to the requested IP address

    //Send the message
    this -> Send(arpMessage.dstMAC, (uint8_t*)&arpMessage, sizeof(AddressResolutionProtocolMessage));


}

/**
 * Get the MAC address from the cache.
 * @param IP_BE The IP address to search for.
 * @return The MAC address if found, broadcast address otherwise.
 */
common::uint64_t AddressResolutionProtocol::GetMACFromCache(common::uint32_t IP_BE) {

    //Search for IP in cache
    for (int i = 0; i < numCacheEntries; ++i) {             //Iterate through cache
        if(cacheIPAddress[i] == IP_BE){                     //Compare IP to cache
            return cacheMACAddress[i];                      //Return MAC if found
        }
    }

    //If not found, return broadcast address
    return 0xFFFFFFFFFFFF;

}


/**
 * Get the MAC address from an IP via ARP.
 * @param IP_BE The IP address to get the MAC address from.
 * @return The MAC address of the IP address.
 */
common::uint64_t AddressResolutionProtocol::Resolve(common::uint32_t IP_BE) {


    //This function will return the MAC address of the IP address given as parameter

    //First, check if the IP address is in the cache
    uint64_t MAC = GetMACFromCache(IP_BE);

    //If not, request it
    if(MAC == 0xFFFFFFFFFFFF){
        RequestMACAddress(IP_BE);
    }

    //This isnt safe because the MAC address might not be in the cache yet or the machine may not be connected to the network (possible infinite loop)
    while (MAC == 0xFFFFFFFFFFFF) {                         //Wait until the MAC address is found
        MAC = GetMACFromCache(IP_BE);                       //Check if the MAC address is in the cache
    }

    //Return the MAC address

    return MAC;

}
