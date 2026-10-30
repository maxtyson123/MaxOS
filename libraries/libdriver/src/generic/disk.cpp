/**
 * @file disk.cpp
 * @brief Implementation of a Disk driver base class
 *
 * @date 18th April 2025
 * @author Max Tyson
 *
 * @todo split > 512 into multiple calls / error
 */

#include <libdriver/generic/disk.h>

using namespace LibDriver::generic;
using namespace MaxOS;
using namespace MaxOS::common;

Disk::Disk() = default;

Disk::~Disk() = default;

/**
 * @brief read data from the disk into a buffer (max capacity 512 bytes)
 *
 * @param sector The sector to read from
 * @param data_buffer The buffer to read the data into
 *
 */
void Disk::read(uint32_t sector, buffer_t* data_buffer, size_t amount) {

    // Only read as much as the buffer can still hold
    size_t remaining = data_buffer->capacity() > data_buffer->offset() ? data_buffer->capacity() - data_buffer->offset() : 0;
    size_t length = (amount > 512) ? 512 : amount;
    if (length > remaining)
        length = remaining;

    // Read the sector, then place it at the buffer's current offset
    uint8_t sector_data[512] = {};
    read(sector, sector_data, length);
    data_buffer->copy_from(sector_data, length);

}

/**
 * @brief read data from the disk
 *
 * @param sector The sector to read from
 * @param data_buffer The buffer to read the data into
 * @param amount The amount of data to read
 */
void Disk::read(uint32_t sector, uint8_t* data_buffer, size_t amount) {

    disk_sector_t result = read_sector(sector);
    memcpy(data_buffer, result.data, amount > 512 ? 512 : amount);

}

/**
 * @brief write data to the disk from a buffer (max capacity 512 bytes)
 *
 * @param sector The sector to write to
 * @param data The buffer to write the data from
 */
void Disk::write(uint32_t sector, buffer_t* data, size_t amount) {

    // Only write what the buffer actually holds from its current offset
    size_t remaining = data->capacity() > data->offset() ? data->capacity() - data->offset() : 0;
    size_t length = (amount > 512) ? 512 : amount;
    if (length > remaining)
        length = remaining;

    // Take the data from the buffer's current offset
    uint8_t sector_data[512] = {};
    data->copy_to(sector_data, length);
    write(sector, sector_data, length);

}

/**
 * @brief write data to the disk
 *
 * @param sector The sector to write to
 * @param data The buffer to write the data into
 * @param count The amount of data to write
 */
void Disk::write(uint32_t sector, uint8_t* data, size_t count) {

    disk_sector_t out = {};
    memcpy(out.data, data, count > 512 ? 512 : count);
    write_sector(sector, out);

}

/**
 * @brief Read one whole sector by value. The ATA driver (which overrides read) answers the RPC through this.
 */
disk_sector_t Disk::read_sector(uint32_t sector) {

    disk_sector_t result = {};
    read(sector, result.data, 512);
    return result;
}

int Disk::write_sector(uint32_t sector, disk_sector_t data) {

    write(sector, data.data, 512);
    return 0;
}

/**
 * @brief Flush the disk cache
 *
 * This function is used to flush the disk cache to ensure that all data is written to the disk.
 */
void Disk::flush() {
}