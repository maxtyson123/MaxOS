/**
 * @file pci.h
 * @brief Defines a Peripheral Component Interconnect (PCI) controller for managing PCI devices and loading their drivers
 *
 * @date 12th October 2022
 * @author Max Tyson
 */

#ifndef MAX_OS_HARDWARECOMMUNICATION_PCI_H
#define MAX_OS_HARDWARECOMMUNICATION_PCI_H

#include <stdint.h>
#include <common/string.h>
#include <drivers/driver.h>
#include <memory/memorymanagement.h>
#include <hardwarecommunication/port.h>
#include <hardwarecommunication/interrupts.h>
#include <drivers/video/vga.h>

namespace MaxOS
{
    namespace hardwarecommunication
    {
		/**
		 * @enum BaseAddressRegisterType
		 * @brief Determines whether the PCI device communicates via IO ports or memory.
		 */
        enum class BaseAddressRegisterType{
            MemoryMapped,
            InputOutput
        };

        /**
         * @class BaseAddressRegister
         * @brief Used to store the base address register
         *
         * @todo Should be a struct
         */
        class BaseAddressRegister{
            public:
                bool pre_fetchable;                 ///< Reading from this address wont change the state of the device and data can be cached by the CPU
                uint8_t* address;                   ///< The address of the device (IO port or memory address, can be 32 or 64 bit)
                uint32_t size;                      ///< Todo: Size of the address space
                BaseAddressRegisterType type;       ///< Where to access the device

        };


        /**
         * @class PeripheralComponentInterconnectDeviceDescriptor
         * @brief Stores information about a PCI device
         *
         * @todo Should be a struct aswell
         */
        class PeripheralComponentInterconnectDeviceDescriptor {
            public:
                bool has_port_base = false;         ///< Whether the device has an IO port base address
                uint32_t port_base = 0;             ///< The IO port base address

                bool has_memory_base = false;       ///< Whether the device has a memory base address
                uint32_t memory_base = 0;           ///< The memory base address

                uint32_t interrupt = 0;             ///< The interrupt number the device uses to communicate with the CPU

                uint16_t bus = 0;                   ///< The PCI bus the device is connected to
                uint16_t device = 0;                ///< The device number on the PCI bus
                uint16_t function = 0;              ///< The function number of the device

                uint16_t vendor_id = 0;             ///< The company's that made the device unique identifier
                uint16_t device_id = 0;             ///< The device's unique identifier

                uint8_t class_id = 0;               ///< The class type of the device
                uint8_t subclass_id = 0;            ///< The subclass type of the device
                uint8_t interface_id = 0;           ///< The interface type of the device

                uint8_t revision = 0;               ///< The device version number

                PeripheralComponentInterconnectDeviceDescriptor();
                ~PeripheralComponentInterconnectDeviceDescriptor();

                string get_type() const;
        };


        /**
         * @class PeripheralComponentInterconnectController
         * @brief Handles the selecting and loading of drivers for PCI devices
         */
        class PeripheralComponentInterconnectController : public drivers::DriverSelector
        {
          private:
                  // Ports
                  Port32Bit m_data_port;
                  Port32Bit m_command_port;

                  // I/O
                  uint32_t read(uint16_t bus, uint16_t device, uint16_t function, uint32_t register_offset);
                  void write(uint16_t bus, uint16_t device, uint16_t function, uint32_t register_offset, uint32_t value);

                  // Device
                  PeripheralComponentInterconnectDeviceDescriptor get_device_descriptor(uint16_t bus, uint16_t device, uint16_t function);
                  BaseAddressRegister get_base_address_register(uint16_t bus, uint16_t device, uint16_t function, uint16_t bar);
                  bool device_has_functions(uint16_t bus, uint16_t device);

          public:
                  PeripheralComponentInterconnectController();
                  ~PeripheralComponentInterconnectController();

                  void select_drivers(drivers::DriverSelectorEventHandler* handler) override;
                  static drivers::Driver* get_driver(PeripheralComponentInterconnectDeviceDescriptor dev);
                  static void list_known_device(const PeripheralComponentInterconnectDeviceDescriptor& dev);
        };
    }
}

#endif //MAX_OS_HARDWARECOMMUNICATION_PCI_H
