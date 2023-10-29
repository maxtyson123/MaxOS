//
// Created by 98max on 24/11/2022.
//

#ifndef MAXOS_NET_ICMP_H
#define MAXOS_NET_ICMP_H

#include <net/ipv4.h>
#include <common/types.h>


namespace maxOS{

    namespace net{

        struct InternetControlMessageProtocolHeader{
            common::uint8_t type;
            common::uint8_t code;

            common::uint16_t checksum;
            common::uint32_t data;
        }__attribute__((packed));                       // Packed to avoid padding

        class InternetControlMessageProtocol : InternetProtocolPayloadHandler{

            public:
                InternetControlMessageProtocol(InternetProtocolHandler* internetProtocolHandler);
                ~InternetControlMessageProtocol();

                bool handleInternetProtocolPayload(InternetProtocolAddress sourceIP, InternetProtocolAddress destinationIP, common::uint8_t* payloadData, common::uint32_t size);
                void RequestEchoReply(InternetProtocolAddress address);
        };

    }

}

#endif //MAXOS_NET_ICMP_H
