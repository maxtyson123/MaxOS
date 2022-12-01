//
// Created by 98max on 12/1/2022.
//

#ifndef MAXOS_DRIVERS_ETHERNET_ETHERNET_H
#define MAXOS_DRIVERS_ETHERNET_ETHERNET_H

#include <common/types.h>
#include <drivers/driver.h>
#include <common/vector.h>

namespace maxOS{

    namespace drivers{

        namespace ethernet {

            typedef common::uint64_t MediaAccessControlAddress;

            class EthernetDriverEventHandler
            {
            public:
                EthernetDriverEventHandler();
                ~EthernetDriverEventHandler();

                virtual void BeforeSend(common::uint8_t* buffer, common::uint32_t size);
                virtual void DataSent(common::uint8_t* buffer, common::uint32_t size);
                virtual bool DataReceived(common::uint8_t* buffer, common::uint32_t size);
            };

            class EthernetDriver : public Driver
            {
            protected:
                common::Vector<EthernetDriverEventHandler*> handlers;
                virtual void DoSend(common::uint8_t* buffer, common::uint32_t size);
                void FireDataReceived(common::uint8_t* buffer, common::uint32_t size);
                void FireDataSent(common::uint8_t* buffer, common::uint32_t size);

            public:
                EthernetDriver();
                ~EthernetDriver();

                common::string GetDeviceName();
                static MediaAccessControlAddress CreateMediaAccessControlAddress(common::uint8_t digit1, common::uint8_t digit2, common::uint8_t digit3, common::uint8_t digit4, common::uint8_t digit5, common::uint8_t digit6);
                virtual MediaAccessControlAddress GetMediaAccessControlAddress();

                void Send(common::uint8_t* buffer, common::uint32_t size);
                void ConnectEventHandler(EthernetDriverEventHandler* handler);
            };

        }
    }
}


#endif //MAXOS_DRIVERS_ETHERNET_ETHERNET_H
