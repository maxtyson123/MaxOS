/**
 * @file icmp.cpp
 * @brief Implementation of the Internet Control Message Protocol (ICMP) handler
 *
 * @date 24th November 2022
 * @author Max Tyson
 */

#include <net/icmp.h>

using namespace  MaxOS;
using namespace  MaxOS::common;
using namespace  MaxOS::net;

/**
 * @brief Constructs an InternetControlMessageProtocol handler.
 *
 * @param internet_protocol_handler The Internet protocol handler to use.
 * @param error_messages The output stream to use for error messages.
 */
InternetControlMessageProtocol::InternetControlMessageProtocol(InternetProtocolHandler *internet_protocol_handler, OutputStream* error_messages)
: IPV4PayloadHandler(internet_protocol_handler, 0x01)
{
    this -> error_messages = error_messages;
}



InternetControlMessageProtocol::~InternetControlMessageProtocol() = default;
/**
 * @brief Called by the InternetProtocolProvider when a new packet has arrived
 *
 * @param src_ip_be  The source IP address of the packet
 * @param dst_ip_be  The destination IP address of the packet
 * @param payload_data  The payload of the packet
 * @param size The size of the payload

 * @return True if the packet is to be sent back to the sender, false otherwise
 *
 * @todo Reply to ping requests
 * @todo Implement the rest of the ICMP messages
 */
bool InternetControlMessageProtocol::handle_internet_protocol_payload(net::InternetProtocolAddress src_ip_be,
                                                                      net::InternetProtocolAddress dst_ip_be,
                                                                      uint8_t *payload_data,
                                                                      uint32_t size)
{

    error_messages -> write("ICMP received a packet\n");

    // Check if the size is at least the size of the header
    if(size < sizeof(ICMPHeader)){
        return false;
    }

    // Cast the payload to the ICMP header
    auto* icmp = (ICMPHeader*)payload_data;

    switch (icmp -> type) {

        case 0: // Echo reply
            break;

        case 8: // Echo request

            // Create a response
            icmp -> type = 0;                                                                                                                    // Echo reply
            icmp -> checksum = 0;                                                                                                                // Reset the checksum
            icmp -> checksum = InternetProtocolHandler::checksum((uint16_t*) &icmp, sizeof(ICMPHeader));             // Calculate the checksum

            return true;    //Send data back

    }

    return false;

}

/**
 * @brief Sends an ICMP echo request to the specified IP address
 *
 * @param ip_be The IP address to send the request to
 */
void InternetControlMessageProtocol::request_echo_reply(uint32_t ip_be) {

    error_messages -> write("ICMP: Sending echo request\n");

    ICMPHeader icmp = {};
    icmp.type = 8;                      // Echo request
    icmp.code = 0;                      // Code must be 0
    icmp.checksum = 0;                  // Checksum must be 0 to calculate it
    icmp.data = 0x69420;        // Data

    icmp.checksum = InternetProtocolHandler::checksum((uint16_t*) &icmp, sizeof(ICMPHeader));

	send(ip_be, (uint8_t*) &icmp, sizeof(ICMPHeader));

    error_messages -> write("ICMP: Echo request sent\n");
}
