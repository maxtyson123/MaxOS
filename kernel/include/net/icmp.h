//
// Created by 98max on 24/11/2022.
//

#ifndef MAXOS_NET_ICMP_H
#define MAXOS_NET_ICMP_H

#include <net/ipv4.h>
#include <common/types.h>


namespace maxOS{

    namespace net{

        struct InternetControlMessageProtocolMessage{
            common::uint8_t type;
            common::uint8_t code;

            common::uint16_t checksum;
            common::uint32_t restOfHeader;              // (Data)
        }__attribute__((packed));                       // Packed to avoid padding

        class InternetControlMessageProtocol : InternetProtocolHandler{

            public:
                InternetControlMessageProtocol(InternetProtocolProvider* backend);
                ~InternetControlMessageProtocol();

                bool OnInternetProtocolReceived(common::uint32_t srcIP_BE, common::uint32_t dstIP_BE, common::uint8_t* internetprotocolPayload, common::uint32_t size);
                void RequestEchoReply(common::uint32_t ip_be);
        };

    }

}

#endif //MAXOS_NET_ICMP_H
