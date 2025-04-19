//
// Created by 98max on 24/10/2022.
//

#ifndef MAXOS_DRIVERS_DISK_ATA_H
#define MAXOS_DRIVERS_DISK_ATA_H

#include <common/outputStream.h>
#include <hardwarecommunication/port.h>
#include <drivers/disk/disk.h>
#include <stdint.h>


namespace MaxOS{

    namespace drivers{

        namespace disk{

            /**
             * @class AdvancedTechnologyAttachment
             * @brief Driver for the ATA controller, handles the reading and writing of data to the hard drive
             */
            class AdvancedTechnologyAttachment : public Disk {

                protected:
                    hardwarecommunication::Port16Bit m_data_port;
                    hardwarecommunication::Port8Bit m_error_port;
                    hardwarecommunication::Port8Bit m_sector_count_port;
                    hardwarecommunication::Port8Bit m_LBA_low_port;
                    hardwarecommunication::Port8Bit m_LBA_mid_port;
                    hardwarecommunication::Port8Bit m_LBA_high_Port;
                    hardwarecommunication::Port8Bit m_device_port;
                    hardwarecommunication::Port8Bit m_command_port;
                    hardwarecommunication::Port8Bit m_control_port;
                    bool m_is_master;
                    uint16_t m_bytes_per_sector { 512 };

                public:
                    AdvancedTechnologyAttachment(uint16_t port_base, bool master);
                    ~AdvancedTechnologyAttachment();

                    bool identify();
                    void read(uint32_t sector, uint8_t* data_buffer, size_t amount) final;
                    void write(uint32_t sector, const uint8_t* data, size_t count) final;
                    void flush() final;

                    void activate() final;

                    string device_name() final;
                    string vendor_name() final;
            };
       }
    }
}

#endif //MAXOS_DRIVERS_DISK_ATA_H
