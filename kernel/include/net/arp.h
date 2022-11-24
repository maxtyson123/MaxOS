//
// Created by 98max on 11/11/2022.
//

#ifndef MAXOS_NET_ARP_H
#define MAXOS_NET_ARP_H

#include <common/types.h>
#include <common/timer.h>
#include <net/etherframe.h>

namespace maxOS{

    namespace net{

        struct AddressResolutionProtocolMessage {

            common::uint16_t hardwareType;
            common::uint16_t protocol;
            common::uint8_t hardwareAddressSize;    //6
            common::uint8_t protocolAddressSize;    //4

            common::uint16_t command;               //1 = request, 2 = reply

            common::uint64_t srcMAC : 48;
            common::uint32_t srcIP;
            common::uint64_t dstMAC : 48;
            common::uint32_t dstIP;



        }__attribute__((packed));

        class AddressResolutionProtocol : EtherFrameHandler{
            private:
                common::uint32_t cacheIPAddress[128];
                common::uint32_t cacheMACAddress[128];
                int numCacheEntries;

            public:
                AddressResolutionProtocol(EtherFrameProvider* backend);
                ~AddressResolutionProtocol();

                bool OnEtherFrameReceived(common::uint8_t* etherframePayload, common::uint32_t size);

                void RequestMACAddress(common::uint32_t IP_BE);
                common::uint64_t GetMACFromCache(common::uint32_t IP_BE);

                common::uint64_t Resolve(common::uint32_t IP_BE);
                void BroadcastMACAddress(common::uint32_t IP_BE);
        };

    }

}


#endif //MAXOS_NET_ARP_H
