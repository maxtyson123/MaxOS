/**
 * @file disk.cpp
 * @brief Implementation of a Disk driver base class
 *
 * @date 18th April 2025
 * @author Max Tyson
 */

#include <drivers/disk/disk.h>

using namespace MaxOS;
using namespace MaxOS::common;
using namespace MaxOS::drivers;
using namespace MaxOS::drivers::disk;

Disk::Disk() = default;

Disk::~Disk() = default;

/**
 * @brief Read data from the disk into a buffer (max capacity 512 bytes)
 *
 * @param sector The sector to read from
 * @param data_buffer The buffer to read the data into
 */
void Disk::read(uint32_t sector, common::buffer_t* data_buffer) {

	size_t amount = (data_buffer->capacity() > 512) ? 512 : data_buffer->capacity();
	read(sector, data_buffer, amount);

}

/**
 * @brief Read data from the disk
 *
 * @param sector The sector to read from
 * @param data_buffer The buffer to read the data into
 * @param amount The amount of data to read
 */
void Disk::read(uint32_t sector, buffer_t *data_buffer, size_t amount) {

}

/**
 * @brief Write data to the disk from a buffer (max capacity 512 bytes)
 *
 * @param sector The sector to write to
 * @param data The buffer to write the data from
 */
void Disk::write(uint32_t sector, common::buffer_t const *data) {

	size_t amount = (data->capacity() > 512) ? 512 : data->capacity();
	write(sector, data, amount);

}

/**
 * @brief Write data to the disk
 *
 * @param sector The sector to write to
 * @param data The buffer to write the data into
 * @param count The amount of data to write
 */
void Disk::write(uint32_t sector, const buffer_t* data, size_t count) {
}

/**
 * @brief Flush the disk cache
 *
 * This function is used to flush the disk cache to ensure that all data is written to the disk.
 */
void Disk::flush() {
}

/**
 * @brief Activate the disk driver
 */
void Disk::activate() {
	Driver::activate();
}

/**
 * @brief Get the device name
 *
 * @return The name of the device
 */
string Disk::device_name() {
	return "Disk";
}

/**
 * @brief Get the vendor name
 *
 * @return The name of the vendor
 */
string Disk::vendor_name() {
	return "Generic";
}