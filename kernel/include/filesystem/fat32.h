//
// Created by 98max on 1/01/2023.
//

#ifndef MAXOS_FILESYSTEM_FAT32_H
#define MAXOS_FILESYSTEM_FAT32_H

#include <drivers/disk/disk.h>
#include <filesystem/filesystem.h>
#include <stdint.h>

namespace MaxOS{

    namespace filesystem{

         /**
         * @struct BiosParameterBlock
         * @brief Stores information about the FAT32 filesystem
         */
        typedef struct BiosParameterBlock32{

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

        } __attribute__((packed)) bpb32_t;

        /**
         * @struct FileSystemInfo
         * @brief Stores extra information about the FAT32 filesystem
         */
        typedef struct FSInfo
        {
            uint32_t    lead_signature;
            uint8_t     reserved1[480];
            uint32_t    structure_signature;
            uint32_t    free_cluster_count;
            uint32_t    next_free_cluster;
            uint8_t     reserved2[12];
            uint32_t    trail_signature;
        } __attribute__((packed)) fs_info_t;

        /**
         * @struct DirectoryEntry
         * @brief Stores information about a file or directory
         */
        typedef struct DirectoryEntry{

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

        } __attribute__((packed)) dir_entry_t;

        typedef struct LongFileNameEntry
        {
            uint8_t     order;
            uint16_t     name1[5];
            uint8_t     attributes;
            uint8_t     type;
            uint8_t     checksum;
            uint16_t    name2[6];
            uint16_t    zero;
            uint16_t    name3[2];

        } __attribute__((packed)) long_file_name_entry_t;

        typedef uint32_t lba_t;

        enum class ClusterState
        {
            FREE            = 0x00000000,
            BAD             = 0x0FFFFFF7,
            END_OF_CHAIN    = 0x0FFFFFF8,
        };

        /**
         * @class Fat32Volume
         * @brief Handles the FAT table that stores the information about the files on the disk and operations on the disk
         */
        class Fat32Volume
        {
            public:
                Fat32Volume(drivers::disk::Disk* disk, lba_t partition_offset);
                ~Fat32Volume();

                bpb32_t bpb;
                fs_info_t fsinfo;

                uint8_t  sectors_per_cluster;
                uint16_t bytes_per_sector;

                size_t   fat_size;
                size_t   fat_total_clusters;
                lba_t    fat_first_sector;
                lba_t    fat_lba;
                lba_t    fat_info_lba;

                lba_t    data_lba;
                lba_t    root_lba;

                drivers::disk::Disk* disk;

                lba_t next_cluster(lba_t cluster);
                lba_t set_next_cluster(lba_t cluster, lba_t next_cluster);
                lba_t find_free_cluster();

                lba_t allocate_cluster(lba_t cluster);
                lba_t allocate_cluster(lba_t cluster, size_t amount);

                void free_cluster(lba_t cluster);
                void free_cluster(lba_t cluster, size_t amount);
        };

        /**
         * @class Fat32File
         * @brief Handles the file operations on the FAT32 filesystem
         */
        class Fat32File : public File
        {

            private:
                Fat32Volume* m_volume;

                lba_t m_first_cluster;

            public:
                Fat32File(Fat32Volume* volume, lba_t cluster, size_t size, const string& name);
                ~Fat32File() final;

                void write(const uint8_t* data, size_t size) final;
                void read(uint8_t* data, size_t size) final;
                void flush() final;

                lba_t first_cluster() const { return m_first_cluster; }
        };

        enum class DirectoryEntryType
        {
            LAST   = 0x00,
            DIRECTORY = 0x10,
            FILE    = 0x20,
            DELETED = 0xE5,
        };

        /**
         * @class Fat32Directory
         * @brief Handles the directory operations on the FAT32 filesystem
         */
        class Fat32Directory : public Directory
        {

            private:
                Fat32Volume* m_volume;
                lba_t m_first_cluster;

                common::Vector<dir_entry_t> m_entries;

                lba_t create_entry(const string& name, bool is_directory);
                void remove_entry(lba_t cluster, const string& name);

            public:
                Fat32Directory(Fat32Volume* volume, lba_t cluster, const string& name);
                ~Fat32Directory() final;

                static const size_t MAX_NAME_LENGTH = 255;

                File* create_file(const string& name) final;
                void remove_file(const string& name) final;

                Directory* create_subdirectory(const string& name) final;
                void remove_subdirectory(const string& name) final;

                lba_t first_cluster() const { return m_first_cluster; }
        };

        /**
         * @class Fat32FileSystem
         * @brief Handles the FAT32 filesystem operations
         */
        class Fat32FileSystem : public FileSystem
        {
            private:
                Fat32Volume m_volume;

            public:
                Fat32FileSystem(drivers::disk::Disk* disk, uint32_t partition_offset);
                ~Fat32FileSystem() final;
        };

  }
}

#endif //MAXOS_FILESYSTEM_FAT32_H
