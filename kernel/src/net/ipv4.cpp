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

/**
 * @brief Construct a new IPV4 Address Resolver object and register it with the Internet Protocol Handler
 * @param internet_protocol_handler The Internet Protocol Handler to register with
 */
IPV4AddressResolver::IPV4AddressResolver(InternetProtocolHandler* internet_protocol_handler) {

	internet_protocol_handler->register_ipv_4_address_resolver(this);

}


IPV4AddressResolver::~IPV4AddressResolver() = default;

/**
 * @brief Resolves an IP address to a MAC address. (Default, returns broadcast address, override for use)
 *
 * @param address The IP address to turn into a MAC address.
 * @return The MAC address.
 */
MediaAccessControlAddress IPV4AddressResolver::resolve(InternetProtocolAddress address) {
	return 0xFFFFFFFFFFFF;
}

/**
 * @brief Stores an IP address to MAC address mapping. (Default, does nothing, override for use)
 *
 * @param internetProtocolAddress The IP address.
 * @param mediaAccessControlAddress The MAC address.
 */
void IPV4AddressResolver::store(InternetProtocolAddress, drivers::ethernet::MediaAccessControlAddress) {

}

/**
 * @brief Construct a new IPV4 Payload Handler object and register it with the Internet Protocol Handler
 *
 * @param internet_protocol_handler The Internet Protocol Handler to register with
 * @param protocol The IP protocol to handle
 */
IPV4PayloadHandler::IPV4PayloadHandler(InternetProtocolHandler* internet_protocol_handler, uint8_t protocol) {

	// Store vars
	this->internet_protocol_handler = internet_protocol_handler;
	this->ip_protocol = protocol;

	//Register handler
	internet_protocol_handler->connect_ipv_4_payload_handler(this);

}

IPV4PayloadHandler::~IPV4PayloadHandler() = default;

/**
 * @brief Called when an IP packet is received. (Deafult, does nothing, overide for use)
 *
 * @param src_ip_be The source IP address.
 * @param dst_ip_be The destination IP address.
 * @param internetprotocol_payload The payload of the IP packet.
 * @param size The size of the IP packet.
 * @return True if the packet was handled, false otherwise.
 */
bool IPV4PayloadHandler::handle_internet_protocol_payload(net::InternetProtocolAddress src_ip_be,
                                                          net::InternetProtocolAddress dst_ip_be,
                                                          uint8_t* internetprotocol_payload,
                                                          uint32_t size) {
	return false;
}

/**
 * @brief Sends an IP packet.
 *
 * @param destination_ip The destination IP address.
 * @param payload_data The payload of the IP packet.
 * @param size The size of the IP packet.
 */
void IPV4PayloadHandler::send(InternetProtocolAddress destination_ip, uint8_t* payload_data, uint32_t size) {


	//Pass to backend
	internet_protocol_handler->send_internet_protocol_packet(destination_ip, ip_protocol, payload_data, size);

}

/**
 * @brief Construct a new Internet Protocol Handler object
 *
 * @param backend The backend Ethernet frame handler.
 * @param own_internet_protocol_address The IP address of this device.
 * @param default_gateway_internet_protocol_address The IP address of the default gateway.
 * @param subnet_mask The subnet mask.
 * @param error_messages Where to write error messages.
 */
InternetProtocolHandler::InternetProtocolHandler(EthernetFrameHandler* backend, InternetProtocolAddress own_internet_protocol_address, InternetProtocolAddress default_gateway_internet_protocol_address, SubnetMask subnet_mask, OutputStream* error_messages)
: EthernetFramePayloadHandler(backend, 0x0800) {
	//Store vars
	this->own_internet_protocol_address = own_internet_protocol_address;
	this->default_gateway_internet_protocol_address = default_gateway_internet_protocol_address;
	this->subnet_mask = subnet_mask;
	this->error_messages = error_messages;
}

InternetProtocolHandler::~InternetProtocolHandler() = default;

/**
 * @brief Called when an IP packet is received.
 *
 * @param ethernetframe_payload The payload of the IP packet.
 * @param size The size of the IP packet.
 * @return True if the packet is to be sent back, false otherwise.
 *
 * @todo Set the identifier when sending packets back
 */
bool InternetProtocolHandler::handle_ethernetframe_payload(uint8_t* ethernetframe_payload, uint32_t size) {

	error_messages->write("IP: Handling packet\n");

	//Check if the size is big enough to contain an ethernet frame
	if(size < sizeof(IPV4Header))
		return false;

	//Convert to struct for easier use
	auto* ip_message = (IPV4Header*) ethernetframe_payload;
	bool send_back = false;

	//Only handle if it is for this device
	if(ip_message->destination_ip == get_internet_protocol_address()) {
		uint32_t length = ip_message->total_length;                          //Get length of the message
		if(length >
		   size)                                               //Check if the length is bigger than the size of the message
			length = size;                                              //If so, set length to size (this stops heartbleed attacks as it will not read past the end of the message, which the attacker could have filled with data)

		// Get the handler for the protocol
		Map<uint8_t, IPV4PayloadHandler*>::iterator handler_iterator = ipv_4_payload_handlers.find(ip_message->protocol);
		if(handler_iterator != ipv_4_payload_handlers.end()) {
			IPV4PayloadHandler* handler = handler_iterator->second;
			if(handler != nullptr) {
				send_back = handler->handle_internet_protocol_payload(ip_message->source_ip, ip_message->destination_ip,
				                                                      ethernetframe_payload + sizeof(IPV4Header),
				                                                      length - sizeof(IPV4Header));
			}
		}


	}

	//If the data is to be sent back again
	if(send_back) {

		//Swap source and destination
		uint32_t temp = ip_message->destination_ip;                                                                                     //Store destination IP
		ip_message->destination_ip = ip_message->source_ip;                                                                                //Set destination IP to source IP
		ip_message->source_ip = temp;                                                                                              //Set source IP to destination IP

		ip_message->time_to_live = 0x40;                                                                                         //Reset TTL
		ip_message->checksum = checksum((uint16_t*) ip_message, 4 *
		                                                        ip_message->header_length);                  //Reset checksum as the source and destination IP have changed so has the time to live and therefore there is a different header

	}

	error_messages->write("IP: Handled packet\n");
	return send_back;
}


/**
 * @brief Sends an IP packet.
 *
 * @param dst_ip_be The destination IP address.
 * @param protocol The protocol of the IP packet.
 * @param data The payload of the IP packet.
 * @param size The size of the IP packet.
 */
void InternetProtocolHandler::send_internet_protocol_packet(uint32_t dst_ip_be, uint8_t protocol, const uint8_t* data, uint32_t size) {

	auto* buffer = (uint8_t*) MemoryManager::kmalloc(
	sizeof(IPV4Header) + size);                           //Allocate memory for the message
	auto* message = (IPV4Header*) buffer;                                                                            //Convert to struct for easier use

	message->version = 4;                                                                                                                           //Set version
	message->header_length = sizeof(IPV4Header) /
	                         4;                                                                                     //Set header length
	message->type_of_service = 0;                                                                                                                     //Set type of service (not private)

	message->total_length = size +
	                        sizeof(IPV4Header);                                                                                 //Set total length
	message->total_length = ((message->total_length & 0xFF00)
	>> 8)                                                                                 // Convert to big endian (Swap bytes)
	                        | ((message->total_length & 0x00FF)
	<< 8);                                                                              // Convert to big endian (Swap bytes)

	message->identifier = 0x100;                                                                                                                    //Set identification TODO: do properly
	message->flags_and_offset = 0x0040;                                                                                                               //Set flags/offset, 0x40 because we are not fragmenting (TODO: doesnt work for packets bigger than 1500 bytes)

	message->time_to_live = 0x40;                                                                                                                     //Set time to live
	message->protocol = protocol;                                                                                                                   //Set protocol

	message->destination_ip = dst_ip_be;                                                                                                                      //Set destination IP
	message->source_ip = get_internet_protocol_address();                                                                                                  //Set source IP

	message->checksum = 0;                                                                                                                          //Set checksum to 0, init with 0 as checksum funct will also add this value
	message->checksum = checksum((uint16_t*) message,
	                             sizeof(IPV4Header));                                             //Calculate checksum

	//Copy data
	uint8_t* data_buffer = buffer +
	                       sizeof(IPV4Header);                                                                                  //Get pointer to the data
	for(uint32_t i = 0; i <
	                    size; i++)                                                                                                                            //Loop through data
		data_buffer[i] = data[i];                                                                                                                             //Copy data

	//Check if the destination is on the same subnet, The if condition determines if the destination device is on the same Local network as the source device . and if they are not on the same local network then we resolve the ip address of the gateway .
	InternetProtocolAddress route = dst_ip_be;                                                                                                                               //Set route to destination IP by default
	if((dst_ip_be & subnet_mask) != (own_internet_protocol_address &
	                                 subnet_mask))                                                                                             //Check if the destination is on the same subnet
		route = default_gateway_internet_protocol_address;                                                                                                                                   //If not, set route to gateway IP
	//Print debug info
	uint32_t mac = resolver->resolve(route);

	//Send message
	frame_handler->send_ethernet_frame(mac, this->handled_type, buffer, size + sizeof(IPV4Header));      //Send message
	MemoryManager::kfree(
	buffer);                                                                                                 //Free memory
}

/**
 * @brief Creates a checksum for the given data.
 *
 * @param data The data to create a checksum for.
 * @param length_in_bytes The length of the data in bytes.
 * @return The checksum.
 */
uint16_t InternetProtocolHandler::checksum(const uint16_t* data, uint32_t length_in_bytes) {

	uint32_t temp = 0;                                                                             //Init sum

	for(uint32_t i = 0; i < length_in_bytes /
	                        2; i++)                                                       //Loop through data (/2 bc bytes)
		temp += ((data[i] & 0xFF00) >> 8) |
		        ((data[i] & 0x00FF) << 8);                             //Add data to sum in big endian

	if(length_in_bytes %
	   2)                                                                          //If there is an odd number of bytes
		temp += ((uint16_t) ((char*) data)[length_in_bytes - 1])
		<< 8;                                   //Add the last byte to the sum

	while(temp &
	      0xFFFF0000)                                                                       //While there is a carry
		temp = (temp & 0xFFFF) +
		       (temp >> 16);                                                     //Add the carry to the sum

	return ((~temp & 0xFF00) >> 8) | ((~temp & 0x00FF) << 8);
}

/**
 * @brief Registers an IP address resolver.
 *
 * @param resolver The resolver to register.
 */
void InternetProtocolHandler::register_ipv_4_address_resolver(IPV4AddressResolver* ipv4_resolver) {

	this->resolver = ipv4_resolver;

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
InternetProtocolAddress InternetProtocolHandler::create_internet_protocol_address(uint8_t digit1, uint8_t digit2, uint8_t digit3, uint8_t digit4) {
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
InternetProtocolAddress InternetProtocolHandler::parse(string address) {
	uint8_t digits[4];

	uint8_t current_digit = 0;
	for(unsigned char& digit : digits)
		digit = 0;

	for(size_t i = 0; i < address.length(); i++) {
		if(address[i] == '.') {
			current_digit++;
			continue;
		}

		digits[current_digit] *= 10;
		digits[current_digit] += address[i] - '0';
	}

	return create_internet_protocol_address(digits[0], digits[1], digits[2], digits[3]);
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
SubnetMask InternetProtocolHandler::create_subnet_mask(uint8_t digit1, uint8_t digit2, uint8_t digit3, uint8_t digit4) {
	return (SubnetMask) create_internet_protocol_address(digit1, digit2, digit3, digit4);
}

/**
 * @brief Gets the IP address of this device.
 *
 * @return The IP address.
 */
InternetProtocolAddress InternetProtocolHandler::get_internet_protocol_address() const {
	return own_internet_protocol_address;
}

/**
 * @brief Gets the MAC address of this device.
 *
 * @return The MAC address.
 */
MediaAccessControlAddress InternetProtocolHandler::get_media_access_control_address() {
	return frame_handler->get_mac();
}

/**
 * @brief Connects an IP protocol payload handler.
 *
 * @param ipv_4_payload_handler The payload handler to connect.
 */
void InternetProtocolHandler::connect_ipv_4_payload_handler(IPV4PayloadHandler* ipv_4_payload_handler) {
	ipv_4_payload_handlers.insert(ipv_4_payload_handler->ip_protocol, ipv_4_payload_handler);
}
