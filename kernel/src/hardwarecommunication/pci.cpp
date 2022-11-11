//
// Created by 98max on 12/10/2022.
//
#include <hardwarecommunication/pci.h>
#include <drivers/amd_am79c973.h>

using namespace maxOS::common;
using namespace maxOS::hardwarecommunication;
using namespace maxOS::drivers;

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

/**
 * Read data from the PCI Controller
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
    commandPort.Write(id);             //The ID is like the adress of the port
    uint32_t result = dataPort.Read();
    return result >> (8* (registeroffset % 4));

}

/**
 * Write data to the PCI Controller
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
 * Check if the device has a function
 * @param bus Bus number
 * @param device Device number
 * @return true if the device has a function
 */
bool PeripheralComponentInterconnectController::DeviceHasFunctions(uint16_t bus, uint16_t device) {

    //0x0E
    return Read(bus,device,0,0x0E) & (1<<7);    //Read address, the 7th bit of it returns if there is a function
}

/**
 * Select the driver for the device
 * @param driverManager device driver manager
 * @param interruptManager Interrupt manager
 * @return Driver for the device
 */
void PeripheralComponentInterconnectController::SelectDrivers(DriverManager* driverManager, InterruptManager* interruptManager) {
    for (int bus = 0; bus < 8; ++bus) {

        for (int device = 0; device < 32; ++device) {

            int numFunctions = (DeviceHasFunctions(bus,device)) ? 8 : 1;

            for (int function = 0; function < numFunctions; ++function) {
                PeripheralComponentInterconnectDeviceDescriptor deviceDescriptor = GetDeviceDescriptor(bus, device, function);

                //If there is no device then vendor ID is 0x0000, If the device is not ready to react then the vendor ID is 0x0001   (REMOVE 0xFFFF if things go wrong)
                if(deviceDescriptor.vendor_ID == 0x0000 || deviceDescriptor.vendor_ID == 0x0001 || deviceDescriptor.vendor_ID == 0xFFFF){
                    continue;
                }

                for (int barNum = 0; barNum < 6; ++barNum) {
                    BaseAdressRegister bar = GetBaseAdressRegister(bus,device,function, barNum);
                    if(bar.adress && (bar.type == InputOutput)){ //Only if the address is really set
                        deviceDescriptor.portBase = (uint32_t)bar.adress;  //The adress returned is the port number for an I/O bar (for memory mapping it would be mem adress)
                    }
                }

                //Only need one driver for the device, not every BAR
                Driver* driver = GetDriver(deviceDescriptor, interruptManager);
                if(driver != 0){    //If there is a driver
                    driverManager->AddDriver(driver);
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

/**
 * Get the device descriptor
 * @param bus Bus number
 * @param device Device number
 * @param function Function number
 * @return Device descriptor
 */
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

/**
 * Get the driver for the device
 * @param dev Device descriptor
 * @param interruptManager Interrupt manager
 * @return Driver for the device, 0 if there is no driver
 */
Driver* PeripheralComponentInterconnectController::GetDriver(PeripheralComponentInterconnectDeviceDescriptor dev, InterruptManager* interruptManager) {

    Driver* driver = 0;
    switch (dev.vendor_ID) {
        case 0x1022:                            //AMD
            switch (dev.device_ID) {
                case 0x2000:                    //AMD - am79c971 (Ethernet Controller)
                    driver = (amd_am79c973*)MemoryManager::activeMemoryManager->malloc(sizeof(amd_am79c973));       //Allocate memory region of the sie of the class
                    if(driver != 0)                                                                                     //Check if space in memory
                       new (driver) amd_am79c973(&dev, interruptManager);                                               //Create Driver Instance
                    return driver;                                                                                      //Add Driver
                    break;
            }
            break;
        case 0x8089:                            //Intel
            break;
    }

    //If there is no driver for the particular device, go into generic devices
    switch (dev.class_id) {
        case 0x03:                              //Graphics
            switch (dev.subclass_id) {
                case 0x00:                      //Graphics - VGA
                    break;
            }
            break;
    }
    
    
    
    return driver;
}


/**
 * Get the lowest set bit in a 32-bit value
 * @param value 32-bit value
 * @return Lowest set bit
 */
uint32_t get_number_of_lowest_set_bit(uint32_t value)
{
    uint32_t pos = 0;
    uint32_t mask = 0x00000001;
    while (!(value & mask))
    { ++pos; mask=mask<<1; }
    return pos;
}

/**
 * Get the highest set bit in a 32-bit value
 * @param value  32-bit value
 * @return  Highest set bit
 */
uint32_t get_number_of_highest_set_bit(uint32_t value)
{
    uint32_t pos = 31;
    uint32_t mask = 0x80000000;
    while (!(value & mask))
    { --pos; mask=mask>>1; }
    return pos;
}

/**
 * Get the base adress register
 * @param bus Bus number
 * @param device Device number
 * @param function Function number
 * @param barNum Base adress register number
 * @return Base adress register
 */
BaseAdressRegister PeripheralComponentInterconnectController::GetBaseAdressRegister(uint16_t bus, uint16_t device, uint16_t function, uint16_t bar) {




    BaseAdressRegister result;

    uint32_t headerType = Read(bus,device,function,0x0E) & 0x7F;  //Only get first 7 bits

    if (headerType >= 0x02)  // only types 0x00 (normal devices) and 0x01 (PCI-to-PCI bridges) are supported:
    {
        printf("ERROR: unsupported header type found! \n");
        return result;
    }

    const int max_bars = 6 - (headerType * 4); // 6 BARs for type 0x00 and 2 BARs for type 0x01:
    if(bar >= max_bars){
        return result;
    }


    const uint32_t barOffset = 0x10 + (bar * 4);                            // Determine the offset of the current BAR (note: bar addresses begin at register 0x10, bars have the size of 4)
    uint32_t bar_value = Read(bus,device,function,barOffset);
    result.type = (bar_value & 0x1) ? InputOutput : MemoryMapping;          //Examine the last bit (check notes: last bit is type)
    uint32_t temp;

    if(result.type == MemoryMapping){
        result.preFetchable = ((bar_value >> 3) & 0x1) == 0x1;
        switch((bar_value >> 1) & 0x3) //Shift by one which removes the last bit (access mode), [& 0x3] gives us only the bits with 0x3
        {
            case 0:                                                                                          // 32 Bit Mode
            {
                Write(bus,device,function,barOffset,0xFFFFFFF0);                            // overwrite with all 1s
                const uint32_t barValue = Read(bus,device,function,barOffset) & 0xFFFFFFF0;       // and read back again
                if (barValue == 0)                                                                          // at least one address bit must be 1 (i.e. writable).
                {
                    if (result.preFetchable)                                                                // unused BARs must be completely 0 (the prefetchable bit must not be set either)
                    {
                        printf("ERROR : 32-bit memory BAR"); printf(reinterpret_cast<char *>(bar)); printf("contains no writable address bits!\n");
                        return result;
                    }

                    // Output BAR info :
                     //  printf("BAR: "); printf(reinterpret_cast<char *>(bar)); printf("is unused!\n");
                }
                else
                {
                    const uint32_t lowestBit = get_number_of_lowest_set_bit(barValue);
                    // it must be a valid 32-bit address :
                    if ( (get_number_of_highest_set_bit(barValue) != 31) || (lowestBit > 31) || (lowestBit < 4) )
                    {
                        printf("ERROR : 32-bit memory BAR %u contains invalid writable address bits!\n",bar);
                        return result;
                    }

                    // Output BAR info :
                   // printf("BAR: "); printf(reinterpret_cast<char *>(bar)); printf(" works (32Bit)!\n");

                }
            }
                break;
            case 1: // 20 Bit Mode
            case 2: // 64 Bit Mode
                break;
        }

        //                   Shift bar by 3 (removing last 3 bits) check if it == 0x1

    }
    else
    {  // I/O
        result.adress = (uint8_t*)(bar_value & ~0x3); //~0x3 = cancle last 2 bits
        result.preFetchable = false;
    }

    return result;
}










