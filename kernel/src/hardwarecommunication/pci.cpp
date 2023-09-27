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
    commandPort.Write(id);             //The ID is like the adress of the port
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

                //Display INFO
               debugMessagesStream->write("    PCI BUS ");
               debugMessagesStream->writeHex(bus & 0xFF);

               debugMessagesStream->write(", DEVICE ");
               debugMessagesStream->writeHex(device & 0xFF);

               debugMessagesStream->write(", FUNCTION ");
               debugMessagesStream->writeHex(function & 0xFF);

               debugMessagesStream->write(", VENDOR ");
               debugMessagesStream->writeHex((deviceDescriptor.vendor_ID & 0xFF00) >> 8);
               debugMessagesStream->writeHex(deviceDescriptor.vendor_ID & 0xFF);

               debugMessagesStream->write(", DEVICE ");
               debugMessagesStream->writeHex((deviceDescriptor.device_ID & 0xFF00) >> 8);
               debugMessagesStream->writeHex(deviceDescriptor.device_ID & 0xFF);

                for (int barNum = 0; barNum < 6; ++barNum) {
                    BaseAdressRegister bar = GetBaseAdressRegister(bus,device,function, barNum);
                    if(bar.adress && (bar.type == InputOutput)){                                            //Only if the address is really set
                        deviceDescriptor.portBase = (uint32_t)bar.adress;                                   //The adress returned is the port number for an I/O bar (for memory mapping it would be mem adress)
                        deviceDescriptor.hasPortBase = true;
                    }else if(bar.adress && (bar.type == MemoryMapping)){
                        deviceDescriptor.memoryBase = (uint32_t)bar.adress;
                        deviceDescriptor.hasMemoryBase = true;
                    }


                }

                Driver* driver = GetDriver(deviceDescriptor, interruptManager);
                if(driver != 0){    //If there is a driver
                    driverManager->AddDriver(driver);
                }



                if(deviceDescriptor.hasMemoryBase)debugMessagesStream->write(" (MEMORY)");
                if(deviceDescriptor.hasPortBase)debugMessagesStream->write(" (I/O)");

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
        case 0x1022:
        {//AMD

            switch (dev.device_ID)
            {
                case 0x2000:
                {//am79c971

                   debugMessagesStream->write("\r    AMD am79c971");
                    //return MemoryManager::activeMemoryManager -> Instantiate<amd_am79c973>(&dev, interruptManager);
                    amd_am79c973* result = (amd_am79c973*)MemoryManager::activeMemoryManager ->malloc(sizeof(amd_am79c973));
                    new (result) amd_am79c973(&dev, interruptManager);

                    return result;
                    break;

                }//end am79c971

                default:
                    break;
            }
            break;
        }//End AMD

        case 0x106B:
        {//Apple

            switch (dev.device_ID)
            {
                case 0x003F:
                {//KeyLargo/Intrepid USB

                   debugMessagesStream->write("\r   APPLE KeyLargo/Intrepid USB (NO DRIVER)");
                    break;

                }//end KeyLargo/Intrepid USB

                default:
                    break;
            }
            break;
        }//End Apple

        case 1234:
        { //QEMU

            switch (dev.device_ID)
            {

                case 0x1111:
                {//Virtual Video Controller

                   debugMessagesStream->write("\r   QEMU Virtual Video Controller (NO DRIVER)"); //Emulated Video Card
                    break;


                }//Virtual Video Controller

            }
            break;
        }//End QEMU

        case 0x8086:
        { //Intel

            switch (dev.device_ID)
            {

                case 0x100E:
                {//i217 (Ethernet Controller)

                   debugMessagesStream->write("\r    INTEL i217");
                    intel_i217* result = (intel_i217*)MemoryManager::activeMemoryManager ->malloc(sizeof(intel_i217));
                    new (result) intel_i217(&dev, interruptManager);

                    return result;                     //Add Driver
                    break;

                }//end i217

                case 0x1237:
                {//440FX

                   debugMessagesStream->write("\r   INTEL 440FX (NO DRIVER)"); //https://en.wikipedia.org/wiki/Intel_440FX (It is a chipset, not a device)
                    break;

                }//end 440FX

                case 0x2415:
                {//AUDIO CONTROLLER

                   debugMessagesStream->write("\r   INTEL  AC'97 AUDIO CONTROLLER (NO DRIVER)"); //82801AA AC'97 Audio Controller
                    break;

                }//end AUDIO CONTROLLER

                case 0x7000:
                {//PIIX3 ISA

                   debugMessagesStream->write("\r   INTEL PIIX3 ISA (NO DRIVER)");     //https://en.wikipedia.org/wiki/PCI_IDE_ISA_Xcelerator  (IDE Controller) (SOUTH BRIDGE)
                    break;

                }//end PIIX3 ISA

                case 0x7010:
                {//PIIX4 IDE

                   debugMessagesStream->write("\r   INTEL PIIX4 IDE (NO DRIVER)");     //https://en.wikipedia.org/wiki/PCI_IDE_ISA_Xcelerator  (IDE Controller) (SOUTH BRIDGE)
                    break;

                }//end PIIX4 IDE

                case 0x7111:
                {//PIIX3 IDE

                   debugMessagesStream->write("\r   INTEL PIIX3 IDE (NO DRIVER)");     //https://en.wikipedia.org/wiki/PCI_IDE_ISA_Xcelerator  (IDE Controller) (SOUTH BRIDGE)
                    break;

                }//end PIIX3 IDE

                case 0x7113:
                {//PIIX4 ACPI

                   debugMessagesStream->write("\r   INTEL PIIX4 ACPI (NO DRIVER)");     //https://en.wikipedia.org/wiki/PCI_IDE_ISA_Xcelerator  (IDE Controller) (SOUTH BRIDGE)
                    break;

                }//end PIIX4 ACPI

                default:
                    break;

            }
            break;
        }//End Intel

        case 0x80EE: { //VIRTUALBOX

            switch (dev.device_ID) {

                case 0xBEEF: {//GRAPHICS ADAPTER

                   debugMessagesStream->write("\r   VIRTUALBOX GRAPHICS ADAPTER (NO DRIVER)");
                    break;

                }//end GRAPHICS ADAPTER

                case 0xCAFE: {//GUEST SERVICE

                   debugMessagesStream->write("\r   VIRTUALBOX GUEST SERVICE (NO DRIVER)");
                    break;

                }//end GUEST SERVICE

            }
            break;
        }//End VIRTUALBOX

    }

    //If there is no driver for the particular device, go into generic devices
    switch (dev.class_id)
    {
        case 0x03:
        {//Graphics

            switch (dev.subclass_id)
            {
                case 0x00:
                {//VGA
                   debugMessagesStream->write("\r    GRAPHICS VGA");
                    VideoGraphicsArray* result = (VideoGraphicsArray*)MemoryManager::activeMemoryManager ->malloc(sizeof(VideoGraphicsArray));
                    new (result) VideoGraphicsArray();

                    return result;                     //Add Driver
                    break;

                }//end VGA


            }
            break;

        }

    }

    return driver;
}

static unsigned int get_number_of_lowest_set_bit(uint32_t value)
{
    unsigned int  pos = 0;
    uint32_t mask = 0x00000001;
    while (!(value & mask))
    { ++pos; mask=mask<<1; }
    return pos;
}

/* searches for the highest set bit in a 32-bit value, value must not be 0 */
static unsigned int  get_number_of_highest_set_bit(uint32_t value)
{
    unsigned int  pos = 31;
    uint32_t mask = 0x80000000;
    while (!(value & mask))
    { --pos; mask=mask>>1; }
    return pos;
}

/**
 * @details Get the base adress register
 *
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
       debugMessagesStream->write("\r       ERROR: unsupported header type found! \n");
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

        result.preFetchable = ((Read(bus,device,function,barOffset) >> 3) & 0x1) == 1;              //Examine the 3rd bit (check notes: 3rd bit is prefetchable)
        switch((bar_value >> 1) & 0x3)                                                                        //Shift by one which removes the last bit (access mode), [& 0x3] gives us only the bits with 0x3
        {

            case 0:                                                                                          // 32 Bit Mode
            {
                //Check if there are any readable bits
                Write(bus,device,function,barOffset,0xFFFFFFFC);                            // overwrite with all 1s
                const uint32_t barValue = Read(bus,device,function,barOffset) & 0xFFFFFFFC;       // and read back again

                if (barValue == 0)                                                // at least one address bit must be 1 (i.e. writable).
                {
                    if (result.preFetchable)                                      // unused BARs must be completely 0 (the prefetchable bit must not be set either)
                    {
                       debugMessagesStream->write("\r       ERROR (32bit): BAR NOT WRITEABLE\n");
                        return result;
                    }

                }
                else
                {
                    const unsigned  int lowestBit = get_number_of_lowest_set_bit(barValue);
                    // it must be a valid 32-bit address :
                    if ( (get_number_of_highest_set_bit(barValue) != 31) || (lowestBit > 31) || (lowestBit < 4) )
                    {
                        //TODO: MAKE WORK WITH VIRTUALBOX / QEMUdebugMessagesStream->write("       ERROR (32bit): BAR INVALID BITS\n");
                        return result;
                    }

                    //Get the base memory address from the bar_value
                    result.adress = (uint8_t*)(bar_value  & 0xFFFFFFF0);




                }
                break;
            }                                                                                                // END 32 Bit Mode

            case 1: // 20 Bit Mode
            {
                if (headerType == 0x01)                                                // 20 Bit Mode is not supported for PCI-to-PCI bridges
                {
                   debugMessagesStream->write("\r       ERROR (20Bit): 20 BIT MODE NOT SUPPORTED\n");
                    return result;
                }


                //Check if there are any readable bits
                Write(bus,device,function,barOffset,0xFFFFFFFC);                            // overwrite with all 1s
                const uint32_t barValue = Read(bus,device,function,barOffset) & 0xFFFFFFFC;       // and read back again

                if (barValue == 0)                                                // at least one address bit must be 1 (i.e. writable).
                {
                    if (result.preFetchable)                                      // unused BARs must be completely 0 (the prefetchable bit must not be set either)
                    {
                       debugMessagesStream->write("\r       ERROR (20Bit): BAR NOT WRITEABLE\n");
                        return result;
                    }

                }
                const unsigned int lowestBit = get_number_of_lowest_set_bit(barValue);

                // it must be a valid 20-bit address :
                if ( (get_number_of_highest_set_bit(barValue) != 19) || (lowestBit > 19) || (lowestBit < 4) )
                {
                   debugMessagesStream->write("\r       ERROR (20Bit): INVAILD BTIS!\n");
                    return result;

                }


                //Get the base memory address from the bar_value
                result.adress = (uint8_t*)(bar_value  & 0xFFF0);


                break;
            }
            case 2: // 64 Bit Mode
            {
                // check whether a 64-bit BAR is even possible at the current position:
                if (bar >= (max_bars - 1)) {
                   debugMessagesStream->write("\r       ERROR (64bit): BAR CANT STOP AT LAST POS\n");
                    return result;
                }
                // non-prefetchable 64-BARs cannot be used behind bridges (? but they are not forbidden in the spec ?) :
                if ( !result.preFetchable) {
                   debugMessagesStream->write("\r       ERROR (64bit): BAR non fetchable !\n");
                    return result;
                }

                Write(bus,device,function, barOffset, 0xFFFFFFF0);                              // overwrite with all 1s
                Write(bus,device,function, barOffset + 4, 0xFFFFFFFF);                          // overwrite with all 1s
                const uint32_t barLowValue = Read(bus,device,function, barOffset) & 0xFFFFFFF0;       // and read back again
                const uint32_t barHighValue = Read(bus,device,function, barOffset + 4);               // and read back again

                unsigned int lowestBit = 0;
                if (barLowValue != 0) {
                    // less than 4 GB :

                    lowestBit = get_number_of_lowest_set_bit(barLowValue);

                    // it must be a valid small 64-bit address :
                    if ((barHighValue != 0xFFFFFFFF)                                          // upper 32 bits must be 1s
                            || (get_number_of_highest_set_bit(barLowValue) != 31)       // lower 32 bits must be 32-bit address
                            || (lowestBit > 31)                                               // lower 32 bits must be 32-bit address
                            || (lowestBit < 4))                                               // lower 32 bits must be 32-bit address
                    {
                       debugMessagesStream->write("\r       ERROR (64bit): INVALID BITS\n");
                        return result;
                    }

                }
                else
                {
                    // greater than/equal to 4 GB :
                    lowestBit = get_number_of_lowest_set_bit(barHighValue) + 32;

                    // it must be a valid large 64-bit address :
                    if ((get_number_of_highest_set_bit(barHighValue) != 31)       // upper 32 bits must be 32-bit address
                            || (lowestBit > 63)                                         // lower 32 bits must be 32-bit address
                            || (lowestBit < 36))                                        // lower 32 bits must be 32-bit address
                    {
                       debugMessagesStream->write("\r       ERROR (32bit): INVALID BITS\n");
                        return result;
                    }
                }

                //Get the base memory address from the bar_value
                result.adress = (uint8_t*)((barLowValue & 0xFFFFFFF0) + (barHighValue & 0xFFFFFFFF) << 32);



                // skip the subsequent BAR for analysis as cannot be used because it contains the upper 32 bits of the previous 64-bit BAR
                ++bar;

            }
        }

        //                   Shift bar by 3 (removing last 3 bits) check if it == 0x1

    }
    else
    {
        /** fixme: this causes the device to not initialize properly, which means mem map is also probably wrong
              Write(bus,device,function,barOffset,0xFFFFFFFC);                            // overwrite with all 1s
              const uint32_t barValue = Read(bus,device,function,barOffset) & 0xFFFFFFFC;       // and read back again

              if (barValue == 0) // at least one address bit must be 1 (i.e. writable).
              {
                 debugMessagesStream->write("       ERROR : NO WRITEABLE BITS");
                  return result;
              }

              const unsigned int  lowestBit = get_number_of_lowest_set_bit(barValue);
              const unsigned int  highestBit = get_number_of_highest_set_bit(barValue);

              // it must either be a valid 32-bit address or a valid 16-bit address :
              if ( ( (highestBit != 31) && (highestBit != 15) ) || (highestBit < lowestBit) || (lowestBit < 2) )
              {
                 debugMessagesStream->write("       ERROR : INVAlID BITS");
                  return result;
              }
              **/


        // I/O
        result.adress = (uint8_t*)(bar_value & 0xFFFFFFFC); //~0x3 - use this if things go wrong
        result.preFetchable = false;
    }

    return result;
}










