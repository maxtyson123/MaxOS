//
// Created by 98max on 1/01/2023.
//

#ifndef MAXOS_FILESYSTEM_FAT32_H
#define MAXOS_FILESYSTEM_FAT32_H

#include "drivers/disk/ata.h"
#include <common/outputStream.h>
#include <common/vector.h>
#include <filesystem/filesystem.h>
#include <memory/memorymanagement.h>
#include <stdint.h>

namespace maxOS{

    namespace filesystem{

         /**
         * @struct BiosParameterBlock
         * @brief Stores information about the FAT32 filesystem
         */
        struct BiosParameterBlock32{

            uint8_t     jump[3];
            uint8_t     OEM_name[8];
            uint16_t    bytes_per_sector;
            uint8_t     sectors_per_cluster;
            uint16_t    reserved_sectors;
            uint8_t     table_copies;
            uint16_t    root_entries;
            uint16_t    total_sectors_16;
            uint8_t     media_type;
            uint16_t    fat_sector_count;
            uint16_t    sectors_per_track;
            uint16_t    head_count;
            uint32_t    hidden_sectors;
            uint32_t    total_sectors_32;

            uint32_t    table_size_32;
            uint16_t    extended_flags;
            uint16_t    fat_version;
            uint32_t    root_cluster;
            uint16_t    fat_info;
            uint16_t    backup_sector;
            uint8_t     reserved0[12];
            uint8_t     drive_number;
            uint8_t     reserved_1;
            uint8_t     boot_signature;
            uint32_t    uint_32;
            uint8_t     volume_label[11];
            uint8_t     file_system_type[8];

        } __attribute__((packed));

        /**
         * @struct DirectoryEntry
         * @brief Stores information about a file or directory
         */
        struct DirectoryEntry{

            uint8_t     name[8];
            uint8_t     extension[3];
            uint8_t     attributes;
            uint8_t     reserved;

            uint8_t     creation_time_tenth;
            uint16_t    creation_time;
            uint16_t    creation_date;
            uint16_t    last_access_date;

            uint16_t    first_cluster_high;

            uint16_t    last_write_time;
            uint16_t    last_write_date;

            uint16_t    first_cluster_low;

            uint32_t    size;

        } __attribute__((packed));


        // TODO: Redo FAT32
  }
}

#endif //MAXOS_FILESYSTEM_FAT32_H
