//
// Created by 98max on 24/11/2022.
//

#include <net/icmp.h>

using namespace  maxOS;
using namespace  maxOS::common;
using namespace  maxOS::net;


InternetControlMessageProtocol::InternetControlMessageProtocol(InternetProtocolHandler *internetProtocolHandler)
: InternetProtocolPayloadHandler(internetProtocolHandler, 0x01)
{

}



InternetControlMessageProtocol::~InternetControlMessageProtocol() {

}
/**
 * @details Called by the InternetProtocolProvider when a new packet has arrived
 *
 * @param srcIP_BE  The source IP address of the packet
 * @param dstIP_BE  The destination IP address of the packet
 * @param internetprotocolPayload  The payload of the packet
 * @param size The size of the payload

 * @return True if the packet is to be sent back to the sender, false otherwise
 */
bool InternetControlMessageProtocol::handleInternetProtocolPayload(InternetProtocolAddress sourceIP,
                                                                   InternetProtocolAddress destinationIP,
                                                                   uint8_t *payloadData,
                                                                   uint32_t size)
{

    // Check if the size is at least the size of the header
    if(size < sizeof(InternetControlMessageProtocolHeader)){
        return false;
    }

    // Cast the payload to the ICMP header
    InternetControlMessageProtocolHeader* icmp = (InternetControlMessageProtocolHeader*)payloadData;

    switch (icmp -> type) {

        case 0: // Echo reply
            // TODO: reply to the ping
            break;

        case 8: // Echo request

            // Create a response
            icmp -> type = 0;                                                                                                                    // Echo reply
            icmp -> checksum = 0;                                                                                                                // Reset the checksum
            icmp -> checksum = InternetProtocolHandler::Checksum((uint16_t *)&icmp, sizeof(InternetControlMessageProtocolHeader));             // Calculate the checksum

            return true;    //Send data back

    }

    return false;

    //TODO: Implement the rest of the ICMP messages

}

/**
 * @details Sends an ICMP echo request to the specified IP address
 *
 * @param ip_be The IP address to send the request to
 */
void InternetControlMessageProtocol::RequestEchoReply(uint32_t ip_be) {

    InternetControlMessageProtocolHeader icmp;
    icmp.type = 8;                      // Echo request
    icmp.code = 0;                      // Code must be 0
    icmp.checksum = 0;                  // Checksum must be 0 to calculate it
    icmp.data = 0x69420;        // Data

    icmp.checksum = InternetProtocolHandler::Checksum((uint16_t *)&icmp, sizeof(InternetControlMessageProtocolHeader));

    Send(ip_be, (uint8_t *)&icmp, sizeof(InternetControlMessageProtocolHeader));
}
