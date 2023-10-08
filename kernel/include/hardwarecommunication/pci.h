//
// Created by 98max on 12/10/2022.
//

#ifndef MAX_OS_HARDWARECOMMUNICATION_PCI_H
#define MAX_OS_HARDWARECOMMUNICATION_PCI_H

#include <common/types.h>
#include <drivers/driver.h>

// Memory
#include <memory/memorymanagement.h>
#include <hardwarecommunication/port.h>
#include <hardwarecommunication/interrupts.h>

// Drivers
#include <drivers/video/vga.h>

namespace maxOS
{
    namespace hardwarecommunication
    {
        enum BaseAddressRegisterType{        //Used for the last bit of the address register
            MemoryMapping = 0,
            InputOutput = 1
        };

        class BaseAddressRegister{
            public:
                bool preFetchable;                      //If the memory is pre-fetchable
                common::uint8_t* address;                //The address of the register
                common::uint32_t size;                  //The size of the register
                BaseAddressRegisterType type;            //The type of the register

        };


        class PeripheralComponentInterconnectDeviceDescriptor {
            public:
                bool hasPortBase;
                common::uint32_t portBase;  //Port used for communication

                bool hasMemoryBase;
                common::uint32_t memoryBase;  //Mem adress used for communication

                common::uint32_t interrupt; //The interrupt

                common::uint16_t bus;
                common::uint16_t device;
                common::uint16_t function;

                common::uint16_t vendor_ID;
                common::uint16_t device_ID;

                common::uint8_t class_id;
                common::uint8_t subclass_id;
                common::uint8_t interface_id;

                common::uint8_t revision;

                PeripheralComponentInterconnectDeviceDescriptor();
                ~PeripheralComponentInterconnectDeviceDescriptor();

                common::string getType();
        };


        class PeripheralComponentInterconnectController : public drivers::DriverSelector
        {

                // Ports
                Port32Bit dataPort;
                Port32Bit commandPort;

                // Debug
                common::OutputStream* debugMessagesStream;

                // I/O
                common::uint32_t Read(common::uint16_t bus, common::uint16_t device, common::uint16_t function, common::uint32_t registeroffset);
                void Write(common::uint16_t bus, common::uint16_t device, common::uint16_t function, common::uint32_t registeroffset, common::uint32_t value);

                // Device
                PeripheralComponentInterconnectDeviceDescriptor GetDeviceDescriptor(common::uint16_t bus, common::uint16_t device, common::uint16_t function);
                BaseAddressRegister getBaseAddressRegister(common::uint16_t bus, common::uint16_t device, common::uint16_t function, common::uint16_t bar);       // bar = 0-5 in case of header type 0 [or]  bar = 0-1 in case of header type 1
                bool DeviceHasFunctions(common::uint16_t bus, common::uint16_t device);

        public:
                PeripheralComponentInterconnectController(common::OutputStream* debugMessagesStream);
                ~PeripheralComponentInterconnectController();

                void selectDrivers(drivers::DriverSelectorEventHandler* handler, memory::MemoryManager* memoryManager, hardwarecommunication::InterruptManager* interruptManager, common::OutputStream* errorMessageStream);
                drivers::Driver* GetDriver(PeripheralComponentInterconnectDeviceDescriptor dev, InterruptManager* interruptManager);
        };
    }
}

#endif //MAX_HARDWARECOMMUNICATION_OS_PCI_H
