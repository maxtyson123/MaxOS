/**
 * @file intel_i217.h
 * @brief Driver for the Intel I217 Ethernet Controller
 *
 * @date 29th November 2022
 * @author Max Tyson
 */

#ifndef MAXOS_DRIVERS_ETHERNET_INTEL_I127_H
#define MAXOS_DRIVERS_ETHERNET_INTEL_I127_H

#include <cstddef>
#include <outputStream.h>
#include <macros.h>
#include <assert.h>
#include <drivers/driver.h>
#include <hardwarecommunication/pci.h>
#include <hardwarecommunication/interrupts.h>
#include <hardwarecommunication/port.h>
#include <memory/memoryIO.h>
#include <memory/memorymanagement.h>
#include <drivers/ethernet/ethernet.h>


namespace MaxOS::drivers::ethernet {

	/**
	 * @struct ReceiveDescriptor
	 * @brief The receive descriptor for the Intel I217 Ethernet Controller
	 *
	 * @typedef receive_descriptor_t
	 * @brief Alias for ReceiveDescriptor struct
	 */
	typedef struct PACKED ReceiveDescriptor {
		uint64_t buffer_address;              ///< The address of the receive buffer
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
		uint64_t buffer_address;             ///< The address of the send buffer
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
			uint16_t port_base = { 0 };
			uint64_t mem_base = { 0 };
			uint8_t mac_address[6] = { 0 };


			//Registers Addresses (Main Registers)
			uint16_t control_register;                // The control register
			uint16_t status_register;                 // The status register
			uint16_t eprom_register;                  // The address of the eeprom register
			uint16_t control_ext_register;             // The control extension register
			uint16_t interrupt_mask_register;          // The interrupt mask register

			//Registers Addresses (Receive Registers)
			uint16_t receive_control_register;         // The receive control register
			uint16_t receive_descriptor_low_register;   // The receive descriptor low register
			uint16_t receive_descriptor_high_register;  // The receive descriptor high register
			uint16_t receive_descriptor_length_register;// The receive descriptor length register
			uint16_t receive_descriptor_head_register;  // The receive descriptor head register
			uint16_t receive_descriptor_tail_register;  // The receive descriptor tail register

			//Registers Addresses (Send Registers)
			uint16_t send_control_register;            // The send control register
			uint16_t send_descriptor_low_register;      // The send descriptor low register
			uint16_t send_descriptor_high_register;     // The send descriptor high register
			uint16_t send_descriptor_length_register;   // The send descriptor length register
			uint16_t send_descriptor_head_register;     // The send descriptor head register
			uint16_t send_descriptor_tail_register;     // The send descriptor tail register




			//Buffers
			receive_descriptor_t* receive_dsrctrs[32];    // The receive descriptors
			uint16_t current_receive_buffer;           // The current receive buffer

			send_descriptor_t* send_dsrctrs[8];           // The send descriptors
			uint16_t current_send_buffer;              // The current send buffer


			// write Commands and read results From NICs either using MemIO or IO Ports
			void write(uint16_t address, uint32_t data) const;
			[[nodiscard]] uint32_t read(uint16_t address) const;

			//EPROM (Device Memory)
			bool eprom_present = false;                                   // Whether the EPROM is present
			bool detect_ee_prom();                                 // Return true if EEProm exist, else it returns false and set the error_exists data member
			uint32_t eeprom_read(uint8_t addr);  // read 4 bytes from a specific EEProm Address


			bool read_mac_address();       // read MAC Address

			void receive_init();          // Initialise receive descriptors buffers
			void send_init();             // Initialise transmit descriptors buffers

			//Ethernet Driver functions
			MediaAccessControlAddress own_mac;                //MAC address of the device
			volatile bool active;                            //Is the device active
			volatile bool init_done;                          //Is the device initialised

			void fetch_data_received();                        //Fetches the data from the buffer

		public:

			explicit IntelI217(hardwarecommunication::PCIDeviceDescriptor* device_descriptor);
			~IntelI217();


			//Override driver default methods
			uint32_t reset() final;
			void activate() final;
			void deactivate() final;

			//Override Interrupt default methods
			void handle_interrupt() final;


			//Ethernet Driver functions
			string vendor_name() final;
			string device_name() final;

			void do_send(uint8_t* buffer, uint32_t size) final;
			uint64_t get_media_access_control_address() final;
	};

}


#endif //MAXOS_DRIVERS_ETHERNET_INTEL_I127_H
