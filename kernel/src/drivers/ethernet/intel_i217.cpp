//
// Created by 98max on 11/29/2022.
//

#include <drivers/ethernet/intel_i217.h>

using namespace MaxOS;
using namespace MaxOS::common;
using namespace MaxOS::drivers;
using namespace MaxOS::drivers::ethernet;
using namespace MaxOS::hardwarecommunication;
using namespace memory;

// Buffer Sizes
#define buffer256                  (3 << 16)
#define buffer512                  (2 << 16)
#define buffer1024                 (1 << 16)
#define buffer2048                 (0 << 16)
#define buffer4096                 ((3 << 16) | (1 << 25))
#define buffer8192                 ((2 << 16) | (1 << 25))
#define buffer16384                ((1 << 16) | (1 << 25))

///__DRIVER___

intel_i217::intel_i217(PeripheralComponentInterconnectDeviceDescriptor *deviceDescriptor)
: EthernetDriver(nullptr),
  InterruptHandler(0x20 + deviceDescriptor->interrupt)
{

    //Set the registers
    controlRegister = 0x0000;
    statusRegister = 0x0008;
    epromRegister = 0x0014;
    controlExtRegister = 0x0018;
    interruptMaskRegister = 0x00D0;

    receiveControlRegister = 0x0100;
    receiveDescriptorLowRegister = 0x2800;
    receiveDescriptorHighRegister = 0x2804;
    receiveDescriptorLengthRegister = 0x2808;
    receiveDescriptorHeadRegister = 0x2810;
    receiveDescriptorTailRegister = 0x2818;

    sendControlRegister = 0x0400;
    sendDescriptorLowRegister = 0x3800;
    sendDescriptorHighRegister = 0x3804;
    sendDescriptorLengthRegister = 0x3808;
    sendDescriptorHeadRegister = 0x3810;
    sendDescriptorTailRegister = 0x3818;

    // Get BAR0 type, io_base address and MMIO base address
    bar_type = 1; // deviceDescriptor -> has_memory_base ? 0 : 1;  //TODO: Fix memory mapping from PCI as it is unable to get MAC from memory
    portBase = deviceDescriptor ->port_base;
    //TODO: memBase = deviceDescriptor -> memory_base;

    initDone = false;
    active = false;

    //Clear eprom
    epromPresent = false;

    //Detect eprom
    detectEEProm ();

    if (readMACAddress()){

        ownMAC = CreateMediaAccessControlAddress(macAddress[0], macAddress[1], macAddress[2], macAddress[3], macAddress[4], macAddress[5]);

    }else{

      error_message("ERROR, INIT FAILED, MAC ADDRESS NOT FOUND");
        while (true);
    }

    for(int i = 0; i < 0x80; i++)               //Loop through all the registers
        Write(0x5200 + i*4, 0);     //Clear the receive descriptor array




}

intel_i217::~intel_i217() = default;


void intel_i217::Write(uint16_t address, uint32_t data) const {

    //Note: These Ports/MemIO cant be init in the constructor like they normally would as it depends on wether the device is using IO or MemIO, and checking that in every function would be messy

    if(bar_type == 0) {                                             // If the base address register is memory mapped

        MemIO32Bit dataMem(memBase + address);              // Create a 32-bit memory class at the address
        dataMem.write(data);                                       // write the data to the memory address

    } else {

        Port32Bit commandPort(portBase);            // Create a 32 bit port at the address
        Port32Bit dataPort(portBase + 4);           // Create a 32 bit port at the address + 4

        commandPort.write(address);                     // write the address to the command port
        dataPort.write(data);                                // write the data to the data port


    }

}

uint32_t intel_i217::Read(uint16_t address) const {

    //Note: These Ports/MemIO cant be init in the constructor like they normally would as it depends on wether the device is using IO or MemIO, and checking that in every function would be messy
    if(bar_type == 0) {                                             // If the base address register is memory mapped

        MemIO32Bit dataMem(memBase + address);               // Create a 32-bit memory class at the address
        return dataMem.read();                                      // read the data from the memory address

    } else{

        Port32Bit commandPort(portBase);            // Create a 32 bit port at the address
        Port32Bit dataPort(portBase + 4);           // Create a 32 bit port at the address + 4

        commandPort.write(address);                     // write the address to the command port
        return dataPort.read();                              // read the data from the data port

    }

}

bool intel_i217::detectEEProm() {

    uint32_t val = 0;                                   // The value to be returned
    Write(epromRegister, 0x1);              // Set the register to read the EEProm

    for(int i = 0; i < 1000 && !epromPresent; i++)     //Loop 1000 times or until the EEProm is detected
    {
        val = Read( 0x0014);                    // read the register

        if(val & 0x10)                                 // If the EEProm is detected
            epromPresent = true;
        else                                           // If the EEProm is not detected
            epromPresent = false;
    }
    return epromPresent;
}

uint32_t intel_i217::eepromRead( uint8_t addr)
{
    uint16_t data = 0;                                                              // The data to be returned
    uint32_t tmp = 0;                                                               // A temporary variable
    if ( epromPresent)                                                              // If the EEProm is detected
    {
        Write( epromRegister, (1) | ((uint32_t)(addr) << 8) );         // write the address to the register
        while( !((tmp = Read(epromRegister)) & (1 << 4)) );                 // Wait for the EEProm to be ready
    }
    else
    {
        Write( epromRegister, (1) | ((uint32_t)(addr) << 2) );        // write the address to the register
        while( !((tmp = Read(epromRegister)) & (1 << 1)) );                // Wait for the EEProm to be ready
    }
    data = (uint16_t)((tmp >> 16) & 0xFFFF);                                      // Get the data from the register
    return data;                                                                  // Return the data
}

bool intel_i217::readMACAddress() {
    if ( epromPresent)                                                                //If the EPROM exists
    {
        uint32_t temp;

        temp = eepromRead(0);                                                   //read the m_first_memory_chunk 16 bits of the MAC address
        macAddress[0] = temp &0xff;                                                  //Get the m_first_memory_chunk 8 bits of the MAC address
        macAddress[1] = temp >> 8;                                                   //Get the second 8 bits of the MAC address

        temp = eepromRead( 1);                                                  //read the second 16 bits of the MAC address
        macAddress[2] = temp &0xff;                                                  //Get the third 8 bits of the MAC address
        macAddress[3] = temp >> 8;                                                   //Get the fourth 8 bits of the MAC address

        temp = eepromRead( 2);                                                  //read the third 16 bits of the MAC address
        macAddress[4] = temp &0xff;                                                  //Get the fifth 8 bits of the MAC address
        macAddress[5] = temp >> 8;                                                   //Get the sixth 8 bits of the MAC address
    }
    else    //If there is no eprom then read from memory
    {


        auto * mem_base_mac_8 = (uint8_t *) (memBase+0x5400);                   //Get the base address of the MAC address
        auto * mem_base_mac_32 = (uint32_t *) (memBase+0x5400);                //Get the base address of the MAC address

        if ( mem_base_mac_32[0] != 0 )
        {
            for(int i = 0; i < 6; i++)                                            //Loop through the MAC address
                macAddress[i] = mem_base_mac_8[i];                                //Get the MAC address

        }
        else return false;
    }
    return true;
}

void intel_i217::receiveInit() {

    uint8_t * ptr;                                                                                                          //A pointer to the memory
    struct receiveDescriptor *descs;                                                                                        //A pointer to the receive descriptors
    ptr = (uint8_t *)(MemoryManager::kmalloc(sizeof(struct receiveDescriptor)*32 + 16));           //Allocate memory for the receive descriptors
    descs = (struct receiveDescriptor *)ptr;                                                                                //Set the pointer to the receive descriptors

    for(int i = 0; i < 32; i++)
    {
        receiveDsrctrs[i] = (struct receiveDescriptor *)((uint8_t *)descs + i*16);
        receiveDsrctrs[i] -> bufferAddress = (uint64_t)(uint8_t *)(MemoryManager::kmalloc(8192 + 16));
        receiveDsrctrs[i] -> status = 0;
    }

    //write the send descriptor list address to the register
    Write(sendDescriptorLowRegister, (uint32_t)((uint64_t)ptr >> 32) );
    Write(sendDescriptorHighRegister, (uint32_t)((uint64_t)ptr & 0xFFFFFFFF));

    //write the receive descriptor list address to the register
    Write(receiveDescriptorLowRegister, (uint64_t)ptr);
    Write(receiveDescriptorHighRegister, 0);

    //Set the receive descriptor list length
    Write(receiveDescriptorLengthRegister, 32 * 16);


    Write(receiveDescriptorHeadRegister, 0);                                                                       //Set the head to 0
    Write(receiveDescriptorTailRegister, 32-1);                                                                    //Set the tail to 32-1

    currentReceiveBuffer = 0;                                                                                                   //Set the current receive buffer to 0

    Write(receiveControlRegister, (1 << 1)    // Receiver Enable
                                       | (1 << 2)         // Store Bad Packets
                                       | (1 << 3)         // Uni cast Promiscuous Enabled
                                       |  (1 << 4)        // Multicast Promiscuous Enabled
                                       |  (0 << 6)        // No Loop back
                                       |  (0 << 8)        // Free Buffer Threshold is 1/2 of RDLEN
                                       |  (1 << 15)       // Broadcast Accept Mode
                                       |  (1 << 26)       // Strip Ethernet CRC
                                       | buffer8192
                                       );

}

void intel_i217::sendInit() {

    uint8_t * ptr;                                                                                                          //A pointer to the memory
    struct sendDescriptor *descs;                                                                                           //A pointer to the send descriptors
    ptr = (uint8_t *)(MemoryManager::kmalloc(sizeof(struct sendDescriptor)*8 + 16));                //Allocate memory for the send descriptors
    descs = (struct sendDescriptor *)ptr;                                                                                   //Set the pointer to the send descriptors


    for(int i = 0; i < 8; i++)
    {
        sendDsrctrs[i] = (struct sendDescriptor *)((uint8_t*)descs + i*16);
        sendDsrctrs[i] -> bufferAddress = 0;
        sendDsrctrs[i] -> cmd = 0;
        sendDsrctrs[i] -> status = (1 << 0);    // Descriptor Done
    }

    //write the send descriptor list address to the register
    Write(sendDescriptorHighRegister, (uint32_t)((uint64_t)ptr >> 32) );
    Write(sendDescriptorLowRegister, (uint32_t)((uint64_t)ptr & 0xFFFFFFFF));


    //now setup total length of descriptors
    Write(sendDescriptorLengthRegister, 8 * 16);


    //setup numbers
    Write( sendDescriptorHeadRegister, 0);
    Write( sendDescriptorTailRegister, 0);

    currentSendBuffer = 0;

    Write(sendControlRegister,  (1 << 1)    // Transmit Enable
                             | (1 << 3)                 // Pad Short Packets
                             | (15 << 4)                // Collision Threshold
                             | (64 <<  12)              // Collision Distance
                             | (1 << 24)                // Re-transmit on Late Collision
                             );


    // In the case of I217 (id = 0x0410) and 82577LM (id = 0x10EA) packets will not be sent if the TCTRL is not configured using the following bits.
    // write(sendControlRegister,  0b0110000000000111111000011111010);
    //write(0x0410,  0x0060200A);

}

void intel_i217::activate() {

  m_driver_message_stream-> write("Activating Intel i217\n");

    //Enable interrupts
    Write(interruptMaskRegister ,0x1F6DC);                     //Enable all interrupts
    Write(interruptMaskRegister ,0xff & ~4);                   //Enable all interrupts except link status change
    Read(0xc0);                                                     //Clear all interrupts

    //while (!initDone);                                           //Wait for the init to be done

    //Initialise the send and receive descriptors
    receiveInit();
    sendInit();

    active = true;                                               // Set active to true
    m_driver_message_stream-> write("Intel i217 INIT DONE\n");

}

void intel_i217::handle_interrupt() {

    Write(interruptMaskRegister, 0x1);      //Clear the interrupt or it will hang
    uint32_t temp = Read(0xc0);                //read the interrupt status register

    m_driver_message_stream-> write("Interrupt from INTEL i217");

    if(temp & 0x04)
      m_driver_message_stream-> write("INTEL i217 START LINK");//initDone = true;
    if(temp & 0x10)
      m_driver_message_stream-> write("INTEL i217 GOOD THRESHOLD");
    if(temp & 0x80) FetchDataReceived();
}

void intel_i217::FetchDataReceived() {

  m_driver_message_stream-> write("Fetching data... ");

    uint16_t old_cur;

    while((receiveDsrctrs[currentReceiveBuffer] -> status & 0x1))
    {
        auto *buffer = (uint8_t *)receiveDsrctrs[currentReceiveBuffer] -> bufferAddress;
        uint16_t size = receiveDsrctrs[currentReceiveBuffer] -> length;

        if(size > 64){          // If the size is the size of ethernet 2 frame
            size -= 4;          // remove the checksum
        }

        FireDataReceived(buffer, size);  //Pass data to handler


        receiveDsrctrs[currentReceiveBuffer]->status = 0;

        old_cur = currentReceiveBuffer;                         //Save the current receive buffer
        currentReceiveBuffer = (currentReceiveBuffer + 1) % 32; //Increment the current receive buffer

        Write(receiveDescriptorTailRegister, old_cur ); //write the old current receive buffer to the tail register
    }

}

void intel_i217::DoSend(uint8_t* buffer, uint32_t size) {

  m_driver_message_stream-> write("Sending package... ");
    while(!active);

    //Put params into send buffer
    sendDsrctrs[currentSendBuffer] -> bufferAddress = (uint64_t)buffer;
    sendDsrctrs[currentSendBuffer] -> length = size;

    //Set the commands
    sendDsrctrs[currentSendBuffer] -> cmd = (1 << 0)    // End of Packet
                                          | (1 << 1)    // Insert FCS
                                          | (1 << 3)    // Report Status
                                          ;

    sendDsrctrs[currentSendBuffer] -> status = 0;

    uint8_t old_cur = currentSendBuffer;                                    //Save the current send buffer
    currentSendBuffer = (currentSendBuffer + 1) % 8;                        //Increment the current send buffer
    Write(sendDescriptorTailRegister, currentSendBuffer);       //write the current send buffer to the tail register

    //Wait for the packet to be sent
    while(!(sendDsrctrs[old_cur]->status & 0xff));
    m_driver_message_stream-> write(" Done\n");

}

uint64_t intel_i217::GetMediaAccessControlAddress() {
  m_driver_message_stream-> write("Getting MAC address... ");
    while(ownMAC == 0);
    return ownMAC;

}

uint32_t intel_i217::reset() {
    return Driver::reset();
}

void intel_i217::deactivate() {
    Driver::deactivate();
}

string intel_i217::vendor_name() {
    return "Intel";
}

string intel_i217::device_name() {
    return "E1000 (i217)";
}


