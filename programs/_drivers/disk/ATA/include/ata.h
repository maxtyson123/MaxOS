/**
 * @file ata.h
 * @brief Driver for the ATA controller, handles the reading and writing of data to the hard drive
 *
 * @date 24th October 2022
 * @author Max Tyson
 */

#ifndef MAXOS_DRIVERS_DISK_ATA_H
#define MAXOS_DRIVERS_DISK_ATA_H

#include <cstdint>


#include <libcommon/buffer.h>
#include <libcommon/outputStream.h>
#include <libcommon/port.h>
#include <libdriver/generic/disk.h>
#include <libdriver/device.h>


namespace ATADriver {

	/**
	 * @class AdvancedTechnologyAttachment
	 * @brief Driver for the ATA controller, handles the reading and writing of data to the hard drive
	 */
	class AdvancedTechnologyAttachment : public LibDriver::generic::Disk {

		private:
			MaxOS::common::Port16Bit m_data_port;
			MaxOS::common::Port8Bit m_error_port;
			MaxOS::common::Port8Bit m_sector_count_port;
			MaxOS::common::Port8Bit m_LBA_low_port;
			MaxOS::common::Port8Bit m_LBA_mid_port;
			MaxOS::common::Port8Bit m_LBA_high_Port;
			MaxOS::common::Port8Bit m_device_port;
			MaxOS::common::Port8Bit m_command_port;
			MaxOS::common::Port8Bit m_control_port;

			bool m_is_master;

			uint16_t m_bytes_per_sector =  512;

		public:
			AdvancedTechnologyAttachment(LibDriver::device_identification_t& device_id, LibDriver::HardwareCommunication::hardware_mapping_t& hmap);
			virtual ~AdvancedTechnologyAttachment();

			void read(uint32_t sector, uint8_t* data_buffer, size_t amount) final;
			void write(uint32_t sector, uint8_t* data, size_t count) final;
			void flush() final;

	};
}


#endif //MAXOS_DRIVERS_DISK_ATA_H
