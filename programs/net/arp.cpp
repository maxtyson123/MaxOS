/**
 * @file arp.cpp
 * @brief Implementation of the Address Resolution Protocol (ARP) for resolving IP addresses to MAC addresses
 *
 * @date 11th November 2022
 * @author Max Tyson
 */

#include <net/arp.h>

using namespace MaxOS;
using namespace MaxOS::common;
using namespace MaxOS::net;
using namespace MaxOS::drivers;
using namespace MaxOS::drivers::ethernet;


/**
 * @brief Constructs an AddressResolutionProtocol handler.
 *
 * @param ethernet_frame_handler The Ethernet frame handler to use.
 * @param internet_protocol_handler The Internet protocol handler to use.
 * @param error_messages The output stream to use for error messages.
 */
net::AddressResolutionProtocol::AddressResolutionProtocol(EthernetFrameHandler* ethernet_frame_handler, InternetProtocolHandler* internet_protocol_handler, OutputStream* error_messages)
		: EthernetFramePayloadHandler(ethernet_frame_handler, 0x0806),
		IPV4AddressResolver(internet_protocol_handler) {
	this->internet_protocol_handler = internet_protocol_handler;
	this->error_messages = error_messages;
}

net::AddressResolutionProtocol::~AddressResolutionProtocol() = default;

/**
 * @brief Called when an ARP packet is received.
 *
 * @param ethernetframePayload The payload of the ARP packet.
 * @param size The size of the ARP packet.
 * @return True if the device should send a response, false otherwise.
 */
bool AddressResolutionProtocol::handle_ethernetframe_payload(uint8_t* ethernetframePayload, uint32_t size) {

	//Check if the size is correct
	if (size < sizeof(ARPMessage))
		return false;

	//Convert the payload to an ARP message
	auto* arp_message = (ARPMessage*) ethernetframePayload;

	//Check if the message hardware type is Ethernet (BigEndian)
	if (arp_message->hardware_type == 0x100) {

		if (arp_message->protocol == 0x0008             //Check if the protocol is IPv4 (BigEndian)
		    && arp_message->hardware_address_size == 6
		    && arp_message->protocol_address_size == 4
		    && arp_message->dst_ip == internet_protocol_handler->get_internet_protocol_address()) {

			switch (arp_message->command) {
				//Request
				case 0x0100:
					arp_message->command = 0x0200;                                                                         //Set the command to reply
					arp_message->dst_mac = arp_message->src_mac;                                                            //Set the destination MAC to the source MAC
					arp_message->dst_ip = arp_message->src_ip;                                                              //Set the destination IP to the source IP
					arp_message->src_mac = internet_protocol_handler->get_media_access_control_address();                      //Set the source MAC to this MAC
					arp_message->src_ip = internet_protocol_handler->get_internet_protocol_address();                         //Set the source IP to this IP
					return true;

					//Response
				case 0x0200:
					address_cache.insert((InternetProtocolAddress) arp_message->src_ip,
					                     (MediaAccessControlAddress) arp_message->src_mac);     //Insert the MAC address into the cache
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
 * @param address The IP address in BigEndian.
 */
void AddressResolutionProtocol::request_mac_address(InternetProtocolAddress address) {

	//When a MAC address is requested, instantiate a new ARP message block on the stack
	ARPMessage arp_message = { };

	//Set the message's values
	arp_message.hardware_type = 0x0100;                                                   //Ethernet, encoded in BigEndian
	arp_message.protocol = 0x0008;                                                       //IPv4, encoded in BigEndian
	arp_message.hardware_address_size = 6;                                                 //MAC address size
	arp_message.protocol_address_size = 4;                                                 //IPv4 address size
	arp_message.command = 0x0100;                                                        //Request, encoded in BigEndian

	//Set the message's source and destination
	arp_message.src_mac = frame_handler->get_mac();                                       //Set the source MAC address to the backend's MAC address
	arp_message.src_ip = internet_protocol_handler->get_internet_protocol_address();        //Set the source IP address to the backend's IP address
	arp_message.dst_mac = 0xFFFFFFFFFFFF;                                                 //Set the destination MAC address to broadcast
	arp_message.dst_ip = address;                                                           //Set the destination IP address to the requested IP address

	//Send the message
	this->send(arp_message.dst_mac, (uint8_t*) &arp_message, sizeof(ARPMessage));


}


/**
 * @brief Get the MAC address from an IP via ARP.
 *
 * @param address The IP address to get the MAC address from.
 * @return The MAC address of the IP address.
 *
 * @todo Should have a timeout in case the address cannot be resolved and avoid infinite loops
 */
MediaAccessControlAddress AddressResolutionProtocol::resolve(InternetProtocolAddress address) {

	volatile Map<InternetProtocolAddress, MediaAccessControlAddress>::iterator cache_iterator = address_cache.find(
			address); //Check if the MAC address is in the cache

	//If not, request it
	if (address_cache.end() == cache_iterator) {
		request_mac_address(address);
	}

	//This isn't safe because the MAC address might not be in the cache yet or the machine may not be connected to the network (possible infinite loop) //TODO: TIMEOUT
	while (cache_iterator == address_cache.end()) {                         //Wait until the MAC address is found
		cache_iterator = address_cache.find(address);
	}

	//Return the MAC address

	return cache_iterator->second;

}

/**
 * @brief store a mapping of an IP address to a MAC address.
 *
 * @param internet_protocol_address The IP address.
 * @param media_access_control_address The MAC address.
 */
void AddressResolutionProtocol::store(InternetProtocolAddress internet_protocol_address, drivers::ethernet::MediaAccessControlAddress media_access_control_address) {
	address_cache.insert(internet_protocol_address, media_access_control_address);
}
