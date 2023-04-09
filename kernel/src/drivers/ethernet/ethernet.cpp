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

///__EVENT HANDLER___

EthernetDriverEventHandler::EthernetDriverEventHandler()
{
}

EthernetDriverEventHandler::~EthernetDriverEventHandler()
{
}

bool EthernetDriverEventHandler::DataReceived(uint8_t*, uint32_t)
{
    printf("Not implemented\n");
    return false;
}

void EthernetDriverEventHandler::BeforeSend(uint8_t*, uint32_t)
{
}

void EthernetDriverEventHandler::DataSent(uint8_t*, uint32_t)
{
}



///__ETHERNET DRIVER___

EthernetDriver::EthernetDriver()

{
}

EthernetDriver::~EthernetDriver()
{
}

/**
 * @details Get the device name
 *
 * @return  The device name
 */
string EthernetDriver::GetDeviceName()
{
    return "Ethernet";
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
    printf("Sending: ");

    int displayType = 34;                                                        //What header to hide (Ethernet Header = 14, IP Header = 34, UDP = 42, TCP Header = 54, ARP = 42)
    for(int i = displayType; i < size; i++)
    {
        printfHex(buffer[i]);
        printf(" ");
    }
    printf("\n");


    for(Vector<EthernetDriverEventHandler*>::iterator i = handlers.begin(); i != handlers.end(); i++)
        (*i)->BeforeSend(buffer, size);

    //Used for debuging  printf("Status: ");
    DoSend(buffer, size);
}

/**
 * @details (Device Side) Send the data
 */
void EthernetDriver::DoSend(uint8_t*, uint32_t)
{
    printf("Not implemented\n");
}

/**
 * @details Handle the recieved data
 *
 * @param buffer The buffer to handle
 * @param size The size of the buffer
 */
void EthernetDriver::FireDataReceived(uint8_t* buffer, uint32_t size)
{
    printf("Receiving: ");
    //size = 64;
    int displayType = 34;                                                        //What header to hide (Ethernet Header = 14, IP Header = 34, UDP = 42, TCP Header = 54, ARP = 42)
    for(int i = displayType; i < size; i++)
    {
        printfHex(buffer[i]);
        printf(" ");
    }
    printf("\n");


    bool SendBack = false;

    //Used for debuging printf("Status: ");
    for(Vector<EthernetDriverEventHandler*>::iterator i = handlers.begin(); i != handlers.end(); i++)
        if((*i)->DataReceived(buffer, size))
            SendBack = true;

    if(SendBack){
        printf("Sending back... \n");
        Send(buffer, size);
    }

}

/**
 * @details Send data
 *
 * @param buffer The buffer to send
 * @param size The size of the buffer
 */
void EthernetDriver::FireDataSent(uint8_t* buffer, uint32_t size)
{
    for(Vector<EthernetDriverEventHandler*>::iterator i = handlers.begin(); i != handlers.end(); i++)
        (*i)->DataSent(buffer, size);
}

/**
 * @details Connect the event handler to the base protocol e.g. etherframe
 *
 * @param handler The event handler
 */
void EthernetDriver::ConnectEventHandler(EthernetDriverEventHandler* handler)
{
    handlers.push_back(handler);
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

