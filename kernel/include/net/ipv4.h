//
// Created by 98max on 22/11/2022.
//

#ifndef MAXOS_NET_IPV4_H
#define MAXOS_NET_IPV4_H

#include <common/types.h>
#include <net/etherframe.h>

namespace maxOS{

    namespace net{

        typedef common::uint32_t InternetProtocolAddress;
        typedef common::uint32_t SubnetMask;

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
        class InternetProtocolAddressResolver
        {
        public:
            InternetProtocolAddressResolver(InternetProtocolProvider* internetProtocolProvider);
            ~InternetProtocolAddressResolver();
            virtual drivers::ethernet::MediaAccessControlAddress Resolve(InternetProtocolAddress address);
            virtual void Store(InternetProtocolAddress internetProtocolAddress, drivers::ethernet::MediaAccessControlAddress mediaAccessControlAddress);
        };

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

        class InternetProtocolProvider : public net::EtherFrameHandler{

            friend class InternetProtocolHandler;
            friend class InternetProtocolAddressResolver;

            protected:
                InternetProtocolHandler* internetProtocolHandlers[255];
                InternetProtocolAddressResolver* resolver;

                InternetProtocolAddress ownInternetProtocolAddress;
                InternetProtocolAddress defaultGatewayInternetProtocolAddress;
                SubnetMask subnetMask;

                void RegisterInternetProtocolAddressResolver(InternetProtocolAddressResolver* resolver);

            public:
                InternetProtocolProvider(EtherFrameProvider* backend, InternetProtocolAddress ownInternetProtocolAddress,
                                         InternetProtocolAddress defaultGatewayInternetProtocolAddress, SubnetMask subnetMask);
                ~InternetProtocolProvider();

                bool OnEtherFrameReceived(common::uint8_t* etherframePayload, common::uint32_t size);
                void Send(common::uint32_t dstIP_BE, common::uint8_t protocol, common::uint8_t* data, common::uint32_t size);

                static common::uint16_t Checksum(common::uint16_t* data, common::uint32_t lengthInBytes);

                static InternetProtocolAddress CreateInternetProtocolAddress(common::uint8_t digit1, common::uint8_t digit2, common::uint8_t digit3, common::uint8_t digit4);
                static InternetProtocolAddress Parse(common::string address);
                static SubnetMask CreateSubnetMask(common::uint8_t digit1, common::uint8_t digit2, common::uint8_t digit3, common::uint8_t digit4);
                InternetProtocolAddress GetInternetProtocolAddress();
                drivers::ethernet::MediaAccessControlAddress GetMediaAccessControlAddress();


        };


    }

}


#endif //MAXOS_NET_IPV4_H
