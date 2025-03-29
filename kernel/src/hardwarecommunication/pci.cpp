//
// Created by 98max on 12/10/2022.
//
#include <hardwarecommunication/pci.h>
#include <drivers/ethernet/amd_am79c973.h>
#include <drivers/ethernet/intel_i217.h>
#include <common/kprint.h>


using namespace MaxOS;
using namespace MaxOS::common;
using namespace MaxOS::hardwarecommunication;
using namespace MaxOS::drivers;
using namespace MaxOS::drivers::ethernet;
using namespace MaxOS::drivers::video;
using namespace MaxOS::memory;

///__DESCRIPTOR___

PeripheralComponentInterconnectDeviceDescriptor::PeripheralComponentInterconnectDeviceDescriptor() {

}

PeripheralComponentInterconnectDeviceDescriptor::~PeripheralComponentInterconnectDeviceDescriptor() {

}

/**
 * @brief Get the type of the device
 * @return Type of the device as a string (or Unknown if the type is not known)
 */
string PeripheralComponentInterconnectDeviceDescriptor::get_type() {
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
        case 0x02: return "Network";
        case 0x03: return "Display";
        case 0x04:
            switch(subclass_id)
            {
                case 0x00:  return "Video";
                case 0x01:
                case 0x03:  return "Audio";
                default:    return "Multimedia";
            }
        case 0x06:
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

PeripheralComponentInterconnectController::PeripheralComponentInterconnectController()
: m_data_port(0xCFC),
  m_command_port(0xCF8)
{

}

PeripheralComponentInterconnectController::~PeripheralComponentInterconnectController() {

}

/**
 * @brief read data from the PCI Controller
 *
 * @param bus Bus number
 * @param device Device number
 * @param function Function number
 * @param registeroffset Register offset
 * @return data from the PCI Controller
 */
uint32_t PeripheralComponentInterconnectController::read(uint16_t bus, uint16_t device, uint16_t function, uint32_t registeroffset) {

    // Calculate the id
    uint32_t id = 0x1 << 31
                | ((bus & 0xFF) << 16)
                | ((device & 0x1F) << 11)
                | ((function & 0x07) << 8)
                | (registeroffset & 0xFC);
    m_command_port.write(id);

    // read the data from the port
    uint32_t result = m_data_port.read();
    return result >> (8* (registeroffset % 4));

}

/**
 * @brief write data to the PCI Controller
 *
 * @param bus Bus number
 * @param device Device number
 * @param function Function number
 * @param registeroffset Register offset
 * @param value Value to write
 */
void PeripheralComponentInterconnectController::write(uint16_t bus, uint16_t device, uint16_t function, uint32_t registeroffset, uint32_t value) {

    // Calculate the id
    uint32_t id = 0x1 << 31
                  | ((bus & 0xFF) << 16)
                  | ((device & 0x1F) << 11)
                  | ((function & 0x07) << 8)
                  | (registeroffset & 0xFC);
    m_command_port.write(id);

    // write the data to the port
    m_data_port.write(value);
}

/**
 * @brief Check if the device has a function
 *
 * @param bus Bus number
 * @param device Device number
 * @return true if the device has a function
 */
bool PeripheralComponentInterconnectController::device_has_functions(uint16_t bus, uint16_t device) {

    return read(bus, device, 0, 0x0E) & (1<<7);
}

/**
 * @brief Select the driver for the device
 *
 * @param driverManager device driver manager
 * @param interrupt_manager Interrupt manager
 * @return Driver for the device
 */
void PeripheralComponentInterconnectController::select_drivers(DriverSelectorEventHandler *handler, hardwarecommunication::InterruptManager *interrupt_manager)
{
    for (int bus = 0; bus < 8; ++bus) {
        for (int device = 0; device < 32; ++device) {

            int numFunctions = (device_has_functions(bus, device)) ? 8 : 1;

            for (int function = 0; function < numFunctions; ++function) {

                // Get the device descriptor, if the vendor id is 0x0000 or 0xFFFF, the device is not present/ready
                PeripheralComponentInterconnectDeviceDescriptor deviceDescriptor = get_device_descriptor(bus, device, function);
                if(deviceDescriptor.vendor_id == 0x0000 || deviceDescriptor.vendor_id == 0x0001 || deviceDescriptor.vendor_id == 0xFFFF)
                    continue;


                // Get port number
                for(int barNum = 5; barNum >= 0; barNum--){
                    BaseAddressRegister bar = get_base_address_register(bus, device, function, barNum);
                    if(bar.address && (bar.type == InputOutput))
                        deviceDescriptor.port_base = (uint64_t )bar.address;
                }

                // write to the debug stream
                _kprintf("DEVICE FOUND: %s - ", deviceDescriptor.get_type().c_str());

                // Select the driver and print information about the device
                Driver* driver = get_driver(deviceDescriptor, interrupt_manager);
                if(driver != nullptr){
                  handler->on_driver_selected(driver);
                  _kprintf("%h %s %s", driver->get_vendor_name().c_str(), driver->get_device_name().c_str());
                }else{
                  list_known_device(deviceDescriptor);
                }

                // New line
                _kprintf("%h\n");
            }
        }
    }
}

/**
 * @brief Get the device descriptor
 *
 * @param bus Bus number
 * @param device Device number
 * @param function Function number
 * @return Device descriptor
 */
PeripheralComponentInterconnectDeviceDescriptor PeripheralComponentInterconnectController::get_device_descriptor(uint16_t bus, uint16_t device, uint16_t function) {
    PeripheralComponentInterconnectDeviceDescriptor result;

    result.bus          = bus;
    result.device       = device;
    result.function     = function;

    result.vendor_id    = read(bus, device, function, 0x00);
    result.device_id    = read(bus, device, function, 0x02);

    result.class_id     = read(bus, device, function, 0x0B);
    result.subclass_id  = read(bus, device, function, 0x0A);
    result.interface_id = read(bus, device, function, 0x09);

    result.revision     = read(bus, device, function, 0x8);
    result.interrupt    = read(bus, device, function, 0x3C);

    return result;
}

/**
 * @brief Get the driver for the device
 *
 * @param dev Device descriptor
 * @param interrupt_manager Interrupt manager
 * @return Driver for the device, 0 if there is no driver
 */
Driver* PeripheralComponentInterconnectController::get_driver(PeripheralComponentInterconnectDeviceDescriptor dev, InterruptManager*interrupt_manager) {

    //TODO: Bullshit, do use new. Don't use new here, manually allocate memory instead

    Driver* driver = 0;
    switch (dev.vendor_id)
    {
        case 0x1022:    //AMD
        {
            switch (dev.device_id)
            {
                case 0x2000:    //am79c971
                {
                    amd_am79c973* result = (amd_am79c973*)MemoryManager::kmalloc(sizeof(amd_am79c973));
                    new (result) amd_am79c973(&dev, interrupt_manager);
                    return result;

                }
                default:
                    break;
            }
            break;
        }
        case 0x8086:  //Intel
        {
            switch (dev.device_id)
            {
                case 0x100E: //i217 (Ethernet Controller)
                {
                    intel_i217* result = (intel_i217*)MemoryManager::kmalloc(sizeof(intel_i217));
                    new (result) intel_i217(&dev, interrupt_manager);
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
                    VideoGraphicsArray* result = (VideoGraphicsArray*)MemoryManager::kmalloc(sizeof(VideoGraphicsArray));
                    new (result) VideoGraphicsArray();
                    return result;
                }
            }
            break;
        }
    }

    return driver;
}


void PeripheralComponentInterconnectController::list_known_device(PeripheralComponentInterconnectDeviceDescriptor dev) {
    switch (dev.vendor_id)
    {
        case 0x1022:
        {
            // The vendor is AMD
            _kprintf("%hAMD ");

            // List the device
            switch (dev.device_id)
            {
                default:
                  _kprintf("%h0x%x", dev.device_id);
                  break;
            }
            break;
        }

        case 0x106B:
        {
            // The vendor is Apple
            _kprintf("%hApple ");

            // List the device
            switch (dev.device_id)
            {
                case 0x003F:
                {
                  _kprintf("%hKeyLargo/Intrepid USB");
                  break;
                }

                default:
                  _kprintf("%h0x%x", dev.device_id);
                  break;
            }
            break;
        }

        case 1234:
        {
            // The vendor is QEMU
          _kprintf("%hQEMU ");

            // List the device
            switch (dev.device_id)
            {

                case 0x1111:
                {
                  _kprintf("%hVirtual Video Controller");
                  break;
                }
            }
            break;
        }

        case 0x8086:
        {
            // The vendor is Intel
            _kprintf("%hIntel ");

            // List the device
            switch (dev.device_id)
            {

                case 0x1237:
                {
                  _kprintf("%h440FX");
                  break;
                }

                case 0x2415:
                {
                  _kprintf("%hAC'97");
                  break;
                }

                case 0x7000:
                {
                  _kprintf("%hPIIX3");
                  break;

                }

                case 0x7010:
                {
                  _kprintf("%hPIIX4");
                  break;

                }

                case 0x7111:
                {
                  _kprintf("%hPIIX3 ACPI");
                  break;
                }

                case 0x7113:
                {
                  _kprintf("%hPIIX4 ACPI");
                  break;
                }

                default:
                    _kprintf("%h0x%x", dev.device_id);
                    break;

            }
            break;
        }

        case 0x80EE: {

            // The vendor is VirtualBox
            _kprintf("%hVirtualBox ");

            // List the device
            switch (dev.device_id) {

                case 0xBEEF: {
                  _kprintf("%hGraphics Adapter");
                  break;
                }

                case 0xCAFE: {
                  _kprintf("%hGuest Service");
                  break;
                }
            }
            break;
        }

        default:    // Unknown
          _kprintf("%hUnknown (0x%x:0x%x)", dev.vendor_id, dev.device_id);
          break;

    }
}

/**
 * @brief Get the base address register
 *
 * @param bus Bus number
 * @param device Device number
 * @param function Function number
 * @param barNum base address register number
 * @return base address register
 */
BaseAddressRegister PeripheralComponentInterconnectController::get_base_address_register(uint16_t bus, uint16_t device, uint16_t function, uint16_t bar) {

    BaseAddressRegister result;

    // only types 0x00 (normal devices) and 0x01 (PCI-to-PCI bridges) are supported:
    uint32_t headerType = read(bus, device, function, 0x0E);
    if (headerType & 0x3F)
        return result;

    // read the base address register
    uint64_t bar_value = read(bus, device, function, 0x10 + 4 * bar);
    result.type = (bar_value & 0x1) ? InputOutput : MemoryMapping;
    result.address = (uint8_t*) (bar_value & ~0xF);

    // read the size of the base address register
    write(bus, device, function, 0x10 + 4 * bar, 0xFFFFFFF0 | result.type);
    result.size = read(bus, device, function, 0x10 + 4 * bar);
    result.size = (~result.size | 0xF) + 1;

    // Restore the original value of the base address register
    write(bus, device, function, 0x10 + 4 * bar, bar_value);

    return result;
}