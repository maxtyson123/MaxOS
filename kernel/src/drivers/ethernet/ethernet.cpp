//
// Created by 98max on 12/1/2022.
//

#include <drivers/ethernet/ethernet.h>

using namespace maxOS;
using namespace maxOS::common;
using namespace maxOS::drivers;
using namespace maxOS::drivers::ethernet;

void printf(char* str, bool clearLine = false); // Forward declaration
void printfHex(uint8_t key);                    // Forward declaration

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





EthernetDriver::EthernetDriver()

{
}

EthernetDriver::~EthernetDriver()
{
}


string EthernetDriver::GetDeviceName()
{
    return "Ethernet";
}

MediaAccessControlAddress EthernetDriver::GetMediaAccessControlAddress()
{
    return 0;
}

void EthernetDriver::Send(uint8_t* buffer, uint32_t size)
{
    const char* hex = "0123456789abcdef";
    for(uint8_t* buffit = buffer; buffit < buffer+size; buffit++)
    {
        char* c;
        c[0] = hex[*buffit / 16];
        printf(c);
        c[0] = hex[*buffit % 16];
        printf(c);
        c[0] = ' ';
        printf(c);
    }
    printf("\n");


    for(Vector<EthernetDriverEventHandler*>::iterator i = handlers.begin(); i != handlers.end(); i++)
        (*i)->BeforeSend(buffer, size);
    DoSend(buffer, size);
}

void EthernetDriver::DoSend(uint8_t*, uint32_t)
{
}


void EthernetDriver::FireDataReceived(uint8_t* buffer, uint32_t size)
{
    const char* hex = "0123456789abcdef";
    for(uint8_t* buffit = buffer; buffit < buffer+size; buffit++)
    {
        char* c;
        c[0] = hex[*buffit / 16];
        printf(c);
        c[0] = hex[*buffit % 16];
        printf(c);
        c[0] = ' ';
        printf(c);
    }
    printf("\n");


    bool SendBack = false;

    for(Vector<EthernetDriverEventHandler*>::iterator i = handlers.begin(); i != handlers.end(); i++)
        if((*i)->DataReceived(buffer, size))
            SendBack = true;

    if(SendBack)
        Send(buffer, size);
}

void EthernetDriver::FireDataSent(uint8_t* buffer, uint32_t size)
{
    for(Vector<EthernetDriverEventHandler*>::iterator i = handlers.begin(); i != handlers.end(); i++)
        (*i)->DataSent(buffer, size);
}


void EthernetDriver::ConnectEventHandler(EthernetDriverEventHandler* handler)
{
    handlers.push_back(handler);
}

// if your mac address is e.g. 1c:6f:65:07:ad:1a (see output of ifconfig)
// then you would call CreateMediaAccessControlAddress(0x1c, 0x6f, 0x65, 0x07, 0xad, 0x1a)
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

