//
// Created by 98max on 22/11/2022.
//

#ifndef MAXOS_NET_IPV4_H
#define MAXOS_NET_IPV4_H

#include <common/types.h>
#include <net/etherframe.h>
#include <net/arp.h>

namespace maxOS{

    namespace net{

        struct InternetProtocolVersion4Message{

            common::uint8_t headerLength : 4;           // Set to 4 bits, header is here first becuase of big endian encoded
            common::uint8_t version : 4;                // Set to 4 bits
            common::uint8_t tos;                        // Type of service
            common::uint16_t totalLength;               // Total length of the message

            common::uint16_t identification;            // Identification of the message
            common::uint8_t flagsAndOffset;             // Flags and offset

            common::uint8_t timeToLive;                 // Time to live
            common::uint8_t protocol;                   // Protocol
            common::uint16_t checksum;                  // Header checksum

            common::uint32_t srcIP;                     // Source IP
            common::uint32_t dstIP;                     // Destination IP


        }__attribute__((packed));                       // Packed to avoid padding

        class InternetProtocolProvider;

        class InternetProtocolHandler
        {
            protected:
                InternetProtocolProvider* backend;
                common::uint8_t ip_protocol;

            public:
                InternetProtocolHandler(InternetProtocolProvider* backend, common::uint8_t protocol);
                ~InternetProtocolHandler();

                virtual bool OnInternetProtocolReceived(common::uint32_t srcIP_BE, common::uint32_t dstIP_BE, common::uint8_t* internetprotocolPayload, common::uint32_t size);
                void Send(common::uint32_t dstIP_BE, common::uint8_t* internetProtocolPayload, common::uint32_t size);
        };

        class InternetProtocolProvider : net::EtherFrameHandler{

            friend class InternetProtocolHandler;

            protected:
                InternetProtocolHandler* handlers[255];
                AddressResolutionProtocol* arp;

                common::uint32_t gatewayIP;
                common::uint32_t subnetMask;

            public:
                InternetProtocolProvider(EtherFrameProvider* backend, AddressResolutionProtocol* arp, common::uint32_t gatewayIP, common::uint32_t subnetMask);
                ~InternetProtocolProvider();

                bool OnEtherFrameReceived(common::uint8_t* etherframePayload, common::uint32_t size);

                void Send(common::uint32_t dstIP_BE, common::uint8_t protocol, common::uint8_t* data, common::uint32_t size);

                static common::uint16_t Checksum(common::uint16_t* data, common::uint32_t lengthInBytes);

        };


    }

}


#endif //MAXOS_NET_IPV4_H
