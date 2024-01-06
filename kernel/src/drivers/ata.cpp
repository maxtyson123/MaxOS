//
// Created by 98max on 24/10/2022.
//

#include <drivers/ata.h>

using namespace maxOS;
using namespace maxOS::common;
using namespace maxOS::hardwarecommunication;
using namespace maxOS::drivers;

AdvancedTechnologyAttachment::AdvancedTechnologyAttachment(uint16_t port_base, bool master, OutputStream*output_stream)
: m_data_port(port_base),
  m_error_port(port_base + 1),
  m_sector_count_port(port_base + 2),
  m_LBA_low_port(port_base + 3),
  m_LBA_mid_port(port_base + 4),
  m_LBA_high_Port(port_base + 5),
  m_device_port(port_base + 6),
  m_command_port(port_base + 7),
  m_control_port(port_base + 0x206),
  m_is_master(master),
  ata_message_stream(output_stream)
{

}

AdvancedTechnologyAttachment::~AdvancedTechnologyAttachment() {

}

/**
 * @brief Identify the ATA device
 */
void AdvancedTechnologyAttachment::identify() {

  // Select the device (master or slave)
  m_device_port.write(m_is_master ? 0xA0 : 0xB0);

  // Reset the HOB (High Order Byte)
  m_control_port.write(0);

  // Check if the master is present
  m_device_port.write(0xA0);
  uint8_t status = m_command_port.read();
  if(status == 0xFF){
    ata_message_stream-> write("Invalid Status");
    return;
  }

  // Select the device (master or slave)
  m_device_port.write(m_is_master ? 0xA0 : 0xB0);

  // Clear the ports
  m_sector_count_port.write(0);
  m_LBA_low_port.write(0);
  m_LBA_mid_port.write(0);
  m_LBA_high_Port.write(0);

  // Send the identify command
  m_command_port.write(0x0EC);

  // Check if the device is present
  status = m_command_port.read();
  if(status == 0x00)
    return;

  // Wait for the device to be ready or for an error to occur
  while (((status & 0x80) == 0x80)  && ((status & 0x01) != 0x01))
    status = m_command_port.read();

  //Check for any errors
  if(status & 0x01){
    ata_message_stream-> write("ERROR");
    return;
  }

  // read the data and print it
  for (uint16_t i = 0; i < 256; ++i) {
      uint16_t data = m_data_port.read();
      ata_message_stream-> write(" 0x");
      ata_message_stream-> write_hex(data);
  }
}

/**
 * @brief read a sector from the ATA device
 *
 * @param sector The sector to read
 * @param data The data to read into
 * @param count The amount of data to read from that sector
 */
void AdvancedTechnologyAttachment::read_28(uint32_t sector, uint8_t* data, int count)
{
    // Don't allow reading more then a sector
    if(sector & 0xF0000000 || count > m_bytes_per_sector)
        return;

    // Select the device (master or slave) and reset it
    m_device_port.write((m_is_master ? 0xE0 : 0xF0) |
                        ((sector & 0x0F000000) >> 24));
    m_error_port.write(0);
    m_sector_count_port.write(1);

    // Split the sector into the ports
    m_LBA_low_port.write(sector & 0x000000FF);
    m_LBA_mid_port.write((sector & 0x0000FF00) >> 8);
    m_LBA_high_Port.write((sector & 0x00FF0000) >> 16);

    // Send the read command
    m_command_port.write(0x20);

    // Make sure the device is there
    uint8_t status = m_command_port.read();
    if(status == 0x00)
      return;

    // Wait for the device to be ready or for an error to occur
    while(((status & 0x80) == 0x80) && ((status & 0x01) != 0x01))
        status = m_command_port.read();

    //Check for any errors
    if(status & 0x01)
        return;

    // read the data and store it in the array
    for(uint16_t i = 0; i < count; i+= 2)
    {
        uint16_t read_data = m_data_port.read();

        data[i] = read_data & 0x00FF;

        // Place the next byte in the array if there is one
        if(i+1 < count)
            data[i+1] = (read_data >> 8) & 0x00FF;
    }

    // read the remaining bytes
    for(uint16_t i = count + (count % 2); i < m_bytes_per_sector; i+= 2)
      m_data_port.read();
}

/**
 * @brief write to a sector on the ATA device
 *
 * @param sector The sector to write to
 * @param count The amount of data to write to that sector
 */
void AdvancedTechnologyAttachment::write_28(uint32_t sector, uint8_t* data, int count){

    // Don't allow writing more then a sector
    if(sector > 0x0FFFFFFF || count > m_bytes_per_sector)
        return;

    // Select the device (master or slave) and reset it
    m_device_port.write(m_is_master ? 0xE0
                                    : 0xF0 | ((sector & 0x0F000000) >> 24));
    m_error_port.write(0);
    m_sector_count_port.write(1);

    // Split the sector into the ports
    m_LBA_low_port.write(sector & 0x000000FF);
    m_LBA_mid_port.write((sector & 0x0000FF00) >> 8);
    m_LBA_high_Port.write((sector & 0x00FF0000) >> 16);

    // Send the write command
    m_command_port.write(0x30);

    // write the data to the device
    for (uint16_t i = 0; i < m_bytes_per_sector; i+= 2) {

        uint16_t  writeData = data[i];

        // Place the next byte in the array if there is one
        if(i+1 < count)
            writeData |= ((uint16_t)data[i+1]) << 8;

        m_data_port.write(writeData);
    }

    // write the remaining bytes
    for(int i = count + (count%2); i < m_bytes_per_sector; i += 2)
      m_data_port.write(0x0000);
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
  if(status == 0x00)
    return;


  // Wait for the device to be ready or for an error to occur
  while (((status & 0x80) == 0x80) && ((status & 0x01) != 0x01))
      status = m_command_port.read();

  if(status & 0x01)
      return;

}


