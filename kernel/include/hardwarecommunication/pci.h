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
        enum BaseAddressRegisterType{        //Used for the last bit of the address register
            MemoryMapping = 0,
            InputOutput = 1
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
                bool has_port_base;
                uint32_t port_base;  //Port used for communication

                bool has_memory_base;
                uint32_t memory_base;  //Mem address used for communication

                uint32_t interrupt; //The interrupt

                uint16_t bus;
                uint16_t device;
                uint16_t function;

                uint16_t vendor_id;
                uint16_t device_id;

                uint8_t class_id;
                uint8_t subclass_id;
                uint8_t interface_id;

                uint8_t revision;

                PeripheralComponentInterconnectDeviceDescriptor();
                ~PeripheralComponentInterconnectDeviceDescriptor();

                string get_type();
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
                  PeripheralComponentInterconnectController();
                  ~PeripheralComponentInterconnectController();

                  void select_drivers(drivers::DriverSelectorEventHandler *handler, hardwarecommunication::InterruptManager* interrupt_manager) override;
                  drivers::Driver* get_driver(PeripheralComponentInterconnectDeviceDescriptor dev, InterruptManager* interrupt_manager);
                  void list_known_device(PeripheralComponentInterconnectDeviceDescriptor dev);
        };
    }
}

#endif //MAX_OS_HARDWARECOMMUNICATION_PCI_H
