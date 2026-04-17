/**
 * @file pci.h
 * @brief Defines a Peripheral Component Interconnect (PCI) controller for managing PCI devices and loading their drivers
 *
 * @date 12th October 2022
 * @author Max Tyson
 */

#ifndef DRIVER_MANAGER_CORE_PCI_H
#define DRIVER_MANAGER_CORE_PCI_H

#include <cstdint>
#include <string.h>
#include <driver.h>
#include <mem.h>
#include <port.h>
#include <core/device.h>


namespace DriverManager::core {

	/**
	 * @enum BARType
	 * @brief Determines whether the PCI device communicates via IO ports or memory.
	 */
	enum class BARType {
		MemoryMapped,
		InputOutput
	};

	/**
	 * @struct BaseAddressRegister
	 * @brief Used to store the Base Address Register (BAR) of a PCI device
	 *
	 * @typedef bar_t
	 * @breif Alias for BaseAddressRegister
	 */
	typedef struct  BaseAddressRegister {
			bool pre_fetchable;                 ///< Reading from this address wont change the state of the device and data can be cached by the CPU
			uint8_t* address;                   ///< The address of the device (IO port or memory address, can be 32 or 64 bit)
			uint32_t size;                      ///< @todo Document Size of the address space
			BARType type;						///< Where to access the device

	} bar_t;


	/**
	 * @class PCIDeviceDescriptor
	 * @brief Stores information about a PCI device
	 *
	 * @todo Should be a struct aswell
	 */
	typedef struct PCIDeviceDescriptor {

			bool has_port_base = false;         ///< Whether the device has an IO port base address
			uint32_t port_base = 0;             ///< The IO port base address

			bool has_memory_base = false;       ///< Whether the device has a memory base address
			uint32_t memory_base = 0;           ///< The memory base address

			uint32_t interrupt = 0;             ///< The interrupt number the device uses to communicate with the CPU

			uint16_t bus = 0;                   ///< The PCI bus the device is connected to
			uint16_t device = 0;                ///< The device number on the PCI bus
			uint16_t function = 0;              ///< The function number of the device

			uint16_t vendor_id = 0;             ///< The company that made the device unique identifier
			uint16_t device_id = 0;             ///< The device's unique identifier
			uint16_t sub_vendor_id = 0;			///< The company that made the subsystem

			uint8_t class_id = 0;               ///< The class type of the device
			uint8_t subclass_id = 0;            ///< The subclass type of the device
			uint8_t interface_id = 0;           ///< The interface type of the device

			uint8_t revision = 0;               ///< The device version number
	} pci_device_descriptor_t;

	class PCIDevice : public Device {

		private:
			pci_device_descriptor_t m_device_descriptor;

			LibDriver::Driver* handle_driver_start();

		public:
			PCIDevice(pci_device_descriptor_t device_descriptor);
			~PCIDevice();

			bool builtin_driver();

			static LibDriver::DriverType get_driver_type(const pci_device_descriptor_t& device_descriptor);

	};

	/**
	 * @class PCIController
	 * @brief Handles the enumeration and loading of drivers for PCI devices
	 */
	class PCIController final : public DeviceEnumerator {

		private:
			// Ports
			MaxOS::common::Port32Bit m_data_port;
			MaxOS::common::Port32Bit m_command_port;

			// I/O
			uint32_t read(uint16_t bus, uint16_t device, uint16_t function, uint32_t register_offset);
			void write(uint16_t bus, uint16_t device, uint16_t function, uint32_t register_offset, uint32_t value);

			// Device
			PCIDeviceDescriptor get_device_descriptor(uint16_t bus, uint16_t device, uint16_t function);
			BaseAddressRegister get_base_address_register(uint16_t bus, uint16_t device, uint16_t function, uint16_t bar);
			bool device_has_functions(uint16_t bus, uint16_t device);

			MaxOS::common::Vector<MaxOS::string> m_pci_id_lines;

			MaxOS::common::Map<uint8_t, MaxOS::string> m_vendor_name_cache;
			MaxOS::common::Map<uint8_t, MaxOS::string> m_device_name_cache;
			MaxOS::common::Map<uint8_t, MaxOS::string> m_subvendor_name_cache;

		public:
			PCIController();
			~PCIController() final;

			void enumerate_devices(DeviceEnumeratorEventHandler* handler) final;
			static LibDriver::Driver* get_driver(pci_device_descriptor_t dev);
			static void list_known_device(const pci_device_descriptor_t& dev);

			[[nodiscard]] MaxOS::string get_class_string(const pci_device_descriptor_t& dev);
			[[nodiscard]] MaxOS::string get_pci_id_string(const pci_device_descriptor_t& dev);
			[[nodiscard]] LibDriver::DriverType get_driver_type(const pci_device_descriptor_t& dev);

	};
}


#endif //DRIVER_MANAGER_CORE_PCI_H
