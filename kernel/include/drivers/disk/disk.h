/**
 * @file disk.h
 * @brief Defines a generic Disk driver for reading and writing data to disk drives
 *
 * @date 18th April 2025
 * @author Max Tyson
 */

#ifndef MAXOS_DRIVERS_DISK_H
#define MAXOS_DRIVERS_DISK_H

#include <common/outputStream.h>
#include <common/buffer.h>
#include <drivers/driver.h>
#include <cstdint>


namespace MaxOS::drivers::disk {

	/**
	 * @class Disk
	 * @brief Generic Disk, handles the reading and writing of data to the hard drive
	 */
	class Disk : public Driver {

		public:
			Disk();
			~Disk();

			void read(uint32_t sector, common::buffer_t* data_buffer);
			virtual void read(uint32_t sector, common::buffer_t* data_buffer, size_t amount);

			void write(uint32_t sector, common::buffer_t* data);
			virtual void write(uint32_t sector, common::buffer_t* data, size_t count);

			virtual void flush();

			void activate() override;

			string device_name() override;
			string vendor_name() override;
	};
}


#endif //MAXOS_DRIVERS_DISK_H
