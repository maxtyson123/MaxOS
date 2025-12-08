/**
 * @file amd_am79c973.h
 * @brief Driver for the AMD AM79C973 Ethernet Controller
 *
 * @date 22nd October 2022
 * @author Max Tyson
 */

#ifndef MAXOS_DRIVERS_ETHERNET_AMD_AM79C973_H
#define MAXOS_DRIVERS_ETHERNET_AMD_AM79C973_H

#include <cstdint>
#include <macros.h>
#include <drivers/ethernet/ethernet.h>
#include <hardwarecommunication/pci.h>
#include <hardwarecommunication/interrupts.h>
#include <hardwarecommunication/port.h>

namespace MaxOS::drivers::ethernet {

	/**
	 * @struct InitializationBlock
	 * @brief The initialization block for the AMD AM79C973 Ethernet Controller
	 *
	 * @typedef initialisation_block_t
	 * @brief Alias for InitializationBlock struct
	 */
	typedef struct PACKED InitializationBlock {

		uint16_t mode;                      ///< The operation mode
		unsigned reserved1 : 4;                ///< Unused, must be zero
		unsigned num_send_buffers : 4;            ///< How many buffers are used for sending
		unsigned reserved2 : 4;                ///< Unused, must be zero
		unsigned num_recv_buffers : 4;            ///< How many buffers are used for receiving
		uint64_t physical_address : 48;        ///< The physical (MAC) address of the device (Not 64 bits but will be treated like it is)
		uint16_t reserved3;                 ///< Unused, must be zero
		uint64_t logical_address;            ///< The logical address filter for the device to use when deciding whether to accept a packet (0 = no filtering)
		uint32_t recv_buffer_descr_address;    ///< Physical address of the first receive buffer descriptor
		uint32_t send_buffer_descr_address;    ///< Physical address of the first send buffer descriptor

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
			hardwarecommunication::Port16Bit mac_address_0_port;
			hardwarecommunication::Port16Bit mac_address_2_port;
			hardwarecommunication::Port16Bit mac_address_4_port;

			//Register ports
			hardwarecommunication::Port16Bit register_data_port;
			hardwarecommunication::Port16Bit register_address_port;
			hardwarecommunication::Port16Bit bus_control_register_data_port;

			hardwarecommunication::Port16Bit reset_port;

			//The main purpose of the initialization block it to hold a pointer to the array of BufferDescriptors, which hold the pointers to the buffers
			initialisation_block_t init_block { };


			buffer_descriptor_t* send_buffer_descr;               //Descriptor entry
			uint8_t send_buffers[2 * 1024 + 15][8] { };       //8 Send Buffers, 2KB + 15 bytes
			uint8_t current_send_buffer;               //Which buffers are active

			buffer_descriptor_t* recv_buffer_descr;               //Descriptor entry
			uint8_t recv_buffers[2 * 1024 + 15][8] { };       //8 Send Buffers, 2KB + 15 bytes
			uint8_t current_recv_buffer;               //Which buffers are active

			//Ethernet Driver functions
			MediaAccessControlAddress own_mac;                //MAC address of the device
			volatile bool active;                            //Is the device active
			volatile bool init_done;                          //Is the device initialised

			void fetch_data_received();                        //Fetches the data from the buffer
			void fetch_data_sent();                            //Fetches the data from the buffer

		public:
			explicit AMD_AM79C973(hardwarecommunication::PCIDeviceDescriptor* dev);
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
			void do_send(uint8_t* buffer, uint32_t size) final;
			uint64_t get_media_access_control_address() final;
	};


}


#endif //MAXOS_DRIVERS_ETHERNET_AMD_AM79C973_H