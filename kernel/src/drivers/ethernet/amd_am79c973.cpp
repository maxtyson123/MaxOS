// 
//  Created by 98max on 22/10/2022.
// 

#include <drivers/ethernet/amd_am79c973.h>

using namespace MaxOS;
using namespace MaxOS::common;
using namespace MaxOS::memory;
using namespace MaxOS::drivers;
using namespace MaxOS::drivers::ethernet;
using namespace MaxOS::hardwarecommunication;

AMD_AM79C973::AMD_AM79C973(PeripheralComponentInterconnectDeviceDescriptor *dev, OutputStream *amdNetMessageStream)
: EthernetDriver(amdNetMessageStream),
  InterruptHandler(0x20 + dev -> interrupt),
  MACAddress0Port(dev ->port_base),
  MACAddress2Port(dev ->port_base + 0x02),
  MACAddress4Port(dev ->port_base + 0x04),
  registerDataPort(dev ->port_base + 0x10),
  registerAddressPort(dev ->port_base + 0x12),
  busControlRegisterDataPort(dev ->port_base + 0x16),
  resetPort(dev ->port_base + 0x14),
  initBlock(),
  sendBuffers(),
  recvBuffers()
{
    // No active buffer at the start
    currentSendBuffer = 0;
    currentRecvBuffer = 0;

    //Not active or initialized
    active = false;
    initDone = false;

    // Get the MAC addresses (split up in little endian order)
    uint64_t MAC0 = MACAddress0Port.read() % 256;
    uint64_t MAC1 = MACAddress0Port.read() / 256;
    uint64_t MAC2 = MACAddress2Port.read() % 256;
    uint64_t MAC3 = MACAddress2Port.read() / 256;
    uint64_t MAC4 = MACAddress4Port.read() % 256;
    uint64_t MAC5 = MACAddress4Port.read() / 256;

    // Combine MAC addresses into one 48 bit number
    ownMAC = MAC5 << 40
             | MAC4 << 32
             | MAC3 << 24
             | MAC2 << 16
             | MAC1 << 8
             | MAC0;

    // Set the device to 32 bit mode
    registerAddressPort.write(20);              // Tell device to write to register 20
    busControlRegisterDataPort.write(0x102);    // write desired data

    // Reset the stop bit (tell device it's not supposed to be reset now)
    registerAddressPort.write(0);               // Tell device to write to register 0
    registerDataPort.write(0x04);               // write desired data

    // Set the initialization block
    initBlock.mode = 0x0000;                         // Promiscuous mode = false   ( promiscuous mode tells it to receive all packets, not just broadcasts and those for its own MAC address)
    initBlock.reserved1 = 0;                         // Reserved
    initBlock.numSendBuffers = 3;                    // Means 8 because 2^8 (number of bits used)
    initBlock.reserved2 = 0;                         // Reserved
    initBlock.numRecvBuffers = 3;                    // Means 8 because 2^8 (number of bits used)
    initBlock.physicalAddress = ownMAC;              // Set the physical address to the MAC address
    initBlock.reserved3 = 0;                         // Reserved
    initBlock.logicalAddress = 0;                    // None for now


    //TODO: Use malloc?

    // Set Buffer descriptors memory
    sendBufferDescr = (BufferDescriptor*)(MemoryManager::kmalloc((sizeof(BufferDescriptor) * 8) + 15));  // Allocate memory for 8 buffer descriptors
    initBlock.sendBufferDescrAddress = (uint64_t)sendBufferDescr;

    recvBufferDescr = (BufferDescriptor*)(MemoryManager::kmalloc((sizeof(BufferDescriptor) * 8) + 15));  // Allocate memory for 8 buffer descriptors
    initBlock.recvBufferDescrAddress = (uint64_t)recvBufferDescr;

    for(uint8_t i = 0; i < 8; i++)
    {

        // Send buffer descriptors
        sendBufferDescr[i].address = (((uint64_t)&sendBuffers[i]) + 15 ) & ~(uint32_t)0xF;       // Same as above
        sendBufferDescr[i].flags = 0x7FF                                                         // Length of descriptor
                                   | 0xF000;                                                     // Set it to send buffer
        sendBufferDescr[i].flags2 = 0;                                                           // "Flags2" shows whether an error occurred while sending and should therefore be set to 0 by the drive
        sendBufferDescr[i].avail = 0;                                                            // IF it is in use

        // Receive
        recvBufferDescr[i].address = (((uint64_t)&recvBuffers[i]) + 15 ) & ~(uint32_t)0xF;   // Same as above
        recvBufferDescr[i].flags = 0xF7FF                                                        // Length of descriptor        (This 0xF7FF is what was causing the problem, it used to be 0x7FF)
                                   | 0x80000000;                                                 // Set it to receive buffer
        recvBufferDescr[i].flags2 = 0;                                                           // "Flags2" shows whether an error occurred while sending and should therefore be set to 0 by the drive
        recvBufferDescr[i].avail = 0;                                                            // IF it is in use
    }

    // Move initialization block into device
    registerAddressPort.write(1);                                     // Tell device to write to register 1
    registerDataPort.write((uint64_t)(&initBlock) &
                           0xFFFF);             // write address data
    registerAddressPort.write(2);                                     // Tell device to write to register 2
    registerDataPort.write(((uint64_t)(&initBlock) >> 16) &
                           0xFFFF);     // write shifted address data


}

AMD_AM79C973::~AMD_AM79C973() = default;



/**
 * @brief This function activates the device and starts it (Runs when the driver-manger calls activateAll())
 */
void AMD_AM79C973::activate()
{

    // TODO: Have a look at re - implementing this again someday
    return;

    initDone = false;                                            // Set initDone to false
    registerAddressPort.write(0);                           // Tell device to write to register 0
    registerDataPort.write(0x41);                           // Enable Interrupts and start the device
    while(!initDone);                                            // Wait for initDone to be set to true

    registerAddressPort.write(4);                           // Tell device to read from register 4
    uint32_t temp = registerDataPort.read();                     // Get current data

    registerAddressPort.write(4);                           // Tell device to write to register 4
    registerDataPort.write(temp | 0xC00);                   // Bitwise OR function on data (This automatically enlarges packets smaller than 64 bytes to that size and removes some relatively superfluous information from received packets.)

    registerAddressPort.write(0);                           // Tell device to write to register 0
    registerDataPort.write(0x42);                           // Tell device that it is initialised and can begin operating

    active = true;                                               // Set active to true
}

/**
 * @brief This function resets the device
 *
 * @return The amount of ms to wait
 */
uint32_t AMD_AM79C973::reset() {

  resetPort.read();
  resetPort.write(0);
  return 10;                      // 10 means wait for 10ms

}



/**
 * @brief This function handles the interrupt for the device
 *
 * @param esp The stack pointer (where to return to)
*/
void AMD_AM79C973::handle_interrupt() {




    // Similar to PIC, data needs to be read when an interrupt is sent, or it hangs
    registerAddressPort.write(0);                           // Tell device to read from register 0
    uint32_t temp = registerDataPort.read();                     // Get current data

    // Note: Cant be switch case as multiple errors can occur at the same time

    // Errors
    if((temp & 0x8000) == 0x8000)
      error_message("AMD am79c973 ERROR: ");
    if((temp & 0x2000) == 0x2000)
      error_message("COLLISION ERROR\n");
    if((temp & 0x1000) == 0x1000)
      error_message("MISSED FRAME\n");
    if((temp & 0x0800) == 0x0800)
      error_message("MEMORY ERROR\n");


    // Responses
    if((temp & 0x0400) == 0x0400) FetchDataReceived();
    if((temp & 0x0200) == 0x0200) FetchDataSent();
    if((temp & 0x0100) == 0x0100) initDone = true;//

    // Reply that it was received
    registerAddressPort.write(0);                           // Tell device to write to register 0
    registerDataPort.write(temp);                           // Tell device that the interrupt was received
}



// Sending a package :
// "Flags2" shows whether an error occurred while sending and should therefore be set to 0 by the driver.
//  The OWN bit must now be set in the “flags” field (0x80000000) in order to “transfer” the descriptor to the card.
//  Furthermore, STP (Start of Packet, 0x02000000) and ENP (End of Packet, 0x01000000) should be set - this indicates that the data is not split up, but that it is a single Ethernet packet.
//  Furthermore, bits 12-15 must be set (0x0000F000, are probably reserved) and bits 0-11 are negative Size of the package.
/**
 * @brief This function sends a package
 *
 * @param buffer The buffer to send
 * @param size The size of the buffer
 */
void AMD_AM79C973::DoSend(uint8_t *buffer, uint32_t size) {

    while(!active);

    int sendDescriptor = currentSendBuffer;              // Get where data has been written to
    currentSendBuffer = (currentSendBuffer + 1) % 8;    // Move send buffer to next send buffer (div by 8 so that it is cycled) (this allows for data to be sent from different m_tasks in parallel)

    if(size > 1518){                                    // If attempt to send more than 1518 bytes at once it will be too large
        size = 1518;                                    // Discard all data after that  (Generally if data is bigger than that at driver level then a higher up network layer must have made a mistake)

    }

    // What this loop does is copy the information passed as the parameter buffer (src) to the send buffer in the ram (dst) which the card will then use to send the data
    for (uint8_t *src = buffer + size -1,                                                   // Set src pointer to the end of the data that is being sent
         *dst = (uint8_t*)(sendBufferDescr[sendDescriptor].address + size -1);       // Take the buffer that has been selected
         src >= buffer;                                                             // While there is still information in the buffer that hasn't been written to src
         src--,dst--                                                                // Move 2 pointers to the end of the buffers
            )
    {
        *dst = *src;                                                                        // Copy data from source buffer to destination buffer
    }


    sendBufferDescr[sendDescriptor].avail = 0;                               // Set that this buffer is in use
    sendBufferDescr[sendDescriptor].flags2 = 0;                              // Clear any previous error messages
    sendBufferDescr[sendDescriptor].flags = 0x8300F000                       // Encode the size of what is being sent
                                            | ((uint16_t)((-size) & 0xFFF));

    registerAddressPort.write(0);                           // Tell device to write to register 0
    registerDataPort.write(
        0x48);                           // Tell device to send the data currently in the buffer
}

void AMD_AM79C973::FetchDataReceived()
{

    for(;(recvBufferDescr[currentRecvBuffer].flags & 0x80000000) == 0; currentRecvBuffer = (currentRecvBuffer+1)%8)         //Loop through all the buffers
    {
        if(!(recvBufferDescr[currentRecvBuffer].flags    & 0x40000000)                   //Check if there is an error
           && (recvBufferDescr[currentRecvBuffer].flags & 0x03000000) == 0x03000000)    //Check start and end bits of the packet
        {
            uint32_t size = recvBufferDescr[currentRecvBuffer].flags2 & 0xFFF;          //Get the size of the packet
            if (size > 64)                                                              //If the size is the size of ethernet 2 frame
                size -= 4;                                                              //remove the checksum

            auto* buffer = (uint8_t*)(recvBufferDescr[currentRecvBuffer].address);   //Get the buffer
            FireDataReceived(buffer, size);                                             //Pass data to handler
        }

        recvBufferDescr[currentRecvBuffer].flags2 = 0;                                  //write that the data has been read and can now be used again
        recvBufferDescr[currentRecvBuffer].flags = 0x8000F7FF;                          //Clear the buffer
    }
}

void AMD_AM79C973::FetchDataSent()
{

    /*
    for(;(recvBufferDescr[currentRecvBuffer].flags & 0x80000000) == 0; currentRecvBuffer = (currentRecvBuffer+1)%8)
    {
        if(!(recvBufferDescr[currentRecvBuffer].flags    & 0x40000000)
           && (recvBufferDescr[currentRecvBuffer].flags & 0x03000000) == 0x03000000)
        {
            uint32_t size = recvBufferDescr[currentRecvBuffer].flags2 & 0xFFF;
            if (size > 64)
                size -= 4;

            uint8_t* buffer = (uint8_t*)(recvBufferDescr[currentRecvBuffer].address);
            FireDataSent(buffer, size);
        }

        recvBufferDescr[currentRecvBuffer].flags2 = 0;
        recvBufferDescr[currentRecvBuffer].flags = 0x8000F7FF;
    }
    */

}


/**
 * @brief This function gets the MAC address
 *
 * @return The MAC address
 */
uint64_t AMD_AM79C973::GetMediaAccessControlAddress() {
    while(ownMAC == 0);
    return ownMAC;
}

void AMD_AM79C973::deactivate() {

}

string AMD_AM79C973::vendor_name() {
    return "AMD";
}

string AMD_AM79C973::device_name() {
    return "PCnet-Fast III (Am79C973)";
}


