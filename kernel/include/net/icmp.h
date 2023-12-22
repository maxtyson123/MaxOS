//
// Created by 98max on 24/11/2022.
//

#ifndef MAXOS_NET_ICMP_H
#define MAXOS_NET_ICMP_H

#include <net/ipv4.h>
#include <stdint.h>
#include <common/outputStream.h>


namespace maxOS{

    namespace net{

        struct InternetControlMessageProtocolHeader{
            uint8_t type;
            uint8_t code;

            uint16_t checksum;
            uint32_t data;
        }__attribute__((packed));                       // Packed to avoid padding

        class InternetControlMessageProtocol : InternetProtocolPayloadHandler{

            common::OutputStream* errorMessages;

            public:
                InternetControlMessageProtocol(InternetProtocolHandler* internetProtocolHandler, common::OutputStream* errorMessages);
                ~InternetControlMessageProtocol();

                bool handleInternetProtocolPayload(InternetProtocolAddress sourceIP, InternetProtocolAddress destinationIP, uint8_t* payloadData, uint32_t size);
                void RequestEchoReply(InternetProtocolAddress address);
        };

    }

}

#endif //MAXOS_NET_ICMP_H
