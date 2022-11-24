//
// Created by 98max on 24/11/2022.
//

#include <net/icmp.h>

using namespace  maxOS;
using namespace  maxOS::common;
using namespace  maxOS::net;

void printf(char* str, bool clearLine = false); //Forward declaration
void printfHex(uint8_t key);                    //Forward declaration
char printfInt( long num );                     //Forward declaration

InternetControlMessageProtocol::InternetControlMessageProtocol(InternetProtocolProvider *backend)
: InternetProtocolHandler(backend, 0x01)        // 0x01 is the ICMP protocol
{

}


InternetControlMessageProtocol::~InternetControlMessageProtocol() {

}

bool InternetControlMessageProtocol::OnInternetProtocolReceived(uint32_t srcIP_BE, uint32_t dstIP_BE, uint8_t *internetprotocolPayload, uint32_t size) {

    // Check if the size is at least the size of the header
    if(size < sizeof(InternetControlMessageProtocolMessage)){
        return false;
    }

    // Cast the payload to the ICMP header
    InternetControlMessageProtocolMessage* icmp = (InternetControlMessageProtocolMessage*)internetprotocolPayload;

    switch (icmp -> type) {

        case 0: // Echo reply
            printf("\nPing response from "); printfHex(srcIP_BE & 0xFF);
            printf("."); printfHex((srcIP_BE >> 8) & 0xFF);
            printf("."); printfHex((srcIP_BE >> 16) & 0xFF);
            printf("."); printfHex((srcIP_BE >> 24) & 0xFF);
            printf("\n");
            break;

        case 8: // Echo request
            printf("\nPing request from "); printfHex(srcIP_BE & 0xFF);
            printf("."); printfHex((srcIP_BE >> 8) & 0xFF);
            printf("."); printfHex((srcIP_BE >> 16) & 0xFF);
            printf("."); printfHex((srcIP_BE >> 24) & 0xFF);
            printf("\n");

            // Create a response
            icmp -> type = 0;                                                                                                                    // Echo reply
            icmp -> checksum = 0;                                                                                                                // Reset the checksum
            icmp -> checksum = InternetProtocolProvider::Checksum((uint16_t *)&icmp, sizeof(InternetControlMessageProtocolMessage));             // Calculate the checksum

            return true;    //Send data back

    }

    return false;

    //TODO: Implement the rest of the ICMP messages

}

void InternetControlMessageProtocol::RequestEchoReply(uint32_t ip_be) {

    InternetControlMessageProtocolMessage icmp;
    icmp.type = 8;                      // Echo request
    icmp.code = 0;                      // Code must be 0
    icmp.checksum = 0;                  // Checksum must be 0 to calculate it
    icmp.restOfHeader = 0x69420;        // Data

    icmp.checksum = InternetProtocolProvider::Checksum((uint16_t *)&icmp, sizeof(InternetControlMessageProtocolMessage));

    InternetProtocolHandler::Send(ip_be, (uint8_t *)&icmp, sizeof(InternetControlMessageProtocolMessage));
}
