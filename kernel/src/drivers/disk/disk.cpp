//
// Created by Max Tyson on 18/04/2025.
//

#include <drivers/disk/disk.h>

using namespace MaxOS;
using namespace MaxOS::drivers;
using namespace MaxOS::drivers::disk;

Disk::Disk() = default;
Disk::~Disk() = default;


/**
 * @brief Read data from the disk
 *
 * @param sector The sector to read from
 * @param data_buffer The buffer to read the data into
 * @param amount The amount of data to read
 */
void Disk::read(uint32_t sector, uint8_t* data_buffer, size_t amount)
{
}

/**
 * @brief Write data to the disk
 *
 * @param sector The sector to write to
 * @param data_buffer The buffer to write the data into
 * @param amount The amount of data to write
 */
void Disk::write(uint32_t sector, const uint8_t* data, size_t count)
{
}

/**
 * @brief Flush the disk cache
 *
 * This function is used to flush the disk cache to ensure that all data is written to the disk.
 */
void Disk::flush()
{
}

/**
 * @brief Activate the disk driver
 */
void Disk::activate()
{
  Driver::activate();
}

/**
 * @brief Get the device name
 *
 * @return The name of the device
 */
string Disk::device_name()
{
  return "Disk";
}

/**
 * @brief Get the vendor name
 *
 * @return The name of the vendor
 */
string Disk::vendor_name()
{
  return "Generic";
}
