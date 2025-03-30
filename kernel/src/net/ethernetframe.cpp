//
// Created by 98max on 5/11/2022.
//

#include <net/ethernetframe.h>

using namespace MaxOS;
using namespace MaxOS::common;
using namespace MaxOS::net;
using namespace MaxOS::drivers;
using namespace MaxOS::drivers::ethernet;
using namespace MaxOS::memory;

/**
 * @param frameHandler the handler for the ethernet frame
 * @param etherType the type of the protocol, which will be handled by this handler
 */
EthernetFramePayloadHandler::EthernetFramePayloadHandler(EthernetFrameHandler* frameHandler, uint16_t handledType) {

    this -> handledType = handledType;
    this -> frameHandler = frameHandler;
    frameHandler->connectHandler(this);


}

/**
 * @brief Destroy the EtherFrameHandler:: EtherFrameHandler object, Removes it from the handler list
 *
 */
EthernetFramePayloadHandler::~EthernetFramePayloadHandler() = default;


bool EthernetFramePayloadHandler::handleEthernetframePayload(uint8_t*, uint32_t) {

    //By default, don't handle it, will be handled in the override
    return false;

}

/**
 * @brief Send an packet via the backend driver
 *
 * @param dstMAC the destination MAC address
 * @param data the data to send
 * @param size the size of the payload
 */
void EthernetFramePayloadHandler::Send(uint64_t destination, uint8_t *data, uint32_t size) {

    frameHandler -> sendEthernetFrame (destination, handledType, data, size);
}


EthernetFrameHandler::EthernetFrameHandler(EthernetDriver* driver, OutputStream* errorMessages)
: EthernetDriverEventHandler()
{

    this -> ethernetDriver = driver;
    this -> errorMessages = errorMessages;

    driver->connect_event_handler(this);

}

EthernetFrameHandler::~EthernetFrameHandler() = default;

drivers::ethernet::MediaAccessControlAddress EthernetFrameHandler::getMAC() {
    return ethernetDriver -> GetMediaAccessControlAddress();
}


/**
 * @brief Handle the received packet
 *
 * @param buffer the buffer with the received data
 * @param size the size of the received data
 */
bool EthernetFrameHandler::DataReceived(uint8_t* buffer, uint32_t size) {

    errorMessages -> write("EFH: Data received\n");


    //Check if the size is big enough to contain an ethernet frame
    if(size < sizeof(EthernetFrameHeader))
        return false;

    //Convert to struct for easier use
    auto* frame = (EthernetFrameHeader*)buffer;
    bool sendBack = false;

    //Only handle if it is for this device
    if(frame->destinationMAC == 0xFFFFFFFFFFFF                                          //If it is a broadcast
    || frame->destinationMAC == ethernetDriver -> GetMediaAccessControlAddress())      //If it is for this device
    {

        // Find the handler for the protocol
        Map<uint16_t , EthernetFramePayloadHandler*>::iterator handlerIterator = frameHandlers.find(frame->type);

        // If the handler is found
        if(handlerIterator != frameHandlers.end()) {


            //TODO: The override is not being called (IDK WHY)

            //Handle the data
            errorMessages -> write("EFH: Handling ethernet frame payload\n");
            sendBack = handlerIterator->second->handleEthernetframePayload(buffer + sizeof(EthernetFrameHeader), size - sizeof(EthernetFrameHeader));
            errorMessages -> write("..DONE\n");

        } else {

            //If the handler is not found, print an error message
            errorMessages -> write("EFH: Unhandled ethernet frame type 0x");
            errorMessages->write_hex(frame->type);
            errorMessages -> write("\n");

        }
    }

    //If the data is to be sent back again
    if(sendBack){

        errorMessages -> write("EFH: Sending back\n");

        frame -> destinationMAC = frame -> sourceMAC;                             //Set the new destination to be the device the data was received from
        frame -> sourceMAC = ethernetDriver->GetMediaAccessControlAddress();      //Set the new source to be this device's MAC address

    }

    //Return if the data is to be sent back
    return sendBack;

}

void EthernetFrameHandler::connectHandler(EthernetFramePayloadHandler *handler) {

    // Convert the protocol type to big endian
    uint16_t frameType_BE = ((handler->handledType >> 8) & 0xFF) | ((handler->handledType << 8) & 0xFF00);

    // Add the handler to the list
    frameHandlers.insert(frameType_BE, handler);

}

/**
 * @brief Send an packet via the backend driver
 *
 * @param dstMAC_BE the destination MAC address
 * @param etherType_BE the type of the protocol
 * @param buffer the data to send
 * @param size the size of the payload
 */
void EthernetFrameHandler::sendEthernetFrame(uint64_t destinationMAC, uint16_t frameType, uint8_t* data, uint32_t size) {

    errorMessages->write("EFH: Sending frame...");

    //Allocate memory for the buffer
    auto* buffer = (uint8_t*)MemoryManager::kmalloc(size + sizeof(EthernetFrameHeader));
    auto* frame = (EthernetFrameHeader*)buffer;

    //Put data in the header
    frame -> destinationMAC = destinationMAC;
    frame -> sourceMAC = ethernetDriver -> GetMediaAccessControlAddress();
    frame -> type = (frameType >> 8) | (frameType << 8);                        //Convert to big endian

    //Copy the data
    for(uint8_t *src = data + size - 1, *dst = buffer+sizeof(EthernetFrameHeader)+size-1; src >= data; --src, --dst)
        *dst = *src;

    //Send the data
    ethernetDriver -> Send(buffer, size + sizeof(EthernetFrameHeader));

    errorMessages->write("Done\n");


    //Free the buffer
    MemoryManager::kfree(buffer);
}
