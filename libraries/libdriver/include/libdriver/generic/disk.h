/**
 * @file disk.h
 * @brief Defines a generic Disk driver for reading and writing data to disk drives
 *
 * @date 18th April 2025
 * @author Max Tyson
 */

#ifndef LIBDRIVER_GENERIC_DISK_H
#define LIBDRIVER_GENERIC_DISK_H

#include <cstdint>

#include <libcommon/buffer.h>
#include <libdriver/driver.h>

namespace LibDriver::generic {

    /**
    * @struct DiskSector
    * @brief One sector of data on a disk
    */
    typedef struct DiskSector {
        uint8_t data[512];
    } disk_sector_t;

    /**
     * @class Disk
     * @brief Generic Disk, handles the reading and writing of data to the hard drive
     */
    class Disk : public Driver {

    public:
        Disk();
        ~Disk();

        void read(uint32_t sector, MaxOS::common::buffer_t* data_buffer, size_t amount = 512);
        virtual void read(uint32_t sector, uint8_t* data_buffer, size_t amount);

        void write(uint32_t sector, MaxOS::common::buffer_t* data, size_t amount = 512);
        virtual void write(uint32_t sector, uint8_t* data, size_t count);

        virtual disk_sector_t read_sector(uint32_t sector);
        virtual int write_sector(uint32_t sector, disk_sector_t data);
        virtual void flush();
    };
}


#endif //MAXOS_DRIVERS_DISK_H
