/**
 * @file amd_am79c973.h
 * @brief Driver for the AMD AM79C973 Ethernet Controller
 *
 * @date 22nd October 2022
 * @author Max Tyson
 */

#ifndef MAXOS_DRIVERS_ETHERNET_AMD_AM79C973_H
#define MAXOS_DRIVERS_ETHERNET_AMD_AM79C973_H

#include <stdint.h>
#include <common/macros.h>
#include <drivers/ethernet/ethernet.h>
#include <hardwarecommunication/pci.h>
#include <hardwarecommunication/interrupts.h>
#include <hardwarecommunication/port.h>

namespace MaxOS {

	namespace drivers {

		namespace ethernet {

			/**
			 * @struct InitializationBlock
			 * @brief The initialization block for the AMD AM79C973 Ethernet Controller
			 *
			 * @typedef initialisation_block_t
			 * @brief Alias for InitializationBlock struct
			 */
			typedef struct PACKED InitializationBlock {

					uint16_t mode;                      ///< The operation mode
					unsigned reserved1: 4;                ///< Unused, must be zero
					unsigned numSendBuffers: 4;            ///< How many buffers are used for sending
					unsigned reserved2: 4;                ///< Unused, must be zero
					unsigned numRecvBuffers: 4;            ///< How many buffers are used for receiving
					uint64_t physicalAddress: 48;        ///< The physical (MAC) address of the device (Not 64 bits but will be treated like it is)
					uint16_t reserved3;                 ///< Unused, must be zero
					uint64_t logicalAddress;            ///< The logical address filter for the device to use when deciding whether to accept a packet (0 = no filtering)
					uint32_t recvBufferDescrAddress;    ///< Physical address of the first receive buffer descriptor
					uint32_t sendBufferDescrAddress;    ///< Physical address of the first send buffer descriptor

			}
			initialisation_block_t;

			/**
			 * @struct BufferDescriptor
			 * @brief Defines the layout of a buffer descriptor for the AMD AM79C973 Ethernet Controller
			 *
			 * @typedef buffer_descriptor_t
			 * @brief Alias for BufferDescriptor struct
			 */
			typedef struct PACKED BufferDescriptor {

					uint64_t address;   ///< Physical address of the buffer
					uint32_t flags;     ///< Flags for the buffer
					uint32_t flags2;    ///< Additional flags for the buffer (@todo enum this)
					uint32_t avail;     ///< Indicates whether the buffer is available to be used

			}
			buffer_descriptor_t;

			/**
			 * @class AMD_AM79C973
			 * @brief Driver for the AMD AM79C973 Ethernet Controller
			 */
			class AMD_AM79C973 : public EthernetDriver, public hardwarecommunication::InterruptHandler {

				private:

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
					initialisation_block_t initBlock { };


					buffer_descriptor_t* sendBufferDescr;               //Descriptor entry
					uint8_t sendBuffers[2 * 1024 + 15][8] { };       //8 Send Buffers, 2KB + 15 bytes
					uint8_t currentSendBuffer;               //Which buffers are active

					buffer_descriptor_t* recvBufferDescr;               //Descriptor entry
					uint8_t recvBuffers[2 * 1024 + 15][8] { };       //8 Send Buffers, 2KB + 15 bytes
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

					// Override driver default methods
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