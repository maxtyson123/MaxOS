//
// Created by 98max on 12/1/2022.
//

#ifndef MAXOS_DRIVERS_ETHERNET_ETHERNET_H
#define MAXOS_DRIVERS_ETHERNET_ETHERNET_H

#include <common/types.h>
#include <drivers/driver.h>
#include <common/vector.h>
#include <common/eventHandler.h>

namespace maxOS{

    namespace drivers{

        namespace ethernet {

            typedef common::uint64_t MediaAccessControlAddress;

            enum EthernetDriverEvents{
                BEFORE_SEND,
                DATA_SENT,
                DATA_RECEIVED
            };


            class BeforeSendEvent : public common::Event<EthernetDriverEvents>{
                public:
                    common::uint8_t* buffer;
                    common::uint32_t size;
                    BeforeSendEvent(common::uint8_t* buffer, common::uint32_t size);
                    ~BeforeSendEvent();
            };

            class DataSentEvent : public common::Event<EthernetDriverEvents>{
                public:
                    common::uint8_t* buffer;
                    common::uint32_t size;
                    DataSentEvent(common::uint8_t* buffer, common::uint32_t size);
                    ~DataSentEvent();
            };

            class DataReceivedEvent : public common::Event<EthernetDriverEvents>{
                public:
                    common::uint8_t* buffer;
                    common::uint32_t size;
                    DataReceivedEvent(common::uint8_t* buffer, common::uint32_t size);
                    ~DataReceivedEvent();
            };

            class EthernetDriverEventHandler : public common::EventHandler<EthernetDriverEvents>
            {
                public:
                    EthernetDriverEventHandler();
                    ~EthernetDriverEventHandler();

                    virtual common::Event<EthernetDriverEvents>* onEvent(common::Event<EthernetDriverEvents>* event);

                    virtual void BeforeSend(common::uint8_t* buffer, common::uint32_t size);
                    virtual void DataSent(common::uint8_t* buffer, common::uint32_t size);
                    virtual bool DataReceived(common::uint8_t* buffer, common::uint32_t size);
            };

        class EthernetDriver : public Driver, public common::EventManager<EthernetDriverEvents>
            {
                protected:
                    virtual void DoSend(common::uint8_t* buffer, common::uint32_t size);
                    void FireDataReceived(common::uint8_t* buffer, common::uint32_t size);
                    void FireDataSent(common::uint8_t* buffer, common::uint32_t size);

                public:
                    EthernetDriver(common::OutputStream* ethernetMessageStream);
                    ~EthernetDriver();

                    static MediaAccessControlAddress CreateMediaAccessControlAddress(common::uint8_t digit1, common::uint8_t digit2, common::uint8_t digit3, common::uint8_t digit4, common::uint8_t digit5, common::uint8_t digit6);
                    virtual MediaAccessControlAddress GetMediaAccessControlAddress();

                    void Send(common::uint8_t* buffer, common::uint32_t size);
            };

        }
    }
}


#endif //MAXOS_DRIVERS_ETHERNET_ETHERNET_H
