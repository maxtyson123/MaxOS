//
// Created by 98max on 12/10/2022.
//

#ifndef MAX_OS_HARDWARECOMMUNICATION_PCI_H
#define MAX_OS_HARDWARECOMMUNICATION_PCI_H

#include <stdint.h>
#include <drivers/driver.h>
#include <memory/memorymanagement.h>
#include <hardwarecommunication/port.h>
#include <hardwarecommunication/interrupts.h>
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
                uint8_t* address;                //The address of the register
                uint32_t size;                  //The size of the register
                BaseAddressRegisterType type;            //The type of the register

        };


        //TODO: With USB could be a good idea to make a class for the device descriptor
        class PeripheralComponentInterconnectDeviceDescriptor {
            public:
                bool hasPortBase;
                uint32_t portBase;  //Port used for communication

                bool hasMemoryBase;
                uint32_t memoryBase;  //Mem address used for communication

                uint32_t interrupt; //The interrupt

                uint16_t bus;
                uint16_t device;
                uint16_t function;

                uint16_t vendor_ID;
                uint16_t device_ID;

                uint8_t class_id;
                uint8_t subclass_id;
                uint8_t interface_id;

                uint8_t revision;

                PeripheralComponentInterconnectDeviceDescriptor();
                ~PeripheralComponentInterconnectDeviceDescriptor();

                string getType();
        };


        class PeripheralComponentInterconnectController : public drivers::DriverSelector
        {

                // Ports
                Port32Bit dataPort;
                Port32Bit commandPort;

                // Debug
                common::OutputStream* debugMessagesStream;

                // I/O
                uint32_t Read(uint16_t bus, uint16_t device, uint16_t function, uint32_t registeroffset);
                void Write(uint16_t bus, uint16_t device, uint16_t function, uint32_t registeroffset, uint32_t value);

                // Device
                PeripheralComponentInterconnectDeviceDescriptor GetDeviceDescriptor(uint16_t bus, uint16_t device, uint16_t function);
                BaseAddressRegister getBaseAddressRegister(uint16_t bus, uint16_t device, uint16_t function, uint16_t bar);       // bar = 0-5 in case of header type 0 [or]  bar = 0-1 in case of header type 1
                bool DeviceHasFunctions(uint16_t bus, uint16_t device);

        public:
                PeripheralComponentInterconnectController(common::OutputStream* debugMessagesStream);
                ~PeripheralComponentInterconnectController();

                void selectDrivers(drivers::DriverSelectorEventHandler* handler, hardwarecommunication::InterruptManager* interruptManager, common::OutputStream* errorMessageStream);
                drivers::Driver* GetDriver(PeripheralComponentInterconnectDeviceDescriptor dev, InterruptManager* interruptManager);
                void listKnownDeivce(PeripheralComponentInterconnectDeviceDescriptor dev);
        };
    }
}

#endif //MAX_HARDWARECOMMUNICATION_OS_PCI_H
