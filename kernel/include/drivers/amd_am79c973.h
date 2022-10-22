//
// Created by 98max on 22/10/2022.
//

#ifndef MAXOS_DRIVERS_AMD_AM79C973_H
#define MAXOS_DRIVERS_AMD_AM79C973_H

#include <common/types.h>
#include <drivers/driver.h>
#include <hardwarecommunication/pci.h>
#include <hardwarecommunication/interrupts.h>
#include <hardwarecommunication/port.h>


namespace maxOS{

    namespace drivers{

        class amd_am79c973 : public Driver, public hardwarecommunication::InterruptHandler{

            struct InitializationBlock{

                common::uint16_t mode;
                unsigned reserved1 : 4;
                unsigned numSendBuffers : 4;
                unsigned reserved2 : 4;
                unsigned numRecvBuffers : 4;
                common::uint64_t physicalAdress : 48;   //Not 64 bits but will be treated like it is
                common::uint16_t reserved3;
                common::uint64_t logicalAdress;         //Potential For IP adress
                common::uint32_t recvBufferDescrAddress;
                common::uint32_t sendBufferDescrAddress;


            } __attribute__((packed));

            struct BufferDescriptor{

                common::uint32_t adress;
                common::uint32_t flags;
                common::uint32_t flags2;
                common::uint32_t avail;

            } __attribute__((packed));


            //Reading the media access control address (MAC address)
            hardwarecommunication::Port16Bit MACAddress0Port;
            hardwarecommunication::Port16Bit MACAddress2Port;
            hardwarecommunication::Port16Bit MACAddress4Port;

            //Register ports
            hardwarecommunication::Port16Bit registerDataPort;
            hardwarecommunication::Port16Bit registerAddressPort;
            hardwarecommunication::Port16Bit busControlRegisterDataPort;

            hardwarecommunication::Port16Bit resetPort;

            //The main purpose of the initialization block it to hold a pointer to the array of BufferDescriptors, which hold the pointers to the buffers
            InitializationBlock initBlock;


            BufferDescriptor* sendBufferDescr;               //Descriptor entry
            common::uint8_t sendBufferDescrMemory[2048+15];  //RAM for the send buffer, also 16 byte aligned
            common::uint8_t sendBuffers[2*1024+15][8];       //8 Send Buffers, 2KB + 15 bytes
            common::uint8_t currentSendBuffer;               //Which buffers are active

            BufferDescriptor* recvBufferDescr;               //Descriptor entry
            common::uint8_t recvBufferDescrMemory[2048+15];  //RAM for the recive buffer, also 16 byte aligned
            common::uint8_t recvBuffers[2*1024+15][8];       //8 Send Buffers, 2KB + 15 bytes
            common::uint8_t currentRecvBuffer;               //Which buffers are active


            public:
                amd_am79c973(hardwarecommunication::PeripheralComponentInterconnectDeviceDescriptor* deviceDescriptor, hardwarecommunication::InterruptManager* interruptManager);
                ~amd_am79c973();

                //Override driver default methods
                void Activate();
                int Reset();

                //Override Interrupt default methods
                common::uint32_t HandleInterrupt(common::uint32_t esp);



        };


    }

}

#endif //MAXOS_DRIVERS_AMD_AM79C973_H
