//
// Created by 98max on 12/10/2022.
//
#include <hardwarecommunication/pci.h>

using namespace maxos::common;
using namespace maxos::hardwarecommunication;
using namespace maxos::drivers;

void printf(char* str, bool clearLine = false); //Forward declaration
void printfHex(uint8_t key);                    //Forward declaration



///__DESCRIPTOR___

PeripheralComponentInterconnectDeviceDescriptor::PeripheralComponentInterconnectDeviceDescriptor() {

}

PeripheralComponentInterconnectDeviceDescriptor::~PeripheralComponentInterconnectDeviceDescriptor() {

}

///__CONTROLLER___

PeripheralComponentInterconnectController::PeripheralComponentInterconnectController()
: dataPort(0xCFC),      //PCI Controller
  commandPort(0xCF8)    //PCI Controller

{

}

PeripheralComponentInterconnectController::~PeripheralComponentInterconnectController() {

}

uint32_t PeripheralComponentInterconnectController::Read(uint16_t bus, uint16_t device, uint16_t function, uint32_t registeroffset) {
    //Structure of the address I / O port :
    //31    | Enable bit
    //30-24 | Reserved
    //23-16 | Bus number
    //15-11 | Device number
    //10-8  | Function number
    //7-0   | Register number , bit 0 and bit 1 are 0

    uint32_t id =
            0x1 << 31
            | ((bus & 0xFF) << 16)
            | ((device & 0x1F) << 11)
            | ((function & 0x07) << 8)
            | (registeroffset & 0xFC);      //Cut off the last two bits of the number
    commandPort.Write(id);             //The ID is like the adress of the port
    uint32_t result = dataPort.Read();
    return result >> (8* (registeroffset % 4));

}

void PeripheralComponentInterconnectController::Write(uint16_t bus, uint16_t device, uint16_t function, uint32_t registeroffset, uint32_t value) {
    uint32_t id =
            0x1 << 31
            | ((bus & 0xFF) << 16)
            | ((device & 0x1F) << 11)
            | ((function & 0x07) << 8)
            | (registeroffset & 0xFC);
    commandPort.Write(id);
    dataPort.Write(value);
}

bool PeripheralComponentInterconnectController::DeviceHasFunctions(uint16_t bus, uint16_t device) {

    //0x0E
    return Read(bus,device,0,0x0E) & (1<<7);    //Read address, the 7th bit of it returns if there is a function
}

void PeripheralComponentInterconnectController::SelectDrivers(DriverManager* driverManager) {
    for (int bus = 0; bus < 8; ++bus) {

        for (int device = 0; device < 32; ++device) {

            int numFunctions = (DeviceHasFunctions(bus,device)) ? 8 : 1;

            for (int function = 0; function < numFunctions; ++function) {
                PeripheralComponentInterconnectDeviceDescriptor deviceDescriptor = GetDeviceDescriptor(bus, device, function);

                //If there is no device then vendor ID is 0x0000, If the device is not ready to react then the vendor ID is 0x0001   (REMOVE 0xFFFF if things go wrong)
                if(deviceDescriptor.vendor_ID == 0x0000 || deviceDescriptor.vendor_ID == 0x0001 || deviceDescriptor.vendor_ID == 0xFFFF){
                    break;                                                                          //This also means there are no more function afterwards
                }

                //Display INFO
                printf("    PCI BUS ");
                printfHex(bus & 0xFF);

                printf(", DEVICE ");
                printfHex(device & 0xFF);

                printf(", FUNCTION ");
                printfHex(function & 0xFF);

                printf(", VENDOR ");
                printfHex((deviceDescriptor.vendor_ID & 0xFF00) >> 8);
                printfHex(deviceDescriptor.vendor_ID & 0xFF);

                printf(", DEVICE ");
                printfHex((deviceDescriptor.device_ID & 0xFF00) >> 8);
                printfHex(deviceDescriptor.device_ID & 0xFF);

                printf("\n");
            }

        }
    }
}

PeripheralComponentInterconnectDeviceDescriptor PeripheralComponentInterconnectController::GetDeviceDescriptor(uint16_t bus, uint16_t device, uint16_t function) {
    PeripheralComponentInterconnectDeviceDescriptor result;

    result.bus = bus;
    result.device = device;
    result.function = function;

    result.vendor_ID = Read(bus, device, function, 0x00);
    result.device_ID = Read(bus, device, function, 0x02);

    result.class_id = Read(bus, device, function, 0x0B);
    result.subclass_id = Read(bus, device, function, 0x0A);
    result.interface_id = Read(bus, device, function, 0x09);

    result.revision = Read(bus, device, function, 0x8);
    result.interrupt = Read(bus, device, function, 0x3C);

    return result;
}


