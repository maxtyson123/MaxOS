//
// Created by 98max on 12/1/2022.
//

#ifndef MAXOS_DRIVERS_ETHERNET_ETHERNET_H
#define MAXOS_DRIVERS_ETHERNET_ETHERNET_H

#include <stdint.h>
#include <drivers/driver.h>
#include <common/vector.h>
#include <common/eventHandler.h>

namespace maxOS{

    namespace drivers{

        namespace ethernet {

            typedef uint64_t MediaAccessControlAddress;

            enum EthernetDriverEvents{
                BEFORE_SEND,
                DATA_SENT,
                DATA_RECEIVED
            };

            /**
             * @class BeforeSendEvent
             * @brief Event that is triggered before data is sent, holds the buffer and size of the data
             */
            class BeforeSendEvent : public common::Event<EthernetDriverEvents>{
                public:
                    uint8_t* buffer;
                    uint32_t size;
                    BeforeSendEvent(uint8_t* buffer, uint32_t size);
                    ~BeforeSendEvent();
            };

            /**
             * @class DataSentEvent
             * @brief Event that is triggered when data is sent, holds the buffer and size of the data
             */
            class DataSentEvent : public common::Event<EthernetDriverEvents>{
                public:
                    uint8_t* buffer;
                    uint32_t size;
                    DataSentEvent(uint8_t* buffer, uint32_t size);
                    ~DataSentEvent();
            };

            /**
             * @class DataReceivedEvent
             * @brief Event that is triggered when data is received, holds the buffer and size of the data
             */
            class DataReceivedEvent : public common::Event<EthernetDriverEvents>{
                public:
                    uint8_t* buffer;
                    uint32_t size;
                    DataReceivedEvent(uint8_t* buffer, uint32_t size);
                    ~DataReceivedEvent();
            };

            /**
             * @class EthernetDriverEventHandler
             * @brief Handles the events that are triggered by the Ethernet Driver
             */
            class EthernetDriverEventHandler : public common::EventHandler<EthernetDriverEvents>
            {
                public:
                    EthernetDriverEventHandler();
                    ~EthernetDriverEventHandler();

                    virtual common::Event<EthernetDriverEvents>*
                    on_event(common::Event<EthernetDriverEvents>* event);

                    virtual void BeforeSend(uint8_t* buffer, uint32_t size);
                    virtual void DataSent(uint8_t* buffer, uint32_t size);
                    virtual bool DataReceived(uint8_t* buffer, uint32_t size);
            };

            /**
             * @class EthernetDriver
             * @brief Driver for the Ethernet Controller, manages the sending and receiving of data, the mac address, and the events
             */
            class EthernetDriver : public Driver, public common::EventManager<EthernetDriverEvents>
            {
                protected:
                    virtual void DoSend(uint8_t* buffer, uint32_t size);
                    void FireDataReceived(uint8_t* buffer, uint32_t size);
                    void FireDataSent(uint8_t* buffer, uint32_t size);

                public:
                    EthernetDriver(common::OutputStream* ethernetMessageStream);
                    ~EthernetDriver();

                    static MediaAccessControlAddress CreateMediaAccessControlAddress(uint8_t digit1, uint8_t digit2, uint8_t digit3, uint8_t digit4, uint8_t digit5, uint8_t digit6);
                    virtual MediaAccessControlAddress GetMediaAccessControlAddress();

                    void Send(uint8_t* buffer, uint32_t size);
            };

        }
    }
}


#endif //MAXOS_DRIVERS_ETHERNET_ETHERNET_H
