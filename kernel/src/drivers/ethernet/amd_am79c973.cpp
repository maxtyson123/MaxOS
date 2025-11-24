/**
 * @file amd_am79c973.cpp
 * @brief Implementation of the AMD AM79C973 Ethernet Driver
 *
 * @date 22nd October 2022
 * @author Max Tyson
 */

#include <drivers/ethernet/amd_am79c973.h>

using namespace MaxOS;
using namespace MaxOS::common;
using namespace MaxOS::memory;
using namespace MaxOS::drivers;
using namespace MaxOS::drivers::ethernet;
using namespace MaxOS::hardwarecommunication;

/// MAX OS NET CODE:
///     All the old (this) networking code poorly written and not used, this will be moved to userspace in the future
///     but is kept here as a reference for now.

/**
 * @brief Constructs a new AMD_AM79C973 Ethernet driver, reads the MAC address and sets up the initialisation block and buffer descriptors
 * @param dev The PCI device descriptor for this device
 */
AMD_AM79C973::AMD_AM79C973(PCIDeviceDescriptor* dev)
: InterruptHandler(0x20 + dev->interrupt),
mac_address_0_port(dev->port_base),
mac_address_2_port(dev->port_base + 0x02),
mac_address_4_port(dev->port_base + 0x04),
register_data_port(dev->port_base + 0x10),
register_address_port(dev->port_base + 0x12),
bus_control_register_data_port(dev->port_base + 0x16),
reset_port(dev->port_base + 0x14),
init_block(),
send_buffers(),
recv_buffers() {
	// No active buffer at the start
	current_send_buffer = 0;
	current_recv_buffer = 0;

	//Not active or initialised
	active = false;
	init_done = false;

	///@todo move all this to initilise()

	// Get the MAC addresses (split up in little endian order)
	uint64_t mac_0 = mac_address_0_port.read() % 256;
	uint64_t mac_1 = mac_address_0_port.read() / 256;
	uint64_t mac_2 = mac_address_2_port.read() % 256;
	uint64_t mac_3 = mac_address_2_port.read() / 256;
	uint64_t mac_4 = mac_address_4_port.read() % 256;
	uint64_t mac_5 = mac_address_4_port.read() / 256;

	// Combine MAC addresses into one 48 bit number
	own_mac = mac_5 << 40
	          | mac_4 << 32
	          | mac_3 << 24
	          | mac_2 << 16
	          | mac_1 << 8
	          | mac_0;

	// Set the device to 32 bit mode
	register_address_port.write(20);              // Tell device to write to register 20
	bus_control_register_data_port.write(0x102);    // write desired data

	// Reset the stop bit (tell device it's not supposed to be reset now)
	register_address_port.write(0);               // Tell device to write to register 0
	register_data_port.write(0x04);               // write desired data

	// Set the initialization block
	init_block.mode = 0x0000;                         // Promiscuous mode = false   ( promiscuous mode tells it to receive all packets, not just broadcasts and those for its own MAC address)
	init_block.reserved1 = 0;                         // Reserved
	init_block.num_send_buffers = 3;                    // Means 8 because 2^8 (number of bits used)
	init_block.reserved2 = 0;                         // Reserved
	init_block.num_recv_buffers = 3;                    // Means 8 because 2^8 (number of bits used)
	init_block.physical_address = own_mac;              // Set the physical address to the MAC address
	init_block.reserved3 = 0;                         // Reserved
	init_block.logical_address = 0;                    // None for now

	// Set Buffer descriptors memory
	send_buffer_descr = (BufferDescriptor*) (MemoryManager::kmalloc((sizeof(BufferDescriptor) * 8) + 15));  // Allocate memory for 8 buffer descriptors
	init_block.send_buffer_descr_address = (uint64_t) send_buffer_descr;

	recv_buffer_descr = (BufferDescriptor*) (MemoryManager::kmalloc((sizeof(BufferDescriptor) * 8) + 15));  // Allocate memory for 8 buffer descriptors
	init_block.recv_buffer_descr_address = (uint64_t) recv_buffer_descr;

	for(uint8_t i = 0; i < 8; i++) {

		// Send buffer descriptors
		send_buffer_descr[i].address = (((uint64_t) &send_buffers[i]) + 15) & ~(uint32_t) 0xF;       // Same as above
		send_buffer_descr[i].flags =
		0x7FF                                                         // Length of descriptor
		| 0xF000;                                                     // Set it to send buffer
		send_buffer_descr[i].flags2 = 0;                                                           // "Flags2" shows whether an error occurred while sending and should therefore be set to 0 by the drive
		send_buffer_descr[i].avail = 0;                                                            // IF it is in use

		// Receive
		recv_buffer_descr[i].address = (((uint64_t) &recv_buffers[i]) + 15) & ~(uint32_t) 0xF;   // Same as above
		recv_buffer_descr[i].flags =
		0xF7FF                                                        // Length of descriptor        (This 0xF7FF is what was causing the problem, it used to be 0x7FF)
		| 0x80000000;                                                 // Set it to receive buffer
		recv_buffer_descr[i].flags2 = 0;                                                           // "Flags2" shows whether an error occurred while sending and should therefore be set to 0 by the drive
		recv_buffer_descr[i].avail = 0;                                                            // IF it is in use
	}

	// Move initialization block into device
	register_address_port.write(1);                                     // Tell device to write to register 1
	register_data_port.write((uint64_t) (&init_block) &
	                         0xFFFF);             // write address data
	register_address_port.write(2);                                     // Tell device to write to register 2
	register_data_port.write(((uint64_t) (&init_block) >> 16) &
	                         0xFFFF);     // write shifted address data


}

AMD_AM79C973::~AMD_AM79C973() = default;


/**
 * @brief This function activates the device and starts it (Runs when the driver-manger calls activateAll())
 *
 * @todo Re-implement this class
 */
void AMD_AM79C973::activate() {

	return;

	init_done = false;                                            // Set initDone to false
	register_address_port.write(0);                           // Tell device to write to register 0
	register_data_port.write(0x41);                           // Enable Interrupts and start the device
	while(!init_done);                                            // Wait for initDone to be set to true

	register_address_port.write(4);                           // Tell device to read from register 4
	uint32_t temp = register_data_port.read();                     // Get current data

	register_address_port.write(4);                           // Tell device to write to register 4
	register_data_port.write(temp |
	                         0xC00);                   // Bitwise OR function on data (This automatically enlarges packets smaller than 64 bytes to that size and removes some relatively superfluous information from received packets.)

	register_address_port.write(0);                           // Tell device to write to register 0
	register_data_port.write(
	0x42);                           // Tell device that it is initialised and can begin operating

	active = true;                                               // Set active to true
}

/**
 * @brief This function resets the device
 *
 * @return The amount of ms to wait
 */
uint32_t AMD_AM79C973::reset() {

	reset_port.read();
	reset_port.write(0);
	return 10;                      // 10 means wait for 10ms

}


/**
 * @brief This function handles the interrupt for the device
 *
*/
void AMD_AM79C973::handle_interrupt() {

	// Similar to PIC, data needs to be read when an interrupt is sent, or it hangs
	register_address_port.write(0);                           // Tell device to read from register 0
	uint32_t temp = register_data_port.read();                     // Get current data

	// Note: Cant be switch case as multiple errors can occur at the same time

	// Errors
	if((temp & 0x8000) == 0x8000)
		Logger::WARNING() << "AMD am79c973 ERROR: ";
	if((temp & 0x2000) == 0x2000)
		Logger::WARNING() << "COLLISION ERROR\n";
	if((temp & 0x1000) == 0x1000)
		Logger::WARNING() << "MISSED FRAME\n";
	if((temp & 0x0800) == 0x0800)
		Logger::WARNING() << "MEMORY ERROR\n";


	// Responses
	if((temp & 0x0400) == 0x0400) fetch_data_received();
	if((temp & 0x0200) == 0x0200) fetch_data_sent();
	if((temp & 0x0100) == 0x0100) init_done = true;//

	// Reply that it was received
	register_address_port.write(0);                           // Tell device to write to register 0
	register_data_port.write(temp);                           // Tell device that the interrupt was received
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
void AMD_AM79C973::do_send(uint8_t* buffer, uint32_t size) {

	while(!active);

	int send_descriptor = current_send_buffer;              // Get where data has been written to
	current_send_buffer = (current_send_buffer + 1) %
	                      8;    // Move send buffer to next send buffer (div by 8 so that it is cycled) (this allows for data to be sent from different m_tasks in parallel)

	if(size >
	   1518) {                                    // If attempt to send more than 1518 bytes at once it will be too large
		size = 1518;                                    // Discard all data after that  (Generally if data is bigger than that at driver level then a higher up network layer must have made a mistake)

	}

	// What this loop does is copy the information passed as the parameter buffer (src) to the send buffer in the ram (dst) which the card will then use to send the data
	for(uint8_t* src = buffer + size -
	                   1,                                                   // Set src pointer to the end of the data that is being sent
	    * dst = (uint8_t*) (send_buffer_descr[send_descriptor].address + size -
	                        1);       // Take the buffer that has been selected
	    src >=
	    buffer;                                                             // While there is still information in the buffer that hasn't been written to src
	    src--, dst--                                                                // Move 2 pointers to the end of the buffers
	) {
		*dst = *src;                                                                        // Copy data from source buffer to destination buffer
	}


	send_buffer_descr[send_descriptor].avail = 0;                               // Set that this buffer is in use
	send_buffer_descr[send_descriptor].flags2 = 0;                              // Clear any previous error messages
	send_buffer_descr[send_descriptor].flags = 0x8300F000                       // Encode the size of what is being sent
	                                           | ((uint16_t) ((-size) & 0xFFF));

	register_address_port.write(0);                           // Tell device to write to register 0
	register_data_port.write(
	0x48);                           // Tell device to send the data currently in the buffer
}

void AMD_AM79C973::fetch_data_received() {

	for(; (recv_buffer_descr[current_recv_buffer].flags & 0x80000000) == 0; current_recv_buffer =
	                                                                        (current_recv_buffer + 1) %
	                                                                        8)         //Loop through all the buffers
	{
		if(!(recv_buffer_descr[current_recv_buffer].flags & 0x40000000)                   //Check if there is an error
		   && (recv_buffer_descr[current_recv_buffer].flags & 0x03000000) ==
		      0x03000000)    //Check start and end bits of the packet
		{
			uint32_t size = recv_buffer_descr[current_recv_buffer].flags2 & 0xFFF;          //Get the size of the packet
			if(size >
			   64)                                                              //If the size is the size of ethernet 2 frame
				size -= 4;                                                              //remove the checksum

			auto* buffer = (uint8_t*) (recv_buffer_descr[current_recv_buffer].address);   //Get the buffer
			fire_data_received(buffer, size);                                             //Pass data to handler
		}

		recv_buffer_descr[current_recv_buffer].flags2 = 0;                                  //write that the data has been read and can now be used again
		recv_buffer_descr[current_recv_buffer].flags = 0x8000F7FF;                          //Clear the buffer
	}
}

void AMD_AM79C973::fetch_data_sent() {

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
uint64_t AMD_AM79C973::get_media_access_control_address() {
	while(own_mac == 0);
	return own_mac;
}

void AMD_AM79C973::deactivate() {

}

string AMD_AM79C973::vendor_name() {
	return "AMD";
}

string AMD_AM79C973::device_name() {
	return "PCnet-Fast III (Am79C973)";
}


