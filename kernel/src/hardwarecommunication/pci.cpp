//
// Created by 98max on 12/10/2022.
//
#include <hardwarecommunication/pci.h>
#include <drivers/ethernet/amd_am79c973.h>
#include <drivers/ethernet/intel_i217.h>

using namespace maxOS::common;
using namespace maxOS::hardwarecommunication;
using namespace maxOS::drivers;
using namespace maxOS::drivers::ethernet;
using namespace maxOS::drivers::video;
using namespace maxOS::memory;

///__DESCRIPTOR___

PeripheralComponentInterconnectDeviceDescriptor::PeripheralComponentInterconnectDeviceDescriptor() {

}

PeripheralComponentInterconnectDeviceDescriptor::~PeripheralComponentInterconnectDeviceDescriptor() {

}

string PeripheralComponentInterconnectDeviceDescriptor::getType() {
    switch (class_id)
    {
        case 0x00: return (subclass_id == 0x01) ? "VGA" : "Legacy";
        case 0x01:
            switch(subclass_id)
            {
                case 0x01:  return "IDE interface";
                case 0x06:  return "SATA controller";
                default:    return "Storage";
            }
        case 0x02: return "Network";                          // Network
        case 0x03: return "Display";
        case 0x04:                                         // Multimedia
            switch(subclass_id)
            {
                case 0x00:  return "Video";
                case 0x01:
                case 0x03:  return "Audio";
                default:    return "Multimedia";
            }
        case 0x06:                                            // Bridges
            switch(subclass_id)
            {
                case 0x00:  return "Host bridge";
                case 0x01:  return "ISA bridge";
                case 0x04:  return "PCI bridge";
                default:    return "Bridge";
            }
        case 0x07:
            switch(subclass_id)
            {
                case 0x00:  return "Serial controller";
                case 0x80:  return "Communication controller";
            }
            break;
        case 0x0C:
            switch(subclass_id)
            {
                case 0x03:  return "USB";
                case 0x05:  return "System Management Bus";
            }
            break;
    }
    return "Unknown";
}

///__CONTROLLER___

PeripheralComponentInterconnectController::PeripheralComponentInterconnectController(OutputStream* debug)
: dataPort(0xCFC),      //PCI Controller
  commandPort(0xCF8)    //PCI Controller

{
    this->debugMessagesStream = debug;
}

PeripheralComponentInterconnectController::~PeripheralComponentInterconnectController() {

}

/**
 * @details Read data from the PCI Controller
 *
 * @param bus Bus number
 * @param device Device number
 * @param function Function number
 * @param registeroffset Register offset
 * @return  data from the PCI Controller
 */
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
    commandPort.Write(id);             //The ID is like the address of the port
    uint32_t result = dataPort.Read();
    return result >> (8* (registeroffset % 4));

}

/**
 * @details Write data to the PCI Controller
 *
 * @param bus Bus number
 * @param device Device number
 * @param function Function number
 * @param registeroffset Register offset
 * @param value Value to write
 */
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

/**
 * @details Check if the device has a function
 *
 * @param bus Bus number
 * @param device Device number
 * @return true if the device has a function
 */
bool PeripheralComponentInterconnectController::DeviceHasFunctions(uint16_t bus, uint16_t device) {

    //0x0E
    return Read(bus,device,0,0x0E) & (1<<7);    //Read address, the 7th bit of it returns if there is a function
}

/**
 * @details Select the driver for the device
 *
 * @param driverManager device driver manager
 * @param interruptManager Interrupt manager
 * @return Driver for the device
 */
void PeripheralComponentInterconnectController::selectDrivers(drivers::DriverSelectorEventHandler *handler, memory::MemoryManager *memoryManager, hardwarecommunication::InterruptManager *interruptManager, common::OutputStream *errorMessageStream) {
    for (int bus = 0; bus < 8; ++bus) {

        for (int device = 0; device < 32; ++device) {

            int numFunctions = (DeviceHasFunctions(bus,device)) ? 8 : 1;

            for (int function = 0; function < numFunctions; ++function) {
                PeripheralComponentInterconnectDeviceDescriptor deviceDescriptor = GetDeviceDescriptor(bus, device, function);

                //If there is no device then vendor ID is 0x0000, If the device is not ready to react then the vendor ID is 0x0001
                if(deviceDescriptor.vendor_ID == 0x0000 || deviceDescriptor.vendor_ID == 0x0001 || deviceDescriptor.vendor_ID == 0xFFFF){
                    continue;
                }

                // Get port number
                for(int barNum = 5; barNum >= 0; barNum--){

                    BaseAddressRegister bar = getBaseAddressRegister(bus, device, function, barNum);
                    if(bar.address && (bar.type == InputOutput)){
                        deviceDescriptor.portBase = (uint32_t)bar.address;
                    }
                }

                // Write to the debug stream
                debugMessagesStream->write(deviceDescriptor.getType());
                debugMessagesStream->write(": ");

                // Instantiate the driver and add it to the driver manager
                Driver* driver = GetDriver(deviceDescriptor, interruptManager);
                if(driver != 0){
                    handler->onDriverSelected(driver);
                    debugMessagesStream->write(driver->getVendorName());
                    debugMessagesStream->write(" ");
                    debugMessagesStream->write(driver->getDeviceName());
                }else{
                    listKnownDeivce(deviceDescriptor);
                }

                // New line
                debugMessagesStream->write("\n");
            }

        }
    }

}

/**
 * @details Get the device descriptor
 *
 * @param bus Bus number
 * @param device Device number
 * @param function Function number
 * @return Device descriptor
 */
PeripheralComponentInterconnectDeviceDescriptor PeripheralComponentInterconnectController::GetDeviceDescriptor(uint16_t bus, uint16_t device, uint16_t function) {
    PeripheralComponentInterconnectDeviceDescriptor result;

    result.bus          = bus;
    result.device       = device;
    result.function     = function;

    result.vendor_ID    = Read(bus, device, function, 0x00);
    result.device_ID    = Read(bus, device, function, 0x02);

    result.class_id     = Read(bus, device, function, 0x0B);
    result.subclass_id  = Read(bus, device, function, 0x0A);
    result.interface_id = Read(bus, device, function, 0x09);

    result.revision     = Read(bus, device, function, 0x8);
    result.interrupt    = Read(bus, device, function, 0x3C);

    return result;
}

/**
 * @details Get the driver for the device
 *
 * @param dev Device descriptor
 * @param interruptManager Interrupt manager
 * @return Driver for the device, 0 if there is no driver
 */
Driver* PeripheralComponentInterconnectController::GetDriver(PeripheralComponentInterconnectDeviceDescriptor dev, InterruptManager* interruptManager) {

    Driver* driver = 0;
    switch (dev.vendor_ID)
    {
        case 0x1022:    //AMD
        {
            switch (dev.device_ID)
            {
                case 0x2000:    //am79c971
                {
                    amd_am79c973* result = (amd_am79c973*)MemoryManager::activeMemoryManager ->malloc(sizeof(amd_am79c973));
                    new (result) amd_am79c973(&dev, interruptManager);
                    return result;

                }
                default:
                    break;
            }
            break;
        }
        case 0x8086:  //Intel
        {
            switch (dev.device_ID)
            {
                case 0x100E: //i217 (Ethernet Controller)
                {
                    intel_i217* result = (intel_i217*)MemoryManager::activeMemoryManager ->malloc(sizeof(intel_i217));
                    new (result) intel_i217(&dev, interruptManager);

                    return result;
                }
                default:
                    break;
            }
            break;
        }//End Intel
    }

    //If there is no driver for the particular device, go into generic devices
    switch (dev.class_id)
    {
        case 0x03: //Graphics
        {

            switch (dev.subclass_id)
            {
                case 0x00:  //VGA
                {
                    VideoGraphicsArray* result = (VideoGraphicsArray*)MemoryManager::activeMemoryManager ->malloc(sizeof(VideoGraphicsArray));
                    new (result) VideoGraphicsArray();

                    return result;
                }
            }
            break;
        }
    }

    return driver;
}


void PeripheralComponentInterconnectController::listKnownDeivce(PeripheralComponentInterconnectDeviceDescriptor dev) {
    switch (dev.vendor_ID)
    {
        case 0x1022:
        {
            // The vendor is AMD
            debugMessagesStream->write("AMD ");

            // List the device
            switch (dev.device_ID)
            {
                default:
                    debugMessagesStream->writeHex(dev.device_ID);
                    break;
            }
            break;
        }

        case 0x106B:
        {
            // The vendor is Apple
            debugMessagesStream->write("Apple ");

            // List the device
            switch (dev.device_ID)
            {
                case 0x003F:
                {
                    debugMessagesStream->write("KeyLargo/Intrepid USB");
                    break;
                }

                default:
                    debugMessagesStream->writeHex(dev.device_ID);
                    break;
            }
            break;
        }

        case 1234:
        {
            // The vendor is QEMU
            debugMessagesStream->write("QEMU ");

            // List the device
            switch (dev.device_ID)
            {

                case 0x1111:
                {
                    debugMessagesStream->write("Virtual Video Controller");
                    break;
                }
            }
            break;
        }

        case 0x8086:
        {
            // The vendor is Intel
            debugMessagesStream->write("Intel ");

            // List the device
            switch (dev.device_ID)
            {

                case 0x1237:
                {
                    debugMessagesStream->write("440FX");
                    break;
                }

                case 0x2415:
                {
                    debugMessagesStream->write("AC'97");
                    break;
                }

                case 0x7000:
                {
                    debugMessagesStream->write("PIIX3");
                    break;

                }

                case 0x7010:
                {
                    debugMessagesStream->write("PIIX4");
                    break;

                }

                case 0x7111:
                {
                    debugMessagesStream->write("PIIX3");
                    break;
                }

                case 0x7113:
                {
                    debugMessagesStream->write("PIIX4 ACPI");
                    break;
                }

                default:
                    break;

            }
            break;
        }

        case 0x80EE: {

            // The vendor is VirtualBox
            debugMessagesStream->write("VirtualBox ");

            // List the device
            switch (dev.device_ID) {

                case 0xBEEF: {
                    debugMessagesStream->write("Graphics Adapter");
                    break;
                }

                case 0xCAFE: {
                    debugMessagesStream->write("Guest Service");
                    break;
                }
            }
            break;
        }

        default:    // Unknown
            debugMessagesStream ->writeHex(dev.vendor_ID);
            debugMessagesStream ->write(" ");
            debugMessagesStream ->writeHex(dev.device_ID);
            break;

    }
}

/**
 * @details Get the base address register
 *
 * @param bus Bus number
 * @param device Device number
 * @param function Function number
 * @param barNum Base address register number
 * @return Base address register
 */
BaseAddressRegister PeripheralComponentInterconnectController::getBaseAddressRegister(common::uint16_t bus, common::uint16_t device, common::uint16_t function, common::uint16_t bar) {

    BaseAddressRegister result;

    // only types 0x00 (normal devices) and 0x01 (PCI-to-PCI bridges) are supported:
    uint32_t headerType = Read(bus,device,function,0x0E);
    if (headerType & 0x3F)
        return result;

    uint32_t bar_value = Read(bus, device, function, 0x10 + 4*bar);         // Get the offset of the base address register (Starts at 0x10)
    result.type = (bar_value & 0x1) ? InputOutput : MemoryMapping;                     // Get the type of the base address register from the last bit (Input/Output or Memory Mapping)
    result.address = (uint8_t*) (bar_value & ~0xF);                                    // Get the address of the base address register (Mask the last 4 bits)


    // Write all 1's to the register and read it back to get the size of the base address register
    Write(bus, device, function, 0x10 + 4*bar, 0xFFFFFFF0 | result.type);

    // Read the size of the base address register
    result.size = Read(bus, device, function, 0x10 + 4*bar);
    result.size = (~result.size | 0xF) + 1;                                             //Get the size of the base address register (Mask the last 4 bits) and add 1

    // Restore the original value of the base address register
    Write(bus, device, function, 0x10 + 4*bar, bar_value);

    return result;
}











