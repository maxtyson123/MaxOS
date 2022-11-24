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

        struct InternetProtocolV4Message
        {
            common::uint8_t headerLength : 4;
            common::uint8_t version : 4;
            common::uint8_t tos;
            common::uint16_t totalLength;

            common::uint16_t ident;
            common::uint16_t flagsAndOffset;

            common::uint8_t timeToLive;
            common::uint8_t protocol;
            common::uint16_t checksum;

            common::uint32_t srcIP;
            common::uint32_t dstIP;
        } __attribute__((packed));


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
