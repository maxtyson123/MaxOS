//
// Created by 98max on 5/11/2022.
//

#include <net/etherframe.h>

using namespace maxOS;
using namespace maxOS::common;
using namespace maxOS::net;
using namespace maxOS::drivers;

EtherFrameHandler::EtherFrameHandler(EtherFrameProvider *backend, common::uint16_t etherType) {

    //Convert into Big Endian
    this -> etherType_BE = ((etherType & 0x00FF) << 8)  //Shift last bytes to front
                         | ((etherType & 0xFF00) >> 8); //Shift front bytes to back

    //Register this in the Ether Frame Provider
    this -> backend = backend;
    backend -> handlers[etherType_BE] = this;

}

EtherFrameHandler::~EtherFrameHandler() {

    //Remove this from the Ether Frame Provider
    backend -> handlers[etherType_BE] = 0;

}

bool EtherFrameHandler::OnEtherFrameReceived(common::uint8_t *etherframePayload, common::uint32_t size) {

    //By default, don't handle it, will be handled in the override
    return false;

}

void EtherFrameHandler::Send(common::uint64_t dstMAC_BE, common::uint8_t *data, common::uint32_t size) {

    backend -> Send(dstMAC_BE, etherType_BE, data, size);
}



EtherFrameProvider::EtherFrameProvider(drivers::amd_am79c973 *backend)
: RawDataHandler(backend)
{

    //Clear handlers on start
    for (uint32_t i = 0; i < 65535; ++i) {
        handlers[i] = 0;
    }

}

EtherFrameProvider::~EtherFrameProvider() {

}

bool EtherFrameProvider::OnRawDataReceived(common::uint8_t* buffer, common::uint32_t size) {

    //Convert to struct for easier use
    EtherFrameHeader* frame = (EtherFrameHeader*)buffer;
    bool sendBack = false;

    //Only handle if it is for this device
    if(frame->dstMAC_BE == 0xFFFFFFFFFFFF                   //If it is a broadcast
    || frame->dstMAC_BE == backend -> GetMACAddress())      //If it is for this device
    {

        //Check if there is a handler for this frame type
        if(handlers[frame -> etherType_BE] != 0){
            sendBack = handlers[frame -> etherType_BE] -> OnEtherFrameReceived(buffer + sizeof(EtherFrameHeader), size - sizeof(EtherFrameHeader));
            //Note: We don't have to remove the size of the checksum because it was removed in amd_am79c973.cpp :
            //void amd_am79c973::Receive() { .. .. if(size > 64) // remove checksum size -= 4; .. .. }

        }

    }

    //If the data is to be sent back again
    if(sendBack){

        frame -> dstMAC_BE = frame -> srcMAC_BE;            //Set the new destination to be the device the data was received from
        frame -> srcMAC_BE = backend->GetMACAddress();      //Set the new source to be this device's MAC address

    }

    return sendBack;

}

void EtherFrameProvider::Send(common::uint64_t dstMAC_BE, common::uint16_t etherType_BE, common::uint8_t *buffer, common::uint32_t size) {


    uint8_t* buffer2 = (uint8_t*)MemoryManager::activeMemoryManager -> malloc(sizeof(EtherFrameHeader) + size);      //Allocate memory for a buffer
    EtherFrameHeader* frame = (EtherFrameHeader*)buffer2;                                                                //Convert buffer into a EtherFrame

    //Put data in the header
    frame -> dstMAC_BE = dstMAC_BE;
    frame -> srcMAC_BE = backend -> GetMACAddress();
    frame -> etherType_BE = etherType_BE;

    //Set source and destination to their buffers
    uint8_t* src = buffer;
    uint8_t* dst = buffer2 + sizeof(EtherFrameHeader);

    //Move the source to the destination
    for(uint32_t i = 0; i < size; i++)
        dst[i] = src[i];

    //Send via backend
    backend -> Send(buffer2, size + sizeof(EtherFrameHeader));
}
