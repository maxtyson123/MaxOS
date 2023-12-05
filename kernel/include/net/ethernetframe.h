//
// Created by 98max on 5/11/2022.
//

#ifndef MAXOS_NET_ETHERNETFRAME_H
#define MAXOS_NET_ETHERNETFRAME_H

#include <stdint.h>
#include <common/map.h>
#include <drivers/ethernet/ethernet.h>
#include <memory/memorymanagement.h>

namespace maxOS{

    namespace net{

        //Structure the raw data
        struct EthernetFrameHeader{

            uint64_t destinationMAC : 48;     //Destination MAC Address (Big Endian)
            uint64_t sourceMAC : 48;     //Source MAC Address (Big Endian)

            uint16_t type;      //The type (Big Endian) 0x0800 = IPv4, 0x0806 = ARP, 0x86DD = IPv6

        }__attribute__ ((packed));

        struct EthernetFrameFooter {
            uint32_t checksum;          //Checksum of the payload
        }__attribute__ ((packed));

        class EthernetFrameHandler;
        class EthernetFramePayloadHandler{
            friend class EthernetFrameHandler;
            protected:
                EthernetFrameHandler* frameHandler;
                uint16_t handledType;

            public:
                EthernetFramePayloadHandler(EthernetFrameHandler* frameHandler, uint16_t handledType);
                ~EthernetFramePayloadHandler();

                virtual bool handleEthernetframePayload(uint8_t* ethernetframePayload, uint32_t size);
                void Send(uint64_t destinationMAC, uint8_t* data, uint32_t size);


        };


        // using map so no need to use the event manager class
        class EthernetFrameHandler : public drivers::ethernet::EthernetDriverEventHandler{
            protected:

            // A map of the handlers and the ethernet frame type they handle
            common::Map<uint16_t , EthernetFramePayloadHandler*> frameHandlers;

            drivers::ethernet::EthernetDriver* ethernetDriver;
            common::OutputStream* errorMessages;

            public:
                EthernetFrameHandler(drivers::ethernet::EthernetDriver* ethernetDriver, common::OutputStream* errorMessages);
                ~EthernetFrameHandler();

                drivers::ethernet::MediaAccessControlAddress getMAC();
                bool DataReceived(uint8_t* data, uint32_t size);
                void connectHandler(EthernetFramePayloadHandler* handler);
                void sendEthernetFrame(uint64_t destinationMAC, uint16_t frameType, uint8_t* data, uint32_t size);


        };

    }

}

#endif //MAXOS_NET_ETHERNETFRAME_H
