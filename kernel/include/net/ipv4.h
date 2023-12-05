//
// Created by 98max on 22/11/2022.
//

#ifndef MAXOS_NET_IPV4_H
#define MAXOS_NET_IPV4_H

#include <stdint.h>
#include <net/ethernetframe.h>

namespace maxOS{

    namespace net{

        typedef uint32_t InternetProtocolAddress;
        typedef uint32_t SubnetMask;

        struct InternetProtocolV4Header
        {
            uint8_t headerLength : 4;
            uint8_t version : 4;
            uint8_t typeOfService;
            uint16_t totalLength;

            uint16_t identifier;
            uint16_t flagsAndOffset;

            uint8_t timeToLive;
            uint8_t protocol;
            uint16_t checksum;

            uint32_t sourceIP;
            uint32_t destinationIP;
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
                uint8_t ipProtocol;

            public:
                InternetProtocolPayloadHandler(InternetProtocolHandler* internetProtocolHandler, uint8_t protocol);
                ~InternetProtocolPayloadHandler();

                virtual bool handleInternetProtocolPayload(InternetProtocolAddress sourceIP, InternetProtocolAddress destinationIP, uint8_t* payloadData, uint32_t size);
                void Send(InternetProtocolAddress destinationIP, uint8_t* payloadData, uint32_t size);
        };

        class InternetProtocolHandler : public EthernetFramePayloadHandler{

            friend class InternetProtocolAddressResolver;

            protected:

                common::Map<uint8_t, InternetProtocolPayloadHandler*> internetProtocolPayloadHandlers;

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

                bool handleEthernetframePayload(uint8_t* ethernetframePayload, uint32_t size) override;
                void sendInternetProtocolPacket(uint32_t dstIP_BE, uint8_t protocol, uint8_t* data, uint32_t size);

                static uint16_t Checksum(uint16_t* data, uint32_t lengthInBytes);

                static InternetProtocolAddress CreateInternetProtocolAddress(uint8_t digit1, uint8_t digit2, uint8_t digit3, uint8_t digit4);
                static InternetProtocolAddress Parse(string address);
                static SubnetMask CreateSubnetMask(uint8_t digit1, uint8_t digit2, uint8_t digit3, uint8_t digit4);
                InternetProtocolAddress GetInternetProtocolAddress();
                drivers::ethernet::MediaAccessControlAddress GetMediaAccessControlAddress();

                void connectInternetProtocolPayloadHandler(InternetProtocolPayloadHandler* internetProtocolPayloadHandler);


        };


    }

}


#endif //MAXOS_NET_IPV4_H
