//
// Created by 98max on 11/29/2022.
//

#ifndef MAXOS_DRIVERS_ETHERNET_INTEL_I127_H
#define MAXOS_DRIVERS_ETHERNET_INTEL_I127_H

#include <common/types.h>
#include <drivers/driver.h>
#include <hardwarecommunication/pci.h>
#include <hardwarecommunication/interrupts.h>
#include <hardwarecommunication/port.h>
#include <memory/memoryIO.h>
#include <memory/memorymanagement.h>
#include <drivers/ethernet/ethernet.h>

namespace maxOS{

    namespace drivers {

        namespace ethernet{


            class intel_i217 : public EthernetDriver, public hardwarecommunication::InterruptHandler {



                struct receiveDescriptor {
                    common::uint64_t bufferAddress;              // The address of the receive buffer
                    common::uint16_t length;                     // The length of the received frame
                    common::uint16_t checksum;                   // The checksum of the received frame
                    common::uint8_t status;                      // The status of the received frame
                    common::uint8_t errors;                      // Any errors that occurred
                    common::uint16_t special;                    // Special
                } __attribute__((packed));

                struct sendDescriptor {
                    common::uint64_t bufferAddress;             // The address of the send buffer
                    common::uint16_t length;                    // The length of the send frame
                    common::uint8_t cso;                        // The checksum offset
                    common::uint8_t cmd;                        // The command
                    common::uint8_t status;                     // The status
                    common::uint8_t css;                        // The checksum start
                    common::uint16_t special;                   // Special
                } __attribute__((packed));

                common::uint8_t bar_type;                        // The type of base address register
                common::uint16_t portBase;                       // The base address of the port I/O registers
                common::uint64_t memBase;                        // The base address of the memory registers
                common::uint8_t macAddress[6];                   // The MAC address of the device


                //Registers Addresses (Main Registers)
                common::uint16_t controlRegister;                // The control register
                common::uint16_t statusRegister;                 // The status register
                common::uint16_t epromRegister;                  // The address of the eeprom register
                common::uint16_t controlExtRegister;             // The control extension register
                common::uint16_t interruptMaskRegister;          // The interrupt mask register

                //Registers Addresses (Receive Registers)
                common::uint16_t receiveControlRegister;         // The receive control register
                common::uint16_t receiveDescriptorLowRegister;   // The receive descriptor low register
                common::uint16_t receiveDescriptorHighRegister;  // The receive descriptor high register
                common::uint16_t receiveDescriptorLengthRegister;// The receive descriptor length register
                common::uint16_t receiveDescriptorHeadRegister;  // The receive descriptor head register
                common::uint16_t receiveDescriptorTailRegister;  // The receive descriptor tail register

                //Registers Addresses (Send Registers)
                common::uint16_t sendControlRegister;            // The send control register
                common::uint16_t sendDescriptorLowRegister;      // The send descriptor low register
                common::uint16_t sendDescriptorHighRegister;     // The send descriptor high register
                common::uint16_t sendDescriptorLengthRegister;   // The send descriptor length register
                common::uint16_t sendDescriptorHeadRegister;     // The send descriptor head register
                common::uint16_t sendDescriptorTailRegister;     // The send descriptor tail register




                //Buffers
                struct receiveDescriptor *receiveDsrctrs[32];    // The receive descriptors
                common::uint16_t currentReceiveBuffer;           // The current receive buffer

                struct sendDescriptor *sendDsrctrs[8];           // The send descriptors
                common::uint16_t currentSendBuffer;              // The current send buffer


                // Write Commands and read results From NICs either using MemIO or IO Ports
                void Write(common::uint16_t address, common::uint32_t data);
                common::uint32_t Read(common::uint16_t address);

                //EPROM (Device Memory)
                bool epromPresent;                                   // Whether or not the EPROM is present
                bool detectEEProm();                                 // Return true if EEProm exist, else it returns false and set the eerprom_existsdata member
                common::uint32_t eepromRead( common::uint8_t addr);  // Read 4 bytes from a specific EEProm Address


                bool readMACAddress();       // Read MAC Address

                void receiveInit();          // Initialise receive descriptors an buffers
                void sendInit();             // Initialise transmit descriptors an buffers

                //Ethernet Driver functions
                MediaAccessControlAddress ownMAC;                //MAC address of the device
                volatile bool active;                            //Is the device active
                volatile bool initDone;                          //Is the device initialised

                void FetchDataReceived();                        //Fetches the data from the buffer
                void FetchDataSent();                            //Fetches the data from the buffer

            public:

                intel_i217(hardwarecommunication::PeripheralComponentInterconnectDeviceDescriptor* deviceDescriptor, hardwarecommunication::InterruptManager* interruptManager, common::OutputStream* intelNetMessageStream = 0);         // Constructor. takes as a parameter a pointer to an object that encapsulate all he PCI configuration data of the device
                ~intel_i217();                                                  // Default Destructor


                //Override driver default methods
                common::uint32_t reset();
                void activate();
                void deactivate();

                //Override Interrupt default methods
                common::uint32_t HandleInterrupt(common::uint32_t esp);


                //Ethernet Driver functions
                common::string getVendorName();
                common::string getDeviceName();

                void DoSend(common::uint8_t* buffer, common::uint32_t size);
                common::uint64_t GetMediaAccessControlAddress();
                inline void* operator new(common::uint32_t, intel_i217* p)
                {
                    return p;
                }


            };

        }
    }

}


#endif //MAXOS_DRIVERS_ETHERNET_INTEL_I127_H
