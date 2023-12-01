//
// Created by 98max on 22/11/2022.
//

#ifndef MAXOS_NET_IPV4_H
#define MAXOS_NET_IPV4_H

#include <common/types.h>
#include <net/ethernetframe.h>

namespace maxOS{

    namespace net{

        typedef common::uint32_t InternetProtocolAddress;
        typedef common::uint32_t SubnetMask;

        struct InternetProtocolV4Header
        {
            common::uint8_t headerLength : 4;
            common::uint8_t version : 4;
            common::uint8_t typeOfService;
            common::uint16_t totalLength;

            common::uint16_t identifier;
            common::uint16_t flagsAndOffset;

            common::uint8_t timeToLive;
            common::uint8_t protocol;
            common::uint16_t checksum;

            common::uint32_t sourceIP;
            common::uint32_t destinationIP;
        } __attribute__((packed));

        class InternetProtocolHandler;
        class InternetProtocolAddressResolver
        {
        public:
            InternetProtocolAddressResolver(InternetProtocolHandler* internetProtocolHandler);
            ~InternetProtocolAddressResolver();
            virtual drivers::ethernet::MediaAccessControlAddress Resolve(InternetProtocolAddress address);
            virtual void Store(InternetProtocolAddress internetProtocolAddress, drivers::ethernet::MediaAccessControlAddress mediaAccessControlAddress);
        };

        class InternetProtocolPayloadHandler
        {
            friend class InternetProtocolHandler;

            protected:
                InternetProtocolHandler* internetProtocolHandler;
                common::uint8_t ipProtocol;

            public:
                InternetProtocolPayloadHandler(InternetProtocolHandler* internetProtocolHandler, common::uint8_t protocol);
                ~InternetProtocolPayloadHandler();

                virtual bool handleInternetProtocolPayload(InternetProtocolAddress sourceIP, InternetProtocolAddress destinationIP, common::uint8_t* payloadData, common::uint32_t size);
                void Send(InternetProtocolAddress destinationIP, common::uint8_t* payloadData, common::uint32_t size);
        };

        class InternetProtocolHandler : public net::EthernetFramePayloadHandler{

            friend class InternetProtocolAddressResolver;

            protected:

                common::Map<common::uint8_t, InternetProtocolPayloadHandler*> internetProtocolPayloadHandlers;

                InternetProtocolAddressResolver* resolver;
                common::OutputStream* errorMessages;

                InternetProtocolAddress ownInternetProtocolAddress;
                InternetProtocolAddress defaultGatewayInternetProtocolAddress;
                SubnetMask subnetMask;

                void RegisterInternetProtocolAddressResolver(InternetProtocolAddressResolver* resolver);

            public:
                InternetProtocolHandler(EthernetFrameHandler* backend,
                                         InternetProtocolAddress ownInternetProtocolAddress,
                                         InternetProtocolAddress defaultGatewayInternetProtocolAddress,
                                         SubnetMask subnetMask,
                                         common::OutputStream* errorMessages);
                ~InternetProtocolHandler();

                bool handleEthernetframePayload(common::uint8_t* ethernetframePayload, common::uint32_t size);
                void sendInternetProtocolPacket(common::uint32_t dstIP_BE, common::uint8_t protocol, common::uint8_t* data, common::uint32_t size);

                static common::uint16_t Checksum(common::uint16_t* data, common::uint32_t lengthInBytes);

                static InternetProtocolAddress CreateInternetProtocolAddress(common::uint8_t digit1, common::uint8_t digit2, common::uint8_t digit3, common::uint8_t digit4);
                static InternetProtocolAddress Parse(common::string address);
                static SubnetMask CreateSubnetMask(common::uint8_t digit1, common::uint8_t digit2, common::uint8_t digit3, common::uint8_t digit4);
                InternetProtocolAddress GetInternetProtocolAddress();
                drivers::ethernet::MediaAccessControlAddress GetMediaAccessControlAddress();

                void connectInternetProtocolPayloadHandler(InternetProtocolPayloadHandler* internetProtocolPayloadHandler);


        };


    }

}


#endif //MAXOS_NET_IPV4_H
