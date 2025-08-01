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

      // TODO: Revisit when I have the energy.
      //  BUG: Subdirectory seems to write to the disk this end but tools like
      //   fatcat complain the that the EOC isn't written (cluster 3037)
      //  - FAT32 Tests:
      //  - [x] Read subdirectories contents
      //  - [x] Read long path subdirectories contents
      //  - [ ] Create subdirectories
      //  - [ ] Create long path subdirectories
      //  - [ ] Delete subdirectories (need to add ability to free clusters first
      //  - [ ] Delete long path subdirectories
      //  - [ ] Rename directory
      //  - [ ] Rename file
      //  - [ ] Rename lfn directory
      //  - [ ] Rename lfn file
      //  - [x] Read files
      //  - [ ] Read large files
      //  - [x] Write files
      //  - [ ] Write large files
      //  - [ ] Create files
      //  - [ ] Delete files
      //  - [x] Read long path files
      //  - [ ] Create long path files
      //  - [ ] Delete long path files
      //  - [ ] Create files on a different mount point
      //  - [ ] Delete files on a different mount point
      //  - [ ] Read directories on a different mount point
      //  - [ ] Create directories on a different mount point
      //  - [ ] Stress test the filesystem: 1000s of files in a directory, long nested directories, long path files, etc

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

        enum class DirectoryEntryAttributes
        {
            FREE           = 0x00,
            READ_ONLY      = 0x01,
            HIDDEN         = 0x02,
            SYSTEM         = 0x04,
            VOLUME_ID      = 0x08,
            DIRECTORY      = 0x10,
            ARCHIVE        = 0x20,
            LONG_NAME      = READ_ONLY | HIDDEN | SYSTEM | VOLUME_ID,
        };

        enum class DirectoryEntryType
        {
          LAST   = 0x00,
          FREE = 0xE5,
        };

        typedef struct LongFileNameEntry
        {
            uint8_t     order;
            uint16_t    name1[5];
            uint8_t     attributes;
            uint8_t     type;
            uint8_t     checksum;
            uint16_t    name2[6];
            uint16_t    zero;
            uint16_t    name3[2];

        } __attribute__((packed)) long_file_name_entry_t;

        enum class ClusterState: uint32_t
        {
            FREE            = 0x00000000,
            BAD             = 0x0FFFFFF7,
            END_OF_CHAIN    = 0xFFFFFFFF,
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

                size_t   fat_total_clusters;
                lba_t    fat_lba;
                lba_t    fat_info_lba;
                lba_t    fat_copies;

                lba_t    data_lba;
                lba_t    root_lba;

                drivers::disk::Disk* disk;

                uint32_t next_cluster(uint32_t cluster);
                uint32_t set_next_cluster(uint32_t cluster, uint32_t next_cluster);
                uint32_t find_free_cluster();

                uint32_t allocate_cluster(uint32_t cluster);
                uint32_t allocate_cluster(uint32_t cluster, size_t amount);

                void free_cluster(uint32_t cluster);
                void free_cluster(uint32_t cluster, size_t amount);
        };

        // Forward def
        class Fat32Directory;

        /**
         * @class Fat32File
         * @brief Handles the file operations on the FAT32 filesystem
         */
        class Fat32File final : public File
        {

            private:
                Fat32Volume* m_volume;
                Fat32Directory* m_parent_directory;

                dir_entry_t* m_entry;
                uint32_t m_first_cluster;

            public:
                Fat32File(Fat32Volume* volume, Fat32Directory* parent, dir_entry_t* info, const string& name);
                ~Fat32File() final;

                void write(const common::buffer_t* data, size_t amount) final;
                void read(common::buffer_t* data, size_t amount) final;
                void flush() final;

                uint32_t first_cluster() const { return m_first_cluster; }
        };

        /**
         * @class Fat32Directory
         * @brief Handles the directory operations on the FAT32 filesystem
         */
        class Fat32Directory : public Directory
        {
            friend class Fat32File;

            private:
                Fat32Volume* m_volume;

                lba_t m_first_cluster;
                lba_t m_last_cluster;
                size_t m_current_cluster_length = 0;

                common::Vector<dir_entry_t> m_entries;

                dir_entry_t* create_entry(const string& name, bool is_directory);
                void remove_entry(lba_t cluster, const string& name);
                void read_all_entries();

                int entry_index(lba_t cluster);
                int find_free_entries(size_t amount);
                int expand_directory(size_t amount);

                static common::Vector<long_file_name_entry_t> to_long_filenames(string name);
                static string parse_long_filename(long_file_name_entry_t* entry, const string& current);

            protected:

                void save_entry_to_disk(dir_entry_t* entry);
                void update_entry_on_disk(int index);

            public:
                Fat32Directory(Fat32Volume* volume, lba_t cluster, const string& name);
                ~Fat32Directory();

                static const size_t MAX_NAME_LENGTH = 255;

                void read_from_disk() final;

                File* create_file(const string& name) final;
                void remove_file(const string& name) final;

                Directory* create_subdirectory(const string& name) final;
                void remove_subdirectory(const string& name) final;

                [[nodiscard]] lba_t first_cluster() const { return m_first_cluster; }
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
                ~Fat32FileSystem();
        };

  }
}

#endif //MAXOS_FILESYSTEM_FAT32_H
