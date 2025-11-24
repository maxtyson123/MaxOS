/**
 * @file intel_i217.cpp
 * @brief Implementation of the Intel I217 Ethernet Driver
 *
 * @date 29th November 2022
 * @author Max Tyson
 */

#include <drivers/ethernet/intel_i217.h>

using namespace MaxOS;
using namespace MaxOS::common;
using namespace MaxOS::drivers;
using namespace MaxOS::drivers::ethernet;
using namespace MaxOS::hardwarecommunication;
using namespace memory;

/// MAX OS NET CODE:
///     All the old (this) networking code poorly written and not used, this will be moved to userspace in the future
///     but is kept here as a reference for now.
///
///     See OSDEV wiki for the credit for this driver

// Buffer Sizes
#define buffer8192                 ((2 << 16) | (1 << 25))


/**
 * @brief Constructs a new Intel I217 Ethernet driver. Gets the MAC address and clears the receive descriptor array
 *
 * @param device_descriptor The PCI device descriptor for this device
 */
IntelI217::IntelI217(PCIDeviceDescriptor* device_descriptor)
		: InterruptHandler(0x20 + device_descriptor->interrupt) {

	//Set the registers
	control_register = 0x0000;
	status_register = 0x0008;
	eprom_register = 0x0014;
	control_ext_register = 0x0018;
	interrupt_mask_register = 0x00D0;

	receive_control_register = 0x0100;
	receive_descriptor_low_register = 0x2800;
	receive_descriptor_high_register = 0x2804;
	receive_descriptor_length_register = 0x2808;
	receive_descriptor_head_register = 0x2810;
	receive_descriptor_tail_register = 0x2818;

	send_control_register = 0x0400;
	send_descriptor_low_register = 0x3800;
	send_descriptor_high_register = 0x3804;
	send_descriptor_length_register = 0x3808;
	send_descriptor_head_register = 0x3810;
	send_descriptor_tail_register = 0x3818;

	// Get BAR0 type, io_base address and MMIO base address
	bar_type = 1; // deviceDescriptor -> has_memory_base ? 0 : 1;  @todo Fix memory mapping from PCI as it is unable to get MAC from memory
	port_base = device_descriptor->port_base;
	//TODO: memBase = deviceDescriptor -> memory_base;

	init_done = false;
	active = false;

	// Clear eprom
	eprom_present = detect_ee_prom();


	if (read_mac_address()) {
		own_mac = create_media_access_control_address(mac_address[0], mac_address[1], mac_address[2], mac_address[3],
		                                              mac_address[4], mac_address[5]);

	} else {
		ASSERT(false, "ERROR, INIT FAILED, MAC ADDRESS NOT FOUND");
	}

	for (int i = 0; i < 0x80; i++)               //Loop through all the registers
		write(0x5200 + i * 4, 0);     //Clear the receive descriptor array




}

IntelI217::~IntelI217() = default;


void IntelI217::write(uint16_t address, uint32_t data) const {

	//Note: These Ports/MemIO cant be init in the constructor like they normally would as it depends on wether the device is using IO or MemIO, and checking that in every function would be messy

	if (bar_type == 0) {                                             // If the base address register is memory mapped

		MemIO32Bit data_mem(mem_base + address);              // Create a 32-bit memory class at the address
		data_mem.write(data);                                       // write the data to the memory address

	} else {

		Port32Bit command_port(port_base);            // Create a 32 bit port at the address
		Port32Bit data_port(port_base + 4);           // Create a 32 bit port at the address + 4

		command_port.write(address);                     // write the address to the command port
		data_port.write(data);                                // write the data to the data port


	}

}

uint32_t IntelI217::read(uint16_t address) const {

	//Note: These Ports/MemIO cant be init in the constructor like they normally would as it depends on wether the device is using IO or MemIO, and checking that in every function would be messy
	if (bar_type == 0) {                                             // If the base address register is memory mapped

		MemIO32Bit data_mem(mem_base + address);               // Create a 32-bit memory class at the address
		return data_mem.read();                                      // read the data from the memory address

	} else {

		Port32Bit command_port(port_base);            // Create a 32 bit port at the address
		Port32Bit data_port(port_base + 4);           // Create a 32 bit port at the address + 4

		command_port.write(address);                     // write the address to the command port
		return data_port.read();                              // read the data from the data port

	}

}

bool IntelI217::detect_ee_prom() {

	uint32_t val = 0;                                   // The value to be returned
	write(eprom_register, 0x1);              // Set the register to read the EEProm

	for (int i = 0; i < 1000; i++)     //Loop 1000 times or until the EEProm is detected
	{
		val = read(0x0014);                    // read the register

		if (val & 0x10)
			return true;

	}
	return false;
}

uint32_t IntelI217::eeprom_read(uint8_t addr) {
	uint16_t data = 0;                                                              // The data to be returned
	uint32_t tmp = 0;                                                               // A temporary variable
	if (eprom_present)                                                              // If the EEProm is detected
	{
		write(eprom_register, (1) | ((uint32_t) (addr) << 8));         // write the address to the register
		while (!((tmp = read(eprom_register)) & (1 << 4)));                 // Wait for the EEProm to be ready
	} else {
		write(eprom_register, (1) | ((uint32_t) (addr) << 2));        // write the address to the register
		while (!((tmp = read(eprom_register)) & (1 << 1)));                // Wait for the EEProm to be ready
	}
	data = (uint16_t) ((tmp >> 16) & 0xFFFF);                                      // Get the data from the register
	return data;                                                                  // Return the data
}

bool IntelI217::read_mac_address() {
	if (eprom_present)                                                                //If the EPROM exists
	{
		uint32_t temp;

		temp = eeprom_read(
				0);                                                   //read the m_first_memory_chunk 16 bits of the MAC address
		mac_address[0] = temp &
		                 0xff;                                                  //Get the m_first_memory_chunk 8 bits of the MAC address
		mac_address[1] =
				temp >> 8;                                                   //Get the second 8 bits of the MAC address

		temp = eeprom_read(
				1);                                                  //read the second 16 bits of the MAC address
		mac_address[2] =
				temp & 0xff;                                                  //Get the third 8 bits of the MAC address
		mac_address[3] =
				temp >> 8;                                                   //Get the fourth 8 bits of the MAC address

		temp = eeprom_read(
				2);                                                  //read the third 16 bits of the MAC address
		mac_address[4] =
				temp & 0xff;                                                  //Get the fifth 8 bits of the MAC address
		mac_address[5] =
				temp >> 8;                                                   //Get the sixth 8 bits of the MAC address
	} else    //If there is no eprom then read from memory
	{


		auto* mem_base_mac_8 = (uint8_t*) (mem_base +
		                                   0x5400);                   //Get the base address of the MAC address
		auto* mem_base_mac_32 = (uint32_t*) (mem_base + 0x5400);                //Get the base address of the MAC address

		if (mem_base_mac_32[0] != 0) {
			for (int i = 0; i < 6; i++)                                            //Loop through the MAC address
				mac_address[i] = mem_base_mac_8[i];                                //Get the MAC address

		} else return false;
	}
	return true;
}

void IntelI217::receive_init() {

	uint8_t* ptr;                                                                                                          //A pointer to the memory
	receive_descriptor_t* descs;                                                                                        //A pointer to the receive descriptors
	ptr = (uint8_t*) (MemoryManager::kmalloc(
			sizeof(receive_descriptor_t) * 32 + 16));           //Allocate memory for the receive descriptors
	descs = (receive_descriptor_t*) ptr;                                                                                //Set the pointer to the receive descriptors

	for (int i = 0; i < 32; i++) {
		receive_dsrctrs[i] = (receive_descriptor_t*) ((uint8_t*) descs + i * 16);
		receive_dsrctrs[i]->buffer_address = (uint64_t) (uint8_t*) (MemoryManager::kmalloc(8192 + 16));
		receive_dsrctrs[i]->status = 0;
	}

	//write the send descriptor list address to the register
	write(send_descriptor_low_register, (uint32_t) ((uint64_t) ptr >> 32));
	write(send_descriptor_high_register, (uint32_t) ((uint64_t) ptr & 0xFFFFFFFF));

	//write the receive descriptor list address to the register
	write(receive_descriptor_low_register, (uint64_t) ptr);
	write(receive_descriptor_high_register, 0);

	//Set the receive descriptor list length
	write(receive_descriptor_length_register, 32 * 16);


	write(receive_descriptor_head_register,
	      0);                                                                       //Set the head to 0
	write(receive_descriptor_tail_register,
	      32 - 1);                                                                    //Set the tail to 32-1

	current_receive_buffer = 0;                                                                                                   //Set the current receive buffer to 0

	write(receive_control_register, (1 << 1)    // Receiver Enable
	                              | (1 << 2)         // Store Bad Packets
	                              | (1 << 3)         // Uni cast Promiscuous Enabled
	                              | (1 << 4)        // Multicast Promiscuous Enabled
	                              | (0 << 6)        // No Loop back
	                              | (0 << 8)        // Free Buffer Threshold is 1/2 of RDLEN
	                              | (1 << 15)       // Broadcast Accept Mode
	                              | (1 << 26)       // Strip Ethernet CRC
	                              | buffer8192
	);

	delete ptr;

}

void IntelI217::send_init() {

	uint8_t* ptr;                                                                                                          //A pointer to the memory
	send_descriptor_t* descs;                                                                                           //A pointer to the send descriptors
	ptr = (uint8_t*) (MemoryManager::kmalloc(
			sizeof(send_descriptor_t) * 8 + 16));                //Allocate memory for the send descriptors
	descs = (send_descriptor_t*) ptr;                                                                                   //Set the pointer to the send descriptors


	for (int i = 0; i < 8; i++) {
		send_dsrctrs[i] = (send_descriptor_t*) ((uint8_t*) descs + i * 16);
		send_dsrctrs[i]->buffer_address = 0;
		send_dsrctrs[i]->cmd = 0;
		send_dsrctrs[i]->status = (1 << 0);    // Descriptor Done
	}

	//write the send descriptor list address to the register
	write(send_descriptor_high_register, (uint32_t) ((uint64_t) ptr >> 32));
	write(send_descriptor_low_register, (uint32_t) ((uint64_t) ptr & 0xFFFFFFFF));


	//now setup total length of descriptors
	write(send_descriptor_length_register, 8 * 16);


	//setup numbers
	write(send_descriptor_head_register, 0);
	write(send_descriptor_tail_register, 0);

	current_send_buffer = 0;

	write(send_control_register, (1 << 1)    // Transmit Enable
	                           | (1 << 3)                 // Pad Short Packets
	                           | (15 << 4)                // Collision Threshold
	                           | (64 << 12)              // Collision Distance
	                           | (1 << 24)                // Re-transmit on Late Collision
	);


	// In the case of I217 (id = 0x0410) and 82577LM (id = 0x10EA) packets will not be sent if the TCTRL is not configured using the following bits.
	// write(sendControlRegister,  0b0110000000000111111000011111010);
	//write(0x0410,  0x0060200A);

	delete ptr;

}

void IntelI217::activate() {


	//Enable interrupts
	write(interrupt_mask_register, 0x1F6DC);                     //Enable all interrupts
	write(interrupt_mask_register, 0xff & ~4);                   //Enable all interrupts except link status change
	read(0xc0);                                                     //Clear all interrupts

	//while (!initDone);                                           //Wait for the init to be done

	//Initialise the send and receive descriptors
	receive_init();
	send_init();

	active = true;                                               // Set active to true

}

void IntelI217::handle_interrupt() {

	write(interrupt_mask_register, 0x1);      //Clear the interrupt or it will hang
	uint32_t temp = read(0xc0);                //read the interrupt status register

	// if(temp & 0x04)
	//   m_driver_message_stream-> write("INTEL i217 START LINK");//initDone = true;
	//
	// if(temp & 0x10)
	//   m_driver_message_stream-> write("INTEL i217 GOOD THRESHOLD");

	if (temp & 0x80) fetch_data_received();
}

void IntelI217::fetch_data_received() {


	uint16_t old_cur;

	while ((receive_dsrctrs[current_receive_buffer]->status & 0x1)) {
		auto* buffer = (uint8_t*) receive_dsrctrs[current_receive_buffer]->buffer_address;
		uint16_t size = receive_dsrctrs[current_receive_buffer]->length;

		if (size > 64) {          // If the size is the size of ethernet 2 frame
			size -= 4;          // remove the checksum
		}

		fire_data_received(buffer, size);  //Pass data to handler


		receive_dsrctrs[current_receive_buffer]->status = 0;

		old_cur = current_receive_buffer;                         //Save the current receive buffer
		current_receive_buffer = (current_receive_buffer + 1) % 32; //Increment the current receive buffer

		write(receive_descriptor_tail_register, old_cur); //write the old current receive buffer to the tail register
	}

}

void IntelI217::do_send(uint8_t* buffer, uint32_t size) {

	while (!active);

	//Put params into send buffer
	send_dsrctrs[current_send_buffer]->buffer_address = (uint64_t) buffer;
	send_dsrctrs[current_send_buffer]->length = size;

	//Set the commands
	send_dsrctrs[current_send_buffer]->cmd = (1 << 0)    // End of Packet
	                                      | (1 << 1)    // Insert FCS
	                                      | (1 << 3)    // Report Status
			;

	send_dsrctrs[current_send_buffer]->status = 0;

	uint8_t old_cur = current_send_buffer;                                    //Save the current send buffer
	current_send_buffer = (current_send_buffer + 1) % 8;                        //Increment the current send buffer
	write(send_descriptor_tail_register, current_send_buffer);       //write the current send buffer to the tail register

	//Wait for the packet to be sent
	while (!(send_dsrctrs[old_cur]->status & 0xff));

}

uint64_t IntelI217::get_media_access_control_address() {
	while (own_mac == 0);
	return own_mac;

}

uint32_t IntelI217::reset() {
	return Driver::reset();
}

void IntelI217::deactivate() {
	Driver::deactivate();
}

string IntelI217::vendor_name() {
	return "Intel";
}

string IntelI217::device_name() {
	return "E1000 (i217)";
}


