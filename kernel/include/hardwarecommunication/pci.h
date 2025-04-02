//
// Created by 98max on 12/10/2022.
//

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
        enum class BaseAddressRegisterType{        //Used for the last bit of the address register
            MemoryMapped,
            InputOutput
        };

        /**
         * @class BaseAddressRegister
         * @brief Used to store the base address register
         */
        class BaseAddressRegister{
            public:
                bool pre_fetchable;
                uint8_t* address;
                uint32_t size;
                BaseAddressRegisterType type;

        };


        /**
         * @class PeripheralComponentInterconnectDeviceDescriptor
         * @brief Stores information about a PCI device
         */
        class PeripheralComponentInterconnectDeviceDescriptor {
            public:
                bool has_port_base = false;
                uint32_t port_base = 0;

                bool has_memory_base = false;
                uint32_t memory_base = 0;

                uint32_t interrupt = 0;

                uint16_t bus = 0;
                uint16_t device = 0;
                uint16_t function = 0;

                uint16_t vendor_id = 0;
                uint16_t device_id = 0;

                uint8_t class_id = 0;
                uint8_t subclass_id = 0;
                uint8_t interface_id = 0;

                uint8_t revision = 0;

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
                  uint32_t read(uint16_t bus, uint16_t device, uint16_t function, uint32_t registeroffset);
                  void write(uint16_t bus, uint16_t device, uint16_t function, uint32_t registeroffset, uint32_t value);

                  // Device
                  PeripheralComponentInterconnectDeviceDescriptor get_device_descriptor(uint16_t bus, uint16_t device, uint16_t function);
                  BaseAddressRegister get_base_address_register(uint16_t bus, uint16_t device, uint16_t function, uint16_t bar);
                  bool device_has_functions(uint16_t bus, uint16_t device);

          public:
                  PeripheralComponentInterconnectController(drivers::DriverManager* driver_manager);
                  ~PeripheralComponentInterconnectController();

                  void select_drivers(drivers::DriverSelectorEventHandler *handler) override;
                  static drivers::Driver* get_driver(PeripheralComponentInterconnectDeviceDescriptor dev);
                  static void list_known_device(const PeripheralComponentInterconnectDeviceDescriptor& dev);
        };
    }
}

#endif //MAX_OS_HARDWARECOMMUNICATION_PCI_H
