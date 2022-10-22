//
// Created by 98max on 22/10/2022.
//

#include <drivers/amd_am79c973.h>

using namespace maxOS;
using namespace maxOS::common;
using namespace maxOS::drivers;

void printf(char* str, bool clearLine = false); //Forward declaration
void printfHex(uint8_t key);                    //Forward declaration



amd_am79c973::amd_am79c973(hardwarecommunication::PeripheralComponentInterconnectDeviceDescriptor* deviceDescriptor, hardwarecommunication::InterruptManager *interruptManager)
:   Driver(),
    InterruptHandler(deviceDescriptor -> interrupt + interruptManager -> HardwareInterruptOffset(), interruptManager),
    MACAddress0Port( deviceDescriptor -> portBase),
    MACAddress2Port(deviceDescriptor -> portBase + 0x02),
    MACAddress4Port(deviceDescriptor -> portBase + 0x04),
    registerDataPort(deviceDescriptor -> portBase + 0x10),
    registerAddressPort(deviceDescriptor -> portBase + 0x12),
    busControlRegisterDataPort(deviceDescriptor -> portBase + 0x14),
    resetPort(deviceDescriptor -> portBase + 0x16)
{

    //No active buffer at the start
    currentSendBuffer = 0;
    currentRecvBuffer = 0;

    //Get the MAC adresses (split up in little endian order)
    uint64_t MAC0 = MACAddress0Port.Read() % 256;
    uint64_t MAC1 = MACAddress0Port.Read() / 256;
    uint64_t MAC2 = MACAddress2Port.Read() % 256;
    uint64_t MAC3 = MACAddress2Port.Read() / 256;
    uint64_t MAC4 = MACAddress4Port.Read() % 256;
    uint64_t MAC5 = MACAddress4Port.Read() / 256;

    //Combine MAC addresses
    uint64_t MAC = MAC5 << 40
                   | MAC4 << 32
                   | MAC3 << 24
                   | MAC2 << 16
                   | MAC1 << 8
                   | MAC0;

    //Set the device to 32 bit mode
    registerAddressPort.Write(20);              //Tell device to write to register 20
    busControlRegisterDataPort.Write(0x102);    //Write desired data

    //Reset the stop bit (tell device it's not supposed to be reset now)
    registerAddressPort.Write(0);               //Tell device to write to register 0
    registerDataPort.Write(0x04);     //Write desired data

    //Set the initialization block
    initBlock.mode = 0x0000;                         //Promiscuous mode = false   ( promiscuous mode tells it to receive all packets, not just broadcasts and those for its own MAC address)
    initBlock.reserved1 = 0;                         //Reserved
    initBlock.numSendBuffers = 3;                    //Means 8 because 2^8 (number of bits used)
    initBlock.reserved2 = 0;                         //Reserved
    initBlock.numRecvBuffers = 3;                    //Means 8 because 2^8 (number of bits used)
    initBlock.physicalAdress = MAC;                  //Set the physical address to the MAC address
    initBlock.reserved3 = 0;                         //Reserverd
    initBlock.logicalAdress = 0;                     //None for now


    //Set Buffer descriptors memory
    sendBufferDescr = (BufferDescriptor*)((((uint32_t)&sendBufferDescrMemory[0]) + 15) & ~((uint32_t)0xF));
    initBlock.sendBufferDescrAddress = (uint32_t)sendBufferDescr;

    recvBufferDescr = (BufferDescriptor*)((((uint32_t)&recvBufferDescrMemory[0]) + 15) & ~((uint32_t)0xF));
    initBlock.recvBufferDescrAddress = (uint32_t)recvBufferDescr;

    //Set Buffer Descriptors themselves
    for (uint8_t i = 0; i < 8; i++){

        //Send
        sendBufferDescr[i].adress = (((uint32_t)&sendBuffers[i]) + 15 ) & ~(uint32_t)0xF;        //Same as above
        sendBufferDescr[i].flags = 0x7FF                                                         //Legnth of descriptor
                                 | 0xF000;                                                       //Set it to send buffer
        sendBufferDescr[i].flags2 = 0;                                                           //No flags 2
        sendBufferDescr[i].avail = 0;                                                            //This flag is just for devs to use so it doesnt need to be set

        //Receive
        recvBufferDescr[i].adress = (((uint32_t)&recvBufferDescr[i]) + 15 ) & ~(uint32_t)0xF;    //Same as above
        recvBufferDescr[i].flags = 0x7FF                                                         //Legnth of descriptor
                                   | 0x80000000;                                                 //Set it to receive buffer
        recvBufferDescr[i].flags2 = 0;                                                           //No flags 2
        recvBufferDescr[i].avail = 0;                                                            //This flag is just for devs to use so it doesnt need to be set

    }

    //Move initialization block into device
    registerAddressPort.Write(1);                                     //Tell device to write to register 1
    registerDataPort.Write( (uint32_t)(&initBlock) & 0xFFFF );             //Write address data
    registerAddressPort.Write(1);                                     //Tell device to write to register 2
    registerDataPort.Write( ((uint32_t)(&initBlock) >> 16) & 0xFFFF );     //Write shifted address data

}

amd_am79c973::~amd_am79c973() {

}

void amd_am79c973::Activate()
{

    registerAddressPort.Write(0);                           //Tell device to write to register 0
    registerDataPort.Write(0x41);                           //Enables interrupts

    registerAddressPort.Write(4);                           //Tell device to read from register 4
    uint32_t temp = registerDataPort.Read();                     //Get current data

    registerAddressPort.Write(4);                           //Tell device to write to register 4
    registerDataPort.Write(temp | 0xC00);                   //Bitwise OR function on data (This automatically enlarges packets smaller than 64 bytes to that size and removes some relatively superfluous information from received packets.)

    registerAddressPort.Write(0);                           //Tell device to write to register 0
    registerDataPort.Write(0x42);                           //Tell device that it is initialized and can begin operating


}

int amd_am79c973::Reset() {

    resetPort.Read();
    resetPort.Write(0);
    return 10;                      //10 means wait for 10ms

}

common::uint32_t amd_am79c973::HandleInterrupt(common::uint32_t esp) {
    printf("INTERRUPT FROM AMD (am79c973)\n");

    //Similar to PIC, data needs to be read when a interrupt is sent, or it hangs
    registerAddressPort.Write(0);                           //Tell device to read from register 0
    uint32_t temp = registerDataPort.Read();                     //Get current data

    //Note: Cant be switch case as multiple errors can occur at the same time

    //Errors
    if((temp & 0x8000) == 0x8000) printf("AMD am79c973 ERROR\n");
    if((temp & 0x2000) == 0x2000) printf("AMD am79c973 COLLISION ERROR\n");
    if((temp & 0x1000) == 0x1000) printf("AMD am79c973 MISSED FRAME\n");
    if((temp & 0x0800) == 0x0800) printf("AMD am79c973 MEMORY ERROR\n");

    //Responses
    if((temp & 0x0400) == 0x0400) printf("AMD am79c973 DATA RECEVED\n");
    if((temp & 0x0200) == 0x0200) printf("AMD am79c973 DATA SENT\n");
    if((temp & 0x0100) == 0x0100) printf("AMD am79c973 INIT DONE\n");

    //Reply that it was received
    registerAddressPort.Write(0);                           //Tell device to write to register 0
    registerDataPort.Write(temp);                           //Tell device that the interrupt was received

    return esp;

}
