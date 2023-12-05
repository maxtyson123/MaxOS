//
// Created by 98max on 11/11/2022.
//

#ifndef MAXOS_NET_ARP_H
#define MAXOS_NET_ARP_H

#include <stdint.h>

#include <net/ipv4.h>

namespace maxOS{

    namespace net{

        struct AddressResolutionProtocolMessage {

            uint16_t hardwareType;
            uint16_t protocol;
            uint8_t hardwareAddressSize;    //6
            uint8_t protocolAddressSize;    //4

            uint16_t command;               //1 = request, 2 = reply

            uint64_t srcMAC : 48;
            uint32_t srcIP;
            uint64_t dstMAC : 48;
            uint32_t dstIP;



        }__attribute__((packed));

        class AddressResolutionProtocol : public EthernetFramePayloadHandler, public InternetProtocolAddressResolver
        {
            private:
                common::Map<InternetProtocolAddress, drivers::ethernet::MediaAccessControlAddress> addressCache;
                InternetProtocolHandler* internetProtocolHandler;
                common::OutputStream* errorMessages;
            public:
                AddressResolutionProtocol(EthernetFrameHandler* ethernetFrameHandler, InternetProtocolHandler* internetProtocolHandler, common::OutputStream* errorMessages);
                ~AddressResolutionProtocol();

                bool handleEthernetFramePayload(uint8_t* data, uint32_t size);

                void RequestMACAddress(InternetProtocolAddress address);
                drivers::ethernet::MediaAccessControlAddress Resolve(InternetProtocolAddress address);
                void Store(InternetProtocolAddress internetProtocolAddress, drivers::ethernet::MediaAccessControlAddress mediaAccessControlAddress);
        };

    }

}


#endif //MAXOS_NET_ARP_H
