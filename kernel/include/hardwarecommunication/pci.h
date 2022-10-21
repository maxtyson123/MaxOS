//
// Created by 98max on 12/10/2022.
//

#ifndef MAX_OS_HARDWARECOMMUNICATION_PCI_H
#define MAX_OS_HARDWARECOMMUNICATION_PCI_H

#include <common/types.h>
#include <drivers/driver.h>
#include <memorymanagement.h>
#include <hardwarecommunication/port.h>
#include <hardwarecommunication/interrupts.h>


namespace maxOS
{
    namespace hardwarecommunication
    {
        enum BaseAdressRegisterType{        //Used for the last bit of the address register
            MemoryMapping = 0,
            InputOutput = 1
        };

        class BaseAdressRegister{
            public:
                bool preFetchable;
                common::uint8_t* adress;
                common::uint32_t size;
                BaseAdressRegisterType type;

        };


        class PeripheralComponentInterconnectDeviceDescriptor {
            public:
                common::uint32_t portBase;  //Port used for communication
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
        };


        class PeripheralComponentInterconnectController
        {
                Port32Bit dataPort;
                Port32Bit commandPort;
            public:
                PeripheralComponentInterconnectController();
                ~PeripheralComponentInterconnectController();

                common::uint32_t Read(common::uint16_t bus, common::uint16_t device, common::uint16_t function, common::uint32_t registeroffset);
                void Write(common::uint16_t bus, common::uint16_t device, common::uint16_t function, common::uint32_t registeroffset, common::uint32_t value);
                bool DeviceHasFunctions(common::uint16_t bus, common::uint16_t device);

                void SelectDrivers(drivers::DriverManager* driverManager, InterruptManager* interruptManager);
                drivers::Driver* GetDriver(PeripheralComponentInterconnectDeviceDescriptor dev, InterruptManager* interruptManager);

                PeripheralComponentInterconnectDeviceDescriptor GetDeviceDescriptor(common::uint16_t bus, common::uint16_t device, common::uint16_t function);
                BaseAdressRegister GetBaseAdressRegister(common::uint16_t bus, common::uint16_t device, common::uint16_t function, common::uint16_t bar);       // bar = 0-5 in case of header type 0 [or]  bar = 0-1 in case of header type 1
        };
    }
}

#endif //MAX_HARDWARECOMMUNICATION_OS_PCI_H
