//
// Created by 98max on 22/10/2022.
//

#ifndef MAXOS_DRIVERS_ETHERNET_AMD_AM79C973_H
#define MAXOS_DRIVERS_ETHERNET_AMD_AM79C973_H

#include <stdint.h>
#include <drivers/ethernet/ethernet.h>
#include <hardwarecommunication/pci.h>
#include <hardwarecommunication/interrupts.h>
#include <hardwarecommunication/port.h>


namespace MaxOS{

    namespace drivers{

        namespace ethernet{

            class AMD_AM79C973;

            /**
             * @class AMD AM79C973
             * @brief Driver for the AMD AM79C973 Ethernet Controller
             */
            class AMD_AM79C973 : public EthernetDriver, public hardwarecommunication::InterruptHandler{

                struct InitializationBlock{

                    uint16_t mode;
                    unsigned reserved1 : 4;
                    unsigned numSendBuffers : 4;
                    unsigned reserved2 : 4;
                    unsigned numRecvBuffers : 4;
                    uint64_t physicalAddress : 48;   //Not 64 bits but will be treated like it is
                    uint16_t reserved3;
                    uint64_t logicalAddress;
                    uint32_t recvBufferDescrAddress;
                    uint32_t sendBufferDescrAddress;


                } __attribute__((packed));

                struct BufferDescriptor{

                    uint64_t address;
                    uint32_t flags;
                    uint32_t flags2;
                    uint32_t avail;

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
                InitializationBlock initBlock{};


                BufferDescriptor* sendBufferDescr;               //Descriptor entry
                uint8_t sendBuffers[2*1024+15][8]{};       //8 Send Buffers, 2KB + 15 bytes
                uint8_t currentSendBuffer;               //Which buffers are active

                BufferDescriptor* recvBufferDescr;               //Descriptor entry
                uint8_t recvBuffers[2*1024+15][8]{};       //8 Send Buffers, 2KB + 15 bytes
                uint8_t currentRecvBuffer;               //Which buffers are active

                //Ethernet Driver functions
                MediaAccessControlAddress ownMAC;                //MAC address of the device
                volatile bool active;                            //Is the device active
                volatile bool initDone;                          //Is the device initialised

                void FetchDataReceived();                        //Fetches the data from the buffer
                void FetchDataSent();                            //Fetches the data from the buffer

            public:
                AMD_AM79C973(hardwarecommunication::PeripheralComponentInterconnectDeviceDescriptor* device_descriptor);
                ~AMD_AM79C973();

                //Override driver default methods
                uint32_t reset() final;
                void activate() final;
                void deactivate() final;

                // Naming
                string vendor_name() final;
                string device_name() final;

                //Override Interrupt default methods
                void handle_interrupt() final;

                //Ethernet Driver functions
                void DoSend(uint8_t* buffer, uint32_t size) final;
                uint64_t GetMediaAccessControlAddress() final;
            };


        }
    }

}

#endif //MAXOS_DRIVERS_ETHERNET_AMD_AM79C973_H