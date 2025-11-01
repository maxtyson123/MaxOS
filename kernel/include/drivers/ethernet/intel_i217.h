/**
 * @file intel_i217.h
 * @brief Driver for the Intel I217 Ethernet Controller
 *
 * @date 29th November 2022
 * @author Max Tyson
 */

#ifndef MAXOS_DRIVERS_ETHERNET_INTEL_I127_H
#define MAXOS_DRIVERS_ETHERNET_INTEL_I127_H

#include <stddef.h>
#include <common/outputStream.h>
#include <common/macros.h>
#include <drivers/driver.h>
#include <hardwarecommunication/pci.h>
#include <hardwarecommunication/interrupts.h>
#include <hardwarecommunication/port.h>
#include <memory/memoryIO.h>
#include <memory/memorymanagement.h>
#include <drivers/ethernet/ethernet.h>

namespace MaxOS{

    namespace drivers {

        namespace ethernet{

			/**
			 * @struct ReceiveDescriptor
			 * @brief The receive descriptor for the Intel I217 Ethernet Controller
			 *
			 * @typedef receive_descriptor_t
			 * @brief Alias for ReceiveDescriptor struct
			 */
	        typedef struct PACKED ReceiveDescriptor {
		        uint64_t bufferAddress;              ///< The address of the receive buffer
		        uint16_t length;                     ///< The length of the received frame
		        uint16_t checksum;                   ///< The checksum of the received frame
		        uint8_t status;                      ///< The status of the received frame
		        uint8_t errors;                      ///< Any errors that occurred
		        uint16_t special;                    ///< Special
	        } receive_descriptor_t;

			/**
			 * @struct SendDescriptor
			 * @brief The send descriptor for the Intel I217 Ethernet Controller
			 *
			 * @typedef send_descriptor_t
			 * @brief Alias for SendDescriptor struct
			 */
	        typedef struct PACKED SendDescriptor {
		        uint64_t bufferAddress;             ///< The address of the send buffer
		        uint16_t length;                    ///< The length of the send frame
		        uint8_t cso;                        ///< The checksum offset
		        uint8_t cmd;                        ///< The command
		        uint8_t status;                     ///< The status
		        uint8_t css;                        ///< The checksum start
		        uint16_t special;                   ///< Special
	        } send_descriptor_t;


            /**
             * @class IntelI217
             * @brief Driver for the Intel I217 Ethernet Controller
             */
            class IntelI217 : public EthernetDriver, public hardwarecommunication::InterruptHandler {

                uint8_t bar_type = { 0 };
                uint16_t portBase = { 0 };
                uint64_t memBase = { 0 };
                uint8_t macAddress[6] = { 0 };


                //Registers Addresses (Main Registers)
                uint16_t controlRegister;                // The control register
                uint16_t statusRegister;                 // The status register
                uint16_t epromRegister;                  // The address of the eeprom register
                uint16_t controlExtRegister;             // The control extension register
                uint16_t interruptMaskRegister;          // The interrupt mask register

                //Registers Addresses (Receive Registers)
                uint16_t receiveControlRegister;         // The receive control register
                uint16_t receiveDescriptorLowRegister;   // The receive descriptor low register
                uint16_t receiveDescriptorHighRegister;  // The receive descriptor high register
                uint16_t receiveDescriptorLengthRegister;// The receive descriptor length register
                uint16_t receiveDescriptorHeadRegister;  // The receive descriptor head register
                uint16_t receiveDescriptorTailRegister;  // The receive descriptor tail register

                //Registers Addresses (Send Registers)
                uint16_t sendControlRegister;            // The send control register
                uint16_t sendDescriptorLowRegister;      // The send descriptor low register
                uint16_t sendDescriptorHighRegister;     // The send descriptor high register
                uint16_t sendDescriptorLengthRegister;   // The send descriptor length register
                uint16_t sendDescriptorHeadRegister;     // The send descriptor head register
                uint16_t sendDescriptorTailRegister;     // The send descriptor tail register




                //Buffers
                struct receiveDescriptor *receiveDsrctrs[32];    // The receive descriptors
                uint16_t currentReceiveBuffer;           // The current receive buffer

                struct sendDescriptor *sendDsrctrs[8];           // The send descriptors
                uint16_t currentSendBuffer;              // The current send buffer


                // write Commands and read results From NICs either using MemIO or IO Ports
                void Write(uint16_t address, uint32_t data) const;
                uint32_t Read(uint16_t address) const;

                //EPROM (Device Memory)
                bool epromPresent;                                   // Whether the EPROM is present
                bool detectEEProm();                                 // Return true if EEProm exist, else it returns false and set the error_exists data member
                uint32_t eepromRead( uint8_t addr);  // read 4 bytes from a specific EEProm Address


                bool readMACAddress();       // read MAC Address

                void receiveInit();          // Initialise receive descriptors buffers
                void sendInit();             // Initialise transmit descriptors buffers

                //Ethernet Driver functions
                MediaAccessControlAddress ownMAC;                //MAC address of the device
                volatile bool active;                            //Is the device active
                volatile bool initDone;                          //Is the device initialised

                void FetchDataReceived();                        //Fetches the data from the buffer

            public:

	            IntelI217(hardwarecommunication::PeripheralComponentInterconnectDeviceDescriptor* deviceDescriptor);
                ~IntelI217();                                                  // Default Destructor


                //Override driver default methods
                uint32_t reset() final;
                void activate() final;
                void deactivate() final;

                //Override Interrupt default methods
                void handle_interrupt() final;


                //Ethernet Driver functions
                string vendor_name() final;
                string device_name() final;

                void DoSend(uint8_t* buffer, uint32_t size) final;
                uint64_t GetMediaAccessControlAddress() final;
            };

        }
    }

}


#endif //MAXOS_DRIVERS_ETHERNET_INTEL_I127_H
