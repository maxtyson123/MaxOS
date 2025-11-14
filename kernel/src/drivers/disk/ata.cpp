/**
 * @file ata.cpp
 * @brief Implementation of the Advanced Technology Attachment (ATA) driver for disk communication
 *
 * @date 24th October 2022
 * @author Max Tyson
 */

#include <drivers/disk/ata.h>

using namespace MaxOS;
using namespace MaxOS::common;
using namespace MaxOS::hardwarecommunication;
using namespace MaxOS::drivers;
using namespace MaxOS::drivers::disk;

/**
 * @brief Constructor for the AdvancedTechnologyAttachment class
 *
 * @param port_base The base port for the ATA device
 * @param master True if the device is master, false if slave
 */
AdvancedTechnologyAttachment::AdvancedTechnologyAttachment(uint16_t port_base, bool master)
		: m_data_port(port_base),
		m_error_port(port_base + 1),
		m_sector_count_port(port_base + 2),
		m_LBA_low_port(port_base + 3),
		m_LBA_mid_port(port_base + 4),
		m_LBA_high_Port(port_base + 5),
		m_device_port(port_base + 6),
		m_command_port(port_base + 7),
		m_control_port(port_base + 0x206),
		m_is_master(master) {

}

AdvancedTechnologyAttachment::~AdvancedTechnologyAttachment() = default;

/**
 * @brief Identify the ATA device
 *
 * @return True if the device is present, false otherwise
 */
bool AdvancedTechnologyAttachment::identify() {

	// Select the device (master or slave)
	m_device_port.write(m_is_master ? 0xA0 : 0xB0);

	// Reset the High Order Byte
	m_control_port.write(0);

	// Check if the master is present
	m_device_port.write(0xA0);
	uint8_t status = m_command_port.read();
	if (status == 0xFF) {
		Logger::WARNING() << "ATA Device: Invalid status";
		return false;
	}

	// Select the device (master or slave)
	m_device_port.write(m_is_master ? 0xA0 : 0xB0);

	// Clear the ports
	m_sector_count_port.write(0);
	m_LBA_low_port.write(0);
	m_LBA_mid_port.write(0);
	m_LBA_high_Port.write(0);

	// Check if the device is present
	m_command_port.write(0x0EC);
	status = m_command_port.read();
	if (status == 0x00)
		return false;

	// Wait for the device to be ready or for an error to occur
	while (((status & 0x80) == 0x80) && ((status & 0x01) != 0x01))
		status = m_command_port.read();

	//Check for any errors
	if (status & 0x01) {
		Logger::WARNING() << "ATA Device: Error reading status\n";
		return false;
	}

	// Read the rest of the data as a whole sector needs to be read
	for (uint16_t i = 0; i < 256; ++i)
		uint16_t data = m_data_port.read();

	// Device is present and ready
	return true;
}

/**
 * @brief Read a sector from the ATA device
 *
 * @param sector The sector to read
 * @param data_buffer The data to read into
 * @param amount The amount of bytes to read from that sector
 */
void AdvancedTechnologyAttachment::read(uint32_t sector, buffer_t* data_buffer, size_t amount) {

	// Don't allow reading more than a sector
	if (sector & 0xF0000000 || amount > m_bytes_per_sector)
		return;

	// Select the device (master or slave)
	m_device_port.write((m_is_master ? 0xE0 : 0xF0) | ((sector & 0x0F000000) >> 24));

	// Device is busy @todo yeild
	while ((m_command_port.read() & 0x80) != 0);

	// Reset the device
	m_error_port.write(0);
	m_sector_count_port.write(1);

	// Split the sector into the ports
	m_LBA_low_port.write(sector & 0x000000FF);
	m_LBA_mid_port.write((sector & 0x0000FF00) >> 8);
	m_LBA_high_Port.write((sector & 0x00FF0000) >> 16);

	// Tell the device to prepare for reading
	m_command_port.write(0x20);

	// Make sure the device is there
	uint8_t status = m_command_port.read();
	if (status == 0x00)
		return;

	// Wait for the device to be ready or for an error to occur @todo Userspace block here
	while (((status & 0x80) == 0x80) && ((status & 0x01) != 0x01))
		status = m_command_port.read();

	//Check for any errors
	if (status & 0x01)
		return;

	for (size_t i = 0; i < amount; i += 2) {

		// Read from the disk (2 bytes) and store the first byte
		uint16_t read_data = m_data_port.read();
		data_buffer->write(read_data & 0x00FF);

		// Place the second byte in the array if there is one
		if (i + 1 < amount)
			data_buffer->write((read_data >> 8) & 0x00FF);
	}

	// Read the remaining bytes as a full sector has to be read
	for (uint16_t i = amount + (amount % 2); i < m_bytes_per_sector; i += 2)
		m_data_port.read();
}

/**
 * @brief write to a sector on the ATA device
 *
 * @param sector The sector to write to
 * @param data The data to write
 * @param count The amount of data to write to that sector
 */
void AdvancedTechnologyAttachment::write(uint32_t sector, buffer_t* data, size_t count) {

	// Don't allow writing more than a sector
	if (sector > 0x0FFFFFFF || count > m_bytes_per_sector)
		return;

	// Select the device (master or slave)
	m_device_port.write(m_is_master ? 0xE0 : 0xF0 | ((sector & 0x0F000000) >> 24));

	// Device is busy @todo YIELD
	while ((m_command_port.read() & 0x80) != 0);

	// Reset the device
	m_error_port.write(0);
	m_sector_count_port.write(1);

	// Split the sector into the ports
	m_LBA_low_port.write(sector & 0x000000FF);
	m_LBA_mid_port.write((sector & 0x0000FF00) >> 8);
	m_LBA_high_Port.write((sector & 0x00FF0000) >> 16);

	// Send the write command
	m_command_port.write(0x30);

	// Wait for the device be ready writing @todo YIELD
	uint8_t status = m_command_port.read();
	while ((status & 0x80) != 0 || (status & 0x08) == 0)
		status = m_command_port.read();

	// Write the data to the device
	for (size_t i = 0; i < m_bytes_per_sector; i += 2) {

		uint16_t writeData = data->read();

		// Place the next byte in the array if there is one
		if (i + 1 < count)
			writeData |= (uint16_t) (data->read()) << 8;

		m_data_port.write(writeData);
	}

	// Write the remaining bytes as a full sector has to be written
	for (int i = count + (count % 2); i < m_bytes_per_sector; i += 2)
		m_data_port.write(0x0000);

	// Wait for the device to finish writing @todo YIELD
	status = m_command_port.read();
	while ((status & 0x80) != 0 || (status & 0x08) != 0)
		status = m_command_port.read();

	flush();
}

/**
 * @brief Flush the cache of the ATA device
 */
void AdvancedTechnologyAttachment::flush() {

	// Select the device (master or slave)
	m_device_port.write(m_is_master ? 0xE0 : 0xF0);

	// Send the flush command
	m_command_port.write(0xE7);

	// Make sure the device is there
	uint8_t status = m_command_port.read();
	if (status == 0x00)
		return;

	// Wait for the device to be ready or for an error to occur
	while (((status & 0x80) == 0x80) && ((status & 0x01) != 0x01))
		status = m_command_port.read();

	// Check for an error
	if (status & 0x01)
		return;

	// ...
}

/**
 * @brief Get the device name
 *
 * @return The name of the device
 */
string AdvancedTechnologyAttachment::device_name() {
	return "Advanced Technology Attachment";
}

/**
 * @brief Get the vendor name
 *
 * @return The name of the vendor
 */
string AdvancedTechnologyAttachment::vendor_name() {
	return "IDE";
}
