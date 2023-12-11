//
// Created by 98max on 12/1/2022.
//

#include <drivers/ethernet/ethernet.h>

using namespace maxOS;
using namespace maxOS::common;
using namespace maxOS::drivers;
using namespace maxOS::drivers::ethernet;

///__EVENT HANDLER___

EthernetDriverEventHandler::EthernetDriverEventHandler()
{
}

EthernetDriverEventHandler::~EthernetDriverEventHandler()
{
}



bool EthernetDriverEventHandler::DataReceived(uint8_t*, uint32_t)
{
    return false;
}

void EthernetDriverEventHandler::BeforeSend(uint8_t*, uint32_t)
{
}

void EthernetDriverEventHandler::DataSent(uint8_t*, uint32_t)
{
}

Event<EthernetDriverEvents>* EthernetDriverEventHandler::onEvent(Event<EthernetDriverEvents> *event) {

    switch (event -> type) {

        case EthernetDriverEvents::BEFORE_SEND:
            BeforeSend(((BeforeSendEvent*)event) -> buffer, ((BeforeSendEvent*)event) -> size);
            break;

        case EthernetDriverEvents::DATA_SENT:
            DataSent(((DataSentEvent*)event) -> buffer, ((DataSentEvent*)event) -> size);
            break;

        case EthernetDriverEvents::DATA_RECEIVED:
            event->returnValue.boolValue = DataReceived(((DataReceivedEvent*)event) -> buffer, ((DataReceivedEvent*)event) -> size);
            break;

        default:
            break;
    }

    return event;
}



///__ETHERNET DRIVER___

EthernetDriver::EthernetDriver(OutputStream* ethernetMessageStream)
: Driver(ethernetMessageStream)
{
}

EthernetDriver::~EthernetDriver()
{
}

/**
 * @details Get the MAC address
 *
 * @return the MAC address
 */
MediaAccessControlAddress EthernetDriver::GetMediaAccessControlAddress()
{
    return 0;
}

/**
 * @details Send data to the network via the driver backend
 *
 * @param buffer  The buffer to send
 * @param size The size of the buffer
 */
void EthernetDriver::Send(uint8_t* buffer, uint32_t size)
{
    driverMessageStream -> write("Sending: ");

    int displayType = 34;                                                        //What header to hide (Ethernet Header = 14, IP Header = 34, UDP = 42, TCP Header = 54, ARP = 42)
    for(int i = displayType; i < size; i++)
    {
        driverMessageStream -> writeHex(buffer[i]);
        driverMessageStream -> write(" ");
    }
    driverMessageStream -> write("\n");

    // Raise the event
    raiseEvent(new BeforeSendEvent(buffer, size));

    DoSend(buffer, size);
}

/**
 * @details (Device Side) Send the data
 */
void EthernetDriver::DoSend(uint8_t*, uint32_t)
{
}

/**
 * @details Handle the recieved data
 *
 * @param buffer The buffer to handle
 * @param size The size of the buffer
 */
void EthernetDriver::FireDataReceived(uint8_t* buffer, uint32_t size)
{
    driverMessageStream -> write("Receiving: ");
    //size = 64;
    int displayType = 34;                                                        //What header to hide (Ethernet Header = 14, IP Header = 34, UDP = 42, TCP Header = 54, ARP = 42)
    for(int i = displayType; i < size; i++)
    {
        driverMessageStream -> writeHex(buffer[i]);
        driverMessageStream -> write(" ");
    }
    driverMessageStream -> write("\n");

    // Raise the event
    Vector<Event<EthernetDriverEvents>*> values = raiseEvent(new DataReceivedEvent(buffer, size));

    // Loop through the events
    for(typename Vector<Event<EthernetDriverEvents>*>::iterator event = values.begin(); event != values.end(); ++event) {
        switch ((*event)->type) {
            case EthernetDriverEvents::DATA_RECEIVED:
                if((*event)->returnValue.boolValue){
                    driverMessageStream -> write("Sending back... \n");
                    Send(buffer, size);
                }
                break;

            default:
                break;
        }
    }
    driverMessageStream -> write("DATA HANDLED\n");
}

/**
 * @details Send data
 *
 * @param buffer The buffer to send
 * @param size The size of the buffer
 */
void EthernetDriver::FireDataSent(uint8_t* buffer, uint32_t size)
{
    raiseEvent(new DataSentEvent(buffer, size));
}

// if your mac address is e.g. 1c:6f:65:07:ad:1a (see output of ifconfig)
// then you would call CreateMediaAccessControlAddress(0x1c, 0x6f, 0x65, 0x07, 0xad, 0x1a)
/**
 * @details Create a Media Access Control Address
 *
 * @param digit1 The first digit
 * @param digit2 The second digit
 * @param digit3 The third digit
 * @param digit4 The fourth digit
 * @param digit5 The fifth digit
 * @param digit6 The last digit
 * @return The MAC address
 */
MediaAccessControlAddress EthernetDriver::CreateMediaAccessControlAddress(uint8_t digit1, uint8_t digit2, uint8_t digit3, uint8_t digit4, uint8_t digit5, uint8_t digit6)
{
    return // digit6 is the most significant byte
            (uint64_t)digit6 << 40
            | (uint64_t)digit5 << 32
            | (uint64_t)digit4 << 24
            | (uint64_t)digit3 << 16
            | (uint64_t)digit2 << 8
            | (uint64_t)digit1;
}

/// __ EVENTS __

DataSentEvent::DataSentEvent(uint8_t *buffer, uint32_t size)
: Event(EthernetDriverEvents::DATA_SENT)
{
    this -> buffer = buffer;
    this -> size = size;
    this -> size = size;
}

DataSentEvent::~DataSentEvent()
{
}

DataReceivedEvent::DataReceivedEvent(uint8_t *buffer, uint32_t size)
: Event(EthernetDriverEvents::DATA_RECEIVED)
{
    this -> buffer = buffer;
    this -> size = size;
}

DataReceivedEvent::~DataReceivedEvent()
{
}

BeforeSendEvent::BeforeSendEvent(uint8_t *buffer, uint32_t size)
: Event(EthernetDriverEvents::BEFORE_SEND)
{
    this -> buffer = buffer;
    this -> size = size;
}

BeforeSendEvent::~BeforeSendEvent()
{
}