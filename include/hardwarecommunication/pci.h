//
// Created by 98max on 12/10/2022.
//

#ifndef MAX_OS_HARDWARECOMMUNICATION_PCI_H
#define MAX_OS_HARDWARECOMMUNICATION_PCI_H

#include <common/types.h>
#include <drivers/driver.h>
#include <hardwarecommunication/port.h>
#include <hardwarecommunication/interrupts.h>


namespace maxos
{
    namespace hardwarecommunication
    {
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

                void SelectDrivers(drivers::DriverManager* driverManager);
                PeripheralComponentInterconnectDeviceDescriptor GetDeviceDescriptor(common::uint16_t bus, common::uint16_t device, common::uint16_t function);
        };
    }
}

#endif //MAX_HARDWARECOMMUNICATION_OS_PCI_H
